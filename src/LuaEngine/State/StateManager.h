/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _SOL_STATE_MANAGER_H
#define _SOL_STATE_MANAGER_H

#include <sol/sol.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include "Common.h"

namespace ALE::Core
{
    class ScriptCompiler;
    class BytecodeCache;
    class TimedEventManager;

    /**
     * @class StateManager
     * @brief Manages Lua states and their lifecycle
     *
     * @note Currently operates ONLY on master state (-1)
     */
    class StateManager
    {
    public:
        using StateId = int32;

        /**
         * @brief Master state ID constant (-1)
         */
        static constexpr StateId MASTER_STATE_ID = -1;

        /**
         * @brief Default constructor
         */
        StateManager();

        /**
         * @brief Destructor - cleans up all states
         */
        ~StateManager();

        StateManager(const StateManager&) = delete;
        StateManager& operator=(const StateManager&) = delete;
        StateManager(StateManager&&) = delete;
        StateManager& operator=(StateManager&&) = delete;

        /**
         * @brief Get singleton instance
         * @return Reference to the global StateManager
         */
        static StateManager& GetInstance();

        /**
         * @brief Initialize state manager and create master state
         *
         * @return true on success
         */
        bool Initialize();

        /**
         * @brief Shutdown all states and cleanup
         *
         * Destroys all states and their associated managers.
         * Called during server shutdown.
         */
        void Shutdown();

        /**
         * @brief Get or create a state for a specific map
         *
         * @param mapId Map ID (-1 = master, >=0 = per-map)
         * @return Pointer to sol::state, never nullptr (creates if missing)
         */
        sol::state* GetOrCreateState(StateId mapId);

        /**
         * @brief Get existing state (no creation)
         *
         * @param mapId Map ID to retrieve
         * @return Pointer to sol::state, nullptr if not found
         */
        sol::state* GetState(StateId mapId) const;

        /**
         * @brief Remove a state (map unload cleanup)
         *
         * @param mapId Map ID to remove (>=0 only)
         */
        void RemoveState(StateId mapId);

        /**
         * @brief Get master state (-1)
         *
         * @return Pointer to master sol::state, never nullptr after Initialize()
         */
        sol::state* GetMasterState() const;

        /**
         * @brief Check initialization status
         *
         * @return true if Initialize() succeeded
         */
        bool IsInitialized() const { return m_initialized; }

        /**
         * @brief Get number of active states
         *
         * @return State count
         */
        size_t GetStateCount() const;

        /**
         * @brief Get all active state IDs
         *
         * @return Vector of state IDs
         */
        std::vector<StateId> GetAllStateIds() const;

        /**
         * @brief Execute function on all states
         *
         * Iteration helper for operations affecting multiple states.
         *
         * @param func Callback(stateId, sol::state&)
         */
        void ForEachState(std::function<void(StateId, sol::state&)> func);

        /**
         * @brief Get TimedEventManager for a state
         *
         * Each state owns its TimedEventManager for timed Lua callbacks.
         *
         * @param mapId State ID
         * @return Pointer to TimedEventManager, nullptr if state not found
         */
        TimedEventManager* GetTimedEventManager(StateId mapId) const;

    private:
        /**
         * @struct StateEntry
         * @brief Storage for state and its associated managers
         */
        struct StateEntry
        {
            std::unique_ptr<sol::state> state;                      ///< Lua VM instance
            std::unique_ptr<TimedEventManager> timedEventManager;   ///< Timed event scheduler
            std::chrono::steady_clock::time_point created;          ///< Creation timestamp
            size_t executionCount = 0;                              ///< Script execution counter
        };

        std::unordered_map<StateId, StateEntry> m_states; ///< Map of state ID -> StateEntry
        bool m_initialized = false;                       ///< Initialization flag

        /**
         * @brief Create new state with default configuration
         *
         * Initializes Lua VM, loads libraries, creates TimedEventManager.
         *
         * @param mapId State ID for this state
         * @return Pointer to created sol::state
         */
        sol::state* CreateNewState(StateId mapId);

        /**
         * @brief Setup standard Lua libraries
         *
         * Loads: base, coroutine, string, table, math, os (restricted), io (restricted)
         *
         * @param state Lua state to configure
         * @param isMasterState Whether this is master state (affects library restrictions)
         */
        void SetupStateLibraries(sol::state& state, bool isMasterState);

        /**
         * @brief Register global functions exposed to Lua
         *
         * Registers: print, RegisterPlayerEvent, RegisterTimedEvent, etc.
         *
         * @param state Lua state to register functions in
         */
        void RegisterGlobalFunctions(sol::state& state);
    };

} // namespace ALE::Core

#endif // _SOL_STATE_MANAGER_H
