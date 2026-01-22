/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ECLIPSE_EVENT_MANAGER_H
#define _ECLIPSE_EVENT_MANAGER_H

#include <sol/sol.hpp>
#include <unordered_map>
#include <vector>
#include "Common.h"
#include "Log.h"

namespace Eclipse::Core
{
    /**
     * @enum PlayerEvent
     * @brief Player-specific event types
     * 
     * Triggered by PlayerHooks when player state changes occur.
     */
    enum class PlayerEvent : uint32
    {
        ON_CHARACTER_CREATE = 1,  ///< Character created in database
        ON_CHARACTER_DELETE = 2,  ///< Character deleted from database
        ON_LOGIN = 3,             ///< Player logged into the world
        ON_LOGOUT = 4,            ///< Player logged out
        ON_FIRST_LOGIN = 5,       ///< First time login for character
        ON_LEVEL_CHANGE = 6,      ///< Player gained/lost a level
        ON_MONEY_CHANGE = 7,      ///< Player money changed
        ON_GIVE_XP = 8,           ///< Player received experience
        ON_REPUTATION_CHANGE = 9, ///< Reputation with faction changed
        ON_KILL_PLAYER = 10,      ///< Player killed another player
        ON_KILL_CREATURE = 11     ///< Player killed a creature
    };

    /**
     * @enum WorldEvent
     * @brief World-level event types
     * 
     * Triggered by WorldHooks for server-wide events.
     */
    enum class WorldEvent : uint32
    {
        ON_UPDATE = 1,          ///< World update tick
        ON_CONFIG_LOAD = 2,     ///< Configuration reloaded
        ON_SHUTDOWN_INIT = 3,   ///< Shutdown initiated
        ON_SHUTDOWN_CANCEL = 4, ///< Shutdown cancelled
        ON_STARTUP = 5,         ///< Server startup complete
        ON_SHUTDOWN = 6         ///< Server shutting down
    };

    /**
     * @enum WorldObjectEvent
     * @brief WorldObject lifecycle event types
     * 
     * Triggered by WorldObjectHooks for GameObjects.
     */
    enum class WorldObjectEvent : uint32
    {
        ON_CREATE = 1,  ///< GameObject created in world
        ON_DESTROY = 2, ///< GameObject removed from world
        ON_UPDATE = 3,  ///< GameObject updated
        ON_REPOP = 4    ///< GameObject respawned
    };

    /**
     * @struct EventHandler
     * @brief Lua callback wrapper with execution tracking
     * 
     * Stores a Lua function and metadata for shot-limited execution.
     * Move semantics ensure callbacks are never copied.
     */
    struct EventHandler
    {
        sol::protected_function function; ///< Lua callback (moved, never copied)
        uint32 shots;                     ///< Total allowed executions (0 = infinite)
        uint32 callCount;                 ///< Executions so far
        int32 stateId;                    ///< State ID (-1 = all states, future use)

        /**
         * @brief Construct an event handler
         * @param func Lua callback function (moved)
         * @param shotCount Execution limit (0 = infinite)
         * @param sid State ID (-1 for global)
         */
        EventHandler(sol::protected_function&& func, uint32 shotCount = 0, int32 sid = -1)
            : function(std::move(func))
            , shots(shotCount)
            , callCount(0)
            , stateId(sid)
        {
        }

        /**
         * @brief Check if handler should execute
         * @return true if under shot limit or infinite
         */
        bool ShouldExecute() const
        {
            return shots == 0 || callCount < shots;
        }

        /**
         * @brief Check if handler has expired
         * @return true if shot limit reached
         */
        bool IsExpired() const
        {
            return shots > 0 && callCount >= shots;
        }
    };

    /**
     * @struct GlobalEventKey
     * @brief Hash key for global events (type-safe enum discrimination)
     * 
     * Uses typeid hash to prevent collisions between different enum types
     * with identical values (e.g., PlayerEvent::ON_LOGIN=3 vs WorldEvent::ON_UPDATE=1).
     */
    struct GlobalEventKey
    {
        size_t eventCategory; ///< typeid(EventEnum).hash_code()
        uint32 eventType;     ///< Event enum value cast to uint32

        bool operator==(const GlobalEventKey& other) const
        {
            return eventCategory == other.eventCategory && eventType == other.eventType;
        }
    };

    /**
     * @struct EntryEventKey
     * @brief Hash key for entry-based events (Creature/GameObject by template ID)
     */
    struct EntryEventKey
    {
        size_t eventCategory; ///< typeid(EventEnum).hash_code()
        uint32 eventType;     ///< Event enum value
        uint32 entry;         ///< creature_template.entry or gameobject_template.entry

