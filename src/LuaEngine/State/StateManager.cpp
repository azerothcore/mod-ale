/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "StateManager.h"
#include "BytecodeCache.h"
#include "TimedEventManager.h"
#include "Log.h"
#include <chrono>

namespace ALE::Core
{
    /**
     * @brief Singleton getter for StateManager
     * 
     * Thread-safe initialization (C++11 magic statics).
     * 
     * @return Reference to the global StateManager
     */
    StateManager& StateManager::GetInstance()
    {
        static StateManager instance;
        return instance;
    }

    /**
     * @brief Constructor - initializes manager
     */
    StateManager::StateManager()
    {
    }

    /**
     * @brief Destructor - cleanup all states
     */
    StateManager::~StateManager()
    {
        Shutdown();
    }

    /**
     * @brief Initialize state manager and create master state
     * 
     * Creates master state (-1) with its TimedEventManager.
     * Called from ALEScriptLoader during server startup.
     * 
     * @return true on success
     */
    bool StateManager::Initialize()
    {
        if (m_initialized)
            return true;

        try
        {
            sol::state* masterState = GetOrCreateState(MASTER_STATE_ID);
            if (!masterState)
            {
                LOG_ERROR("server.loading", "[ALE] StateManager - Failed to create master state");
                return false;
            }

            m_initialized = true;
            return true;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("server.loading", "[ALE] StateManager - Initialization failed: {}", e.what());
            return false;
        }
    }

    /**
     * @brief Shutdown all states
     * 
     * Destroys all Lua VMs and their TimedEventManagers.
     * Automatic cleanup via unique_ptr.
     */
    void StateManager::Shutdown()
    {
        if (!m_initialized)
            return;

        m_states.clear();
        m_initialized = false;
    }

    /**
     * @brief Get or create state for a map
     * 
     * Current: Always returns master state (-1), ignores mapId.
     * Future: Creates per-map states on demand.
     * 
     * @param mapId Map ID (-1 = master, >=0 = per-map)
     * @return Pointer to sol::state, never nullptr
     */
    sol::state* StateManager::GetOrCreateState(StateId mapId)
    {
        // NO LOCK: State -1 is single-threaded

        // Check if state already exists
        auto it = m_states.find(mapId);
        if (it != m_states.end())
        {
            return it->second.state.get();
        }

        // Create new state
        return CreateNewState(mapId);
    }

    /**
     * @brief Get existing state (no creation)
     * 
     * @param mapId State ID to retrieve
     * @return Pointer to sol::state, nullptr if not found
     */
    sol::state* StateManager::GetState(StateId mapId) const
    {
        // NO LOCK: State -1 is single-threaded

        auto it = m_states.find(mapId);
        if (it != m_states.end())
        {
            return it->second.state.get();
        }

        return nullptr;
    }

    /**
     * @brief Remove a state (map unload cleanup)
     * 
     * Current: Only master state (-1) exists, protected from removal.
     * Future: Called from Map::Destroy() to cleanup per-map states.
     * 
     * @param mapId Map ID to remove
     */
    void StateManager::RemoveState(StateId mapId)
    {
        // NO LOCK: State -1 is single-threaded

        // Don't allow removing the master state
        if (mapId == MASTER_STATE_ID)
        {
            LOG_WARN("server.loading", "[ALE] StateManager - Cannot remove master state");
            return;
        }

        auto it = m_states.find(mapId);
        if (it != m_states.end())
        {
            LOG_DEBUG("server.loading", "[ALE] StateManager - Removing state for map {}", mapId);
            m_states.erase(it); // Unique_ptr auto-destructs state + managers
        }
    }

    /**
     * @brief Get master state (-1)
     * 
     * Convenience method for accessing the master compilation state.
     * 
     * @return Pointer to master sol::state, never nullptr after Initialize()
     */
    sol::state* StateManager::GetMasterState() const
    {
        return GetState(MASTER_STATE_ID);
    }

    /**
     * @brief Get number of active states
     * 
     * Current: Always 1 (master state only).
     * Future: 1 + number of active maps.
     * 
     * @return State count
     */
    size_t StateManager::GetStateCount() const
    {
        // NO LOCK: State -1 is single-threaded
        return m_states.size();
    }