        bool operator==(const EntryEventKey& other) const
        {
            return eventCategory == other.eventCategory 
                && eventType == other.eventType 
                && entry == other.entry;
        }
    };

    /**
     * @struct UniqueEventKey
     * @brief Hash key for unique instance events (specific GUID)
     */
    struct UniqueEventKey
    {
        size_t eventCategory; ///< typeid(EventEnum).hash_code()
        uint32 eventType;     ///< Event enum value
        uint64 guid;          ///< Object GUID

        bool operator==(const UniqueEventKey& other) const
        {
            return eventCategory == other.eventCategory 
                && eventType == other.eventType 
                && guid == other.guid;
        }
    };
}

/**
 * @brief Hash specializations for event keys
 * 
 * Optimized hash functions combining category + type + optional ID.
 * Uses XOR with bit shifts to minimize collisions.
 */
namespace std
{
    template<>
    struct hash<Eclipse::Core::GlobalEventKey>
    {
        size_t operator()(const Eclipse::Core::GlobalEventKey& key) const noexcept
        {
            return key.eventCategory ^ (hash<uint32>()(key.eventType) << 1);
        }
    };

    template<>
    struct hash<Eclipse::Core::EntryEventKey>
    {
        size_t operator()(const Eclipse::Core::EntryEventKey& key) const noexcept
        {
            return key.eventCategory 
                ^ (hash<uint32>()(key.eventType) << 1) 
                ^ (hash<uint32>()(key.entry) << 2);
        }
    };

    template<>
    struct hash<Eclipse::Core::UniqueEventKey>
    {
        size_t operator()(const Eclipse::Core::UniqueEventKey& key) const noexcept
        {
            return key.eventCategory 
                ^ (hash<uint32>()(key.eventType) << 1) 
                ^ (hash<uint64>()(key.guid) << 2);
        }
    };
}


namespace Eclipse::Core
{
    /**
     * @class EventManager
     * @brief High-performance generic event dispatcher for Lua callbacks
     * 
     * **Architecture Overview:**
     * - Type-safe: Uses C++ templates + typeid discrimination for enum safety
     * - Lock-free: No mutexes (state -1 is single-threaded, future per-map states thread-isolated)
     * - Three event types:
     *   1. Global: RegisterPlayerEvent(), etc.
     *   2. Entry: RegisterCreatureGossipEvent(entry, ...)
     *   3. Unique: Future use for specific GUID events
     * 
     * **Design Pattern:**
     * Singleton for state -1, future: per-map instances managed by StateManager.
     * 
     * **Performance Characteristics:**
     * - Register: O(1) amortized (hash insert + vector push)
     * - Trigger: O(n) where n = handlers for that event (unavoidable)
     * - Cancel: O(1) hash lookup + O(k) vector cleanup (k = handlers in bucket)
     * 
     * **Memory Layout:**
     * Handlers stored in vector<pair<id, handler>> for cache locality.
     * Event keys use typeid hash to prevent cross-enum collisions.
     * 
     * @note Currently operates on master state (-1) only
     */
    class EventManager
    {
    public:
        EventManager();
        ~EventManager();

        EventManager(const EventManager&) = delete;
        EventManager& operator=(const EventManager&) = delete;
        EventManager(EventManager&&) = delete;
        EventManager& operator=(EventManager&&) = delete;

        static EventManager& GetInstance();

        /**
         * @brief Initialize the event manager
         * @return true on success
         */
        bool Initialize();

        /**
         * @brief Shutdown and cleanup all events
         */
        void Shutdown();

        /**
         * @brief Register a global event handler
         *
         * Global events are not tied to specific object instances.
         * Example: RegisterPlayerEvent(PlayerEvent::ON_LOGIN, callback)
         *
         * @tparam EventEnum Event type enum (PlayerEvent, WorldEvent, etc.)
         * @param eventType Event enum value
         * @param handler Lua callback (moved, not copied)
         * @param shots Execution limit (0 = infinite)
         * @param stateId State ID (-1 = all states, future use)
         * @return Unique handler ID for cancellation
         */
        template<typename EventEnum>
        uint64 RegisterGlobalEvent(
            EventEnum eventType,
            sol::protected_function&& handler,
            uint32 shots = 0,
            int32 stateId = -1
        );

        /**
         * @brief Register an entry-based event handler
         *
         * Entry events are tied to template IDs (creature_template, gameobject_template).
         * Example: RegisterCreatureGossipEvent(CreatureEvent::ON_GOSSIP_HELLO, 1234, callback)
         *
         * @tparam EventEnum Event type enum
         * @param eventType Event enum value
         * @param entry Template entry ID
         * @param handler Lua callback (moved, not copied)
         * @param shots Execution limit (0 = infinite)
         * @param stateId State ID (-1 = all states, future use)
         * @return Unique handler ID for cancellation
         */
        template<typename EventEnum>
        uint64 RegisterEntryEvent(
            EventEnum eventType,
            uint32 entry,
            sol::protected_function&& handler,
            uint32 shots = 0,
            int32 stateId = -1
        );

        /**
         * @brief Register a unique instance event handler
         *
         * Unique events are tied to specific object GUIDs.
         *
         * @tparam EventEnum Event type enum
         * @param eventType Event enum value
         * @param guid Object GUID
         * @param handler Lua callback (moved, not copied)
         * @param shots Execution limit (0 = infinite)
         * @param stateId State ID (-1 = all states, future use)
         * @return Unique handler ID for cancellation
         */
        template<typename EventEnum>
        uint64 RegisterUniqueEvent(
            EventEnum eventType,
            uint64 guid,
            sol::protected_function&& handler,
            uint32 shots = 0,
            int32 stateId = -1
        );

        /**
         * @brief Cancel a specific event handler
         * @param handlerId Handler ID returned by Register*Event
         * @return true if handler existed and was removed
         */
        bool CancelEvent(uint64 handlerId);

        /**
         * @brief Cancel all handlers for a global event
         * @tparam EventEnum Event type enum
         * @param eventType Event enum value
         */
        template<typename EventEnum>
        void CancelGlobalEvent(EventEnum eventType);

        /**
         * @brief Cancel all handlers for an entry event
         * @tparam EventEnum Event type enum
         * @param eventType Event enum value
         * @param entry Template entry ID
         */
        template<typename EventEnum>
        void CancelEntryEvent(EventEnum eventType, uint32 entry);

        /**
         * @brief Cancel all handlers for a unique event
         * @tparam EventEnum Event type enum
         * @param eventType Event enum value
         * @param guid Object GUID
         */
        template<typename EventEnum>
        void CancelUniqueEvent(EventEnum eventType, uint64 guid);

        /**
         * @brief Cancel all events for a specific state
         * @param stateId State ID to clean up
         */
        void CancelStateEvents(int32 stateId);

        /**
         * @brief Cancel ALL registered events (shutdown)
         */
        void CancelAllEvents();

        /**
         * @brief Trigger a global event (hot path)
         *
         * Executes all registered handlers for this event type.
         * Removes expired handlers after execution.
         *
         * @tparam EventEnum Event type enum
         * @tparam Args Variadic argument types for Lua callback
         * @param eventType Event enum value
         * @param args Arguments forwarded to Lua callbacks
         * @return Number of handlers successfully executed
         */
        template<typename EventEnum, typename... Args>
        uint32 TriggerGlobalEvent(EventEnum eventType, Args&&... args);

        /**
         * @brief Trigger an entry event (hot path)
         *
         * @tparam EventEnum Event type enum
         * @tparam Args Variadic argument types for Lua callback
         * @param eventType Event enum value
         * @param entry Template entry ID
         * @param args Arguments forwarded to Lua callbacks
         * @return Number of handlers successfully executed
         */
        template<typename EventEnum, typename... Args>
        uint32 TriggerEntryEvent(EventEnum eventType, uint32 entry, Args&&... args);

        /**
         * @brief Trigger a unique instance event (hot path)
         *
         * @tparam EventEnum Event type enum
         * @tparam Args Variadic argument types for Lua callback
         * @param eventType Event enum value
         * @param guid Object GUID
         * @param args Arguments forwarded to Lua callbacks
         * @return Number of handlers successfully executed
         */
        template<typename EventEnum, typename... Args>
        uint32 TriggerUniqueEvent(EventEnum eventType, uint64 guid, Args&&... args);