    /**
     * @brief Get all active state IDs
     * 
     * Current: Returns {-1}.
     * Future: Returns {-1, mapId1, mapId2, ...}.
     * 
     * @return Vector of state IDs
     */
    std::vector<StateManager::StateId> StateManager::GetAllStateIds() const
    {
        // NO LOCK: State -1 is single-threaded

        std::vector<StateId> ids;
        ids.reserve(m_states.size());

        for (const auto& [id, _] : m_states)
        {
            ids.push_back(id);
        }

        return ids;
    }

    /**
     * @brief Execute function on all states
     * 
     * Iteration helper for bulk operations.
     * Future: Used for script reload, metrics collection, etc.
     * 
     * @param func Callback(stateId, sol::state&)
     */
    void StateManager::ForEachState(std::function<void(StateId, sol::state&)> func)
    {
        // NO LOCK: State -1 is single-threaded

        for (auto& [id, entry] : m_states)
        {
            if (entry.state)
            {
                func(id, *entry.state);
            }
        }
    }

    /**
     * @brief Get TimedEventManager for a state
     * 
     * Each state owns its TimedEventManager for timed Lua callbacks.
     * 
     * @param mapId State ID
     * @return Pointer to TimedEventManager, nullptr if state not found
     */
    TimedEventManager* StateManager::GetTimedEventManager(StateId mapId) const
    {
        // NO LOCK: State -1 is single-threaded

        auto it = m_states.find(mapId);
        if (it != m_states.end())
        {
            return it->second.timedEventManager.get();
        }

        return nullptr;
    }

    /**
     * @brief Create new state with default configuration
     * 
     * Initializes:
     * - Lua VM (sol::state)
     * - Standard libraries (base, string, table, math, etc.)
     * - TimedEventManager for this state
     * - Global Lua functions (RegisterPlayerEvent, etc.)
     * 
     * Performance: ~10ms for VM init + library loading.
     * 
     * @param mapId State ID for this state
     * @return Pointer to created sol::state
     */
    sol::state* StateManager::CreateNewState(StateId mapId)
    {
        try
        {
            bool isMasterState = (mapId == MASTER_STATE_ID);

            LOG_DEBUG("server.loading", "[ALE] StateManager - Creating {} state for map {}",
                isMasterState ? "master" : "map", mapId);

            // Create new state entry
            StateEntry entry;
            entry.state = std::make_unique<sol::state>();
            entry.timedEventManager = std::make_unique<TimedEventManager>(mapId);  // Per-state event manager
            entry.created = std::chrono::steady_clock::now();
            entry.executionCount = 0;

            // Setup standard Lua libraries
            SetupStateLibraries(*entry.state, isMasterState);

            // Store the state and return pointer
            sol::state* statePtr = entry.state.get();
            m_states[mapId] = std::move(entry);

            LOG_DEBUG("server.loading", "[ALE] StateManager - State {} created successfully with dedicated TimedEventManager", mapId);

            return statePtr;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("server.loading", "[ALE] StateManager - Failed to create state for map {}: {}", 
                mapId, e.what());
            return nullptr;
        }
    }

    /**
     * @brief Setup standard Lua libraries for a state
     * 
     * Loads:
     * - base: print, assert, pcall, etc.
     * - package: require, module system
     * - coroutine: coroutine support
     * - string: string manipulation
     * - table: table operations
     * - math: math functions
     * - bit32: bitwise operations
     * - io: file I/O (restricted in future)
     * - os: OS functions (restricted in future)
     * - debug: debug library (master state only in future)
     * 
     * @param state Lua state to configure
     * @param isMasterState Whether this is the master state
     */
    void StateManager::SetupStateLibraries(sol::state& state, bool isMasterState)
    {
        // Open standard Lua libraries
        state.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::coroutine,
            sol::lib::string,
            sol::lib::table,
            sol::lib::math,
            sol::lib::bit32,
            sol::lib::io,      // Future: Restrict for per-map states
            sol::lib::os,      // Future: Restrict for per-map states
            sol::lib::debug    // Future: Master state only
        );

        // Set up package paths (configured in ScriptLoader)
        // state["package"]["path"] = "./lua/?.lua;./lua/?/init.lua";
        // state["package"]["cpath"] = "./lua/?.dll;./lua/?/init.dll";

        LOG_DEBUG("server.loading", "[ALE] StateManager - Libraries setup complete for {} state",
            isMasterState ? "master" : "map");
    }

} // namespace ALE::Core