        /**
         * @brief Trigger global event with return value capture
         *
         * Executes all handlers and captures return value from LAST successful handler.
         * Use case: Events that modify values (e.g., OnGiveXP returning modified amount).
         *
         * @tparam ReturnType Type of return value to capture
         * @tparam EventEnum Event type enum
         * @tparam Args Variadic argument types for Lua callback
         * @param eventType Event enum value
         * @param defaultValue Default return value if no handlers execute
         * @param args Arguments forwarded to Lua callbacks
         * @return Last valid return value from Lua, or defaultValue if none
         */
        template<typename ReturnType, typename EventEnum, typename... Args>
        ReturnType TriggerGlobalEventWithReturn(EventEnum eventType, ReturnType defaultValue, Args&&... args);

        /**
         * @brief Trigger entry event with return value capture
         *
         * @tparam ReturnType Type of return value to capture
         * @tparam EventEnum Event type enum
         * @tparam Args Variadic argument types for Lua callback
         * @param eventType Event enum value
         * @param entry Template entry ID
         * @param defaultValue Default return value if no handlers execute
         * @param args Arguments forwarded to Lua callbacks
         * @return Last valid return value from Lua, or defaultValue if none
         */
        template<typename ReturnType, typename EventEnum, typename... Args>
        ReturnType TriggerEntryEventWithReturn(EventEnum eventType, uint32 entry, ReturnType defaultValue, Args&&... args);

    private:
        // Handler storage: vector for cache locality, pair<id, handler> for tracking
        using GlobalHandlerMap = std::unordered_map<GlobalEventKey, std::vector<std::pair<uint64, EventHandler>>>;
        using EntryHandlerMap = std::unordered_map<EntryEventKey, std::vector<std::pair<uint64, EventHandler>>>;
        using UniqueHandlerMap = std::unordered_map<UniqueEventKey, std::vector<std::pair<uint64, EventHandler>>>;

        GlobalHandlerMap m_globalHandlers; // < Global event storage
        EntryHandlerMap m_entryHandlers;   // < Entry-based event storage
        UniqueHandlerMap m_uniqueHandlers; // < Unique instance event storage

        uint64 m_nextHandlerId; // < Monotonic counter for unique IDs
        bool m_initialized;     // < Initialization state

        /**
         * @brief Generate next unique handler ID
         * @return Monotonically increasing ID
         */
        uint64 GenerateHandlerId() { return m_nextHandlerId++; }

        /**
         * @brief Execute a handler and update its state
         * @tparam Args Variadic argument types
         * @param handler Handler to execute
         * @param args Arguments to forward to Lua
         * @return true if execution succeeded (not expired)
         */
        template<typename... Args>
        bool ExecuteHandler(EventHandler& handler, Args&&... args);

        /**
         * @brief Remove expired handlers from a vector (batch cleanup)
         * @tparam T Handler type (EventHandler)
         * @param handlers Vector of <id, handler> pairs
         */
        template<typename T>
        void RemoveExpiredHandlers(std::vector<std::pair<uint64, T>>& handlers);

        /**
         * @brief Helper: Find and cancel handler by ID in a map (DRY)
         * @tparam MapType Handler map type (GlobalHandlerMap, EntryHandlerMap, UniqueHandlerMap)
         * @param handlerMap Map to search
         * @param handlerId Handler ID to find
         * @param logType Type name for logging ("global", "entry", "unique")
         * @return true if handler found and cancelled
         */
        template<typename MapType>
        bool FindAndCancelHandler(MapType& handlerMap, uint64 handlerId, const char* logType);

        /**
         * @brief Helper: Cancel all handlers for a state in a map (DRY)
         * @tparam MapType Handler map type
         * @param handlerMap Map to process
         * @param stateId State ID to match
         * @return Number of handlers cancelled
         */
        template<typename MapType>
        size_t CancelStateHandlersInMap(MapType& handlerMap, int32 stateId);
    };

    /**
     * @brief Trigger global event
     *
     */
    template<typename EventEnum, typename... Args>
    uint32 EventManager::TriggerGlobalEvent(EventEnum eventType, Args&&... args)
    {
        GlobalEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType) };

        // Early exit: no handlers registered
        auto it = m_globalHandlers.find(key);
        if (it == m_globalHandlers.end())
        {
            return 0;
        }

        // Execute all handlers for this event
        uint32 executed = 0;
        auto& handlers = it->second;

        for (auto& [handlerId, handler] : handlers)
        {
            if (ExecuteHandler(handler, std::forward<Args>(args)...))
            {
                executed++;
            }
        }

        // Batch remove expired handlers (shot limit reached)
        RemoveExpiredHandlers(handlers);

        return executed;
    }

    /**
     * @brief Trigger entry event
     */
    template<typename EventEnum, typename... Args>
    uint32 EventManager::TriggerEntryEvent(EventEnum eventType, uint32 entry, Args&&... args)
    {
        EntryEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), entry };

        auto it = m_entryHandlers.find(key);
        if (it == m_entryHandlers.end())
        {
            return 0;
        }

        uint32 executed = 0;
        auto& handlers = it->second;

        for (auto& [handlerId, handler] : handlers)
        {
            if (ExecuteHandler(handler, std::forward<Args>(args)...))
            {
                executed++;
            }
        }

        RemoveExpiredHandlers(handlers);

        return executed;
    }

    /**
     * @brief Trigger unique event
     */
    template<typename EventEnum, typename... Args>
    uint32 EventManager::TriggerUniqueEvent(EventEnum eventType, uint64 guid, Args&&... args)
    {
        UniqueEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), guid };

        auto it = m_uniqueHandlers.find(key);
        if (it == m_uniqueHandlers.end())
            return 0;

        uint32 executed = 0;
        auto& handlers = it->second;

        for (auto& [handlerId, handler] : handlers)
        {
            if (ExecuteHandler(handler, std::forward<Args>(args)...))
            {
                executed++;
            }
        }

        RemoveExpiredHandlers(handlers);

        return executed;
    }

    /**
     * @brief Execute handler with Lua error handling
     */
    template<typename... Args>
    bool EventManager::ExecuteHandler(EventHandler& handler, Args&&... args)
    {
        if (!handler.ShouldExecute())
            return false;

        try
        {
            auto result = handler.function(std::forward<Args>(args)...);
            if (!result.valid())
            {
                sol::error err = result;
                // Errors logged by sol2, just track execution
                return false;
            }

            handler.callCount++;
            return true;
        }
        catch (const std::exception&)
        {
            // Exception logged by sol2
            return false;
        }
    }

    /**
     * @brief Batch remove expired handlers
     */
    template<typename T>
    void EventManager::RemoveExpiredHandlers(std::vector<std::pair<uint64, T>>& handlers)
    {
        handlers.erase(
            std::remove_if(handlers.begin(), handlers.end(),
                [](const auto& pair) { return pair.second.IsExpired(); }),
            handlers.end()
        );
    }

    /**
     * @brief Register global event
     */
    template<typename EventEnum>
    uint64 EventManager::RegisterGlobalEvent(EventEnum eventType, sol::protected_function&& handler, uint32 shots, int32 stateId)
    {
        GlobalEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType) };
        uint64 handlerId = GenerateHandlerId();

        // Emplace handler
        m_globalHandlers[key].emplace_back(handlerId, EventHandler(std::move(handler), shots, stateId));

        return handlerId;
    }

    /**
     * @brief Register entry event
     */
    template<typename EventEnum>
    uint64 EventManager::RegisterEntryEvent(EventEnum eventType, uint32 entry, sol::protected_function&& handler, uint32 shots, int32 stateId)
    {
        EntryEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), entry };
        uint64 handlerId = GenerateHandlerId();

        // Emplace handler
        m_entryHandlers[key].emplace_back(handlerId, EventHandler(std::move(handler), shots, stateId));

        return handlerId;
    }

    /**
     * @brief Register unique event
     */
    template<typename EventEnum>
    uint64 EventManager::RegisterUniqueEvent(EventEnum eventType, uint64 guid, sol::protected_function&& handler, uint32 shots, int32 stateId)
    {
        UniqueEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), guid };
        uint64 handlerId = GenerateHandlerId();

        // Emplace handler
        m_uniqueHandlers[key].emplace_back(handlerId, EventHandler(std::move(handler), shots, stateId));

        return handlerId;
    }

    /**
     * @brief Cancel global event - cleanup all handlers
     */
    template<typename EventEnum>
    void EventManager::CancelGlobalEvent(EventEnum eventType)
    {
        GlobalEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType) };
        auto it = m_globalHandlers.find(key);

        if (it != m_globalHandlers.end())
            m_globalHandlers.erase(it);
    }

    /**
     * @brief Cancel entry event - cleanup all handlers
     */
    template<typename EventEnum>
    void EventManager::CancelEntryEvent(EventEnum eventType, uint32 entry)
    {
        EntryEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), entry };
        auto it = m_entryHandlers.find(key);

        if (it != m_entryHandlers.end())
        {
            m_entryHandlers.erase(it);
        }
    }

    /**
     * @brief Cancel unique event - cleanup all handlers
     */
    template<typename EventEnum>
    void EventManager::CancelUniqueEvent(EventEnum eventType, uint64 guid)
    {
        UniqueEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), guid };
        auto it = m_uniqueHandlers.find(key);

        if (it != m_uniqueHandlers.end())
        {
            m_uniqueHandlers.erase(it);
        }
    }

    /**
     * @brief Helper: Find and cancel handler in map
     */
    template<typename MapType>
    bool EventManager::FindAndCancelHandler(MapType& handlerMap, uint64 handlerId, const char* logType)
    {
        for (auto& [key, handlers] : handlerMap)
        {
            auto it = std::find_if(handlers.begin(), handlers.end(),
                [handlerId](const auto& pair) { return pair.first == handlerId; });

            if (it != handlers.end())
            {
                handlers.erase(it);
                LOG_INFO("eclipse.events", "[Eclipse] EventManager - Cancelled {} event handler {}", logType, handlerId);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Helper: Cancel state handlers in map
     */
    template<typename MapType>
    size_t EventManager::CancelStateHandlersInMap(MapType& handlerMap, int32 stateId)
    {
        size_t totalCancelled = 0;
        for (auto& [key, handlers] : handlerMap)
        {
            auto originalSize = handlers.size();
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(),
                    [stateId](const auto& pair) { return pair.second.stateId == stateId; }),
                handlers.end()
            );
            auto cancelled = originalSize - handlers.size();
            totalCancelled += cancelled;
        }
        return totalCancelled;
    }

    /**
     * @brief Trigger global event with return value capture
     *
     * Executes all handlers sequentially. If a handler returns a value,
     * that value becomes the new returnValue (last handler wins).
     */
    template<typename ReturnType, typename EventEnum, typename... Args>
    ReturnType EventManager::TriggerGlobalEventWithReturn(EventEnum eventType, ReturnType defaultValue, Args&&... args)
    {
        GlobalEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType) };

        auto it = m_globalHandlers.find(key);
        if (it == m_globalHandlers.end())
        {
            return defaultValue;
        }

        ReturnType returnValue = defaultValue;
        uint32 executed = 0;
        auto& handlers = it->second;

        for (auto& [handlerId, handler] : handlers)
        {
            if (!handler.ShouldExecute())
                continue;

            try
            {
                auto result = handler.function(std::forward<Args>(args)...);
                if (!result.valid())
                {
                    sol::error err = result;
                    LOG_ERROR("eclipse.events", "[Eclipse] EventManager - Lua error in handler {}: {}", handlerId, err.what());
                    continue;
                }

                handler.callCount++;
                executed++;

                // Try to extract return value from Lua result
                if (result.return_count() > 0)
                {
                    sol::optional<ReturnType> luaReturn = result;
                    if (luaReturn)
                        returnValue = *luaReturn;
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("eclipse.events", "[Eclipse] EventManager - Exception in handler {}: {}", handlerId, e.what());
            }
        }

        RemoveExpiredHandlers(handlers);

        return returnValue;
    }

    /**
     * @brief Trigger entry event with return value capture
     */
    template<typename ReturnType, typename EventEnum, typename... Args>
    ReturnType EventManager::TriggerEntryEventWithReturn(EventEnum eventType, uint32 entry, ReturnType defaultValue, Args&&... args)
    {
        EntryEventKey key{ typeid(EventEnum).hash_code(), static_cast<uint32>(eventType), entry };

        auto it = m_entryHandlers.find(key);
        if (it == m_entryHandlers.end())
            return defaultValue;

        ReturnType returnValue = defaultValue;
        uint32 executed = 0;
        auto& handlers = it->second;

        for (auto& [handlerId, handler] : handlers)
        {
            if (!handler.ShouldExecute())
                continue;

            try
            {
                auto result = handler.function(std::forward<Args>(args)...);
                if (!result.valid())
                {
                    sol::error err = result;
                    LOG_ERROR("eclipse.events", "[Eclipse] EventManager - Lua error in entry handler {}: {}", handlerId, err.what());
                    continue;
                }

                handler.callCount++;
                executed++;

                // Try to extract return value from Lua result
                if (result.return_count() > 0)
                {
                    sol::optional<ReturnType> luaReturn = result;
                    if (luaReturn)
                        returnValue = *luaReturn;
                }
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("eclipse.events", "[Eclipse] EventManager - Exception in entry handler {}: {}", handlerId, e.what());
            }
        }

        RemoveExpiredHandlers(handlers);

        return returnValue;
    }

} // namespace Eclipse::Core

#endif // _ECLIPSE_EVENT_MANAGER_H
