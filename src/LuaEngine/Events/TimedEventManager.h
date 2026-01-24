/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_TIMED_EVENT_MANAGER_H
#define _ALE_TIMED_EVENT_MANAGER_H

#include <sol/sol.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ObjectGuid.h"
#include "Object.h"

namespace ALE::Core
{
    /**
     * @enum TimedEventObjectType
     * @brief Defines the type of object associated with a timed event
     *
     * Used to determine how event callbacks are invoked and what arguments are passed.
     */
    enum class TimedEventObjectType : uint8
    {
        GLOBAL,      ///< Global event, no object association
        PLAYER,      ///< Player-specific event
        CREATURE,    ///< Creature-specific event
        GAMEOBJECT   ///< GameObject-specific event
    };

    /**
     * @struct TimedEvent
     * @brief Represents a scheduled Lua callback with delay and repeat configuration
     *
     * Stores all state needed to track and execute a timed event. Events can be:
     * - Global: Executed at world-level (CreateLuaEvent)
     * - Object-bound: Executed in context of a specific Player/Creature/GameObject
     */
    struct TimedEvent
    {
        uint64 id;                          ///< Unique event identifier (monotonically increasing)
        sol::protected_function callback;   ///< Lua callback function (moved, not copied)
        uint32 delay;                       ///< Delay between executions in milliseconds
        uint32 repeats;                     ///< Total repeats (0 = infinite)
        uint32 remainingRepeats;            ///< Repeats left before expiration
        uint32 elapsed;                     ///< Accumulated time since last execution (ms)
        ObjectGuid objectGuid;              ///< Associated object GUID (Empty for global)
        TimedEventObjectType objectType;    ///< Object type discriminator

        /**
         * @brief Construct a timed event
         * @param eventId Unique event ID
         * @param cb Lua callback (moved for performance)
         * @param delayMs Delay in milliseconds
         * @param repeatCount Number of repeats (0 = infinite)
         * @param guid Object GUID (default: Empty for global events)
         * @param type Object type (default: GLOBAL)
         */
        TimedEvent(uint64 eventId, sol::protected_function&& cb, uint32 delayMs, uint32 repeatCount, ObjectGuid guid = ObjectGuid::Empty, TimedEventObjectType type = TimedEventObjectType::GLOBAL)
            : id(eventId), callback(std::move(cb)), delay(delayMs), repeats(repeatCount)
            , remainingRepeats(repeatCount), elapsed(0), objectGuid(guid), objectType(type)
        {
        }
    };

    /**
     * @class TimedEventManager
     * @brief High-performance timed event scheduler for Lua callbacks
     */
    class TimedEventManager
    {
    public:
        explicit TimedEventManager(int32 mapId);
        ~TimedEventManager();

        /**
         * @brief Register a global timed event (CreateLuaEvent)
         *
         * Global events run independently of any game object. Ideal for:
         * - World-level timers
         * - Periodic server announcements
         * - Background cleanup tasks
         *
         * @param callback Lua function: function(eventId, delay, repeats)
         * @param delay Delay in milliseconds between executions
         * @param repeats Number of times to execute (0 = infinite)
         * @return Unique event ID for cancellation
         */
        uint64 RegisterGlobalEvent(sol::protected_function&& callback, uint32 delay, uint32 repeats);

        /**
         * @brief Register an object-bound timed event (player:RegisterEvent, etc.)
         *
         * Object events are executed with the object as context. Used for:
         * - Player buffs/debuffs
         * - Creature AI timers
         * - GameObject state changes
         *
         * @param objectGuid GUID of the object (Player, Creature, GameObject)
         * @param callback Lua function: function(eventId, delay, repeats, object)
         * @param delay Delay in milliseconds between executions
         * @param repeats Number of times to execute (0 = infinite)
         * @param objectType Type discriminator (PLAYER, CREATURE, GAMEOBJECT)
         * @return Unique event ID for cancellation
         */
        uint64 RegisterObjectEvent(ObjectGuid objectGuid, sol::protected_function&& callback, uint32 delay, uint32 repeats, TimedEventObjectType objectType);

        /**
         * @brief Cancel a specific timed event
         * @param eventId Event ID returned by Register*Event
         * @return true if event existed and was removed, false if not found
         */
        bool RemoveEvent(uint64 eventId);

        /**
         * @brief Cancel all events associated with an object
         *
         * Called when object is destroyed or despawned to prevent dangling callbacks.
         *
         * @param objectGuid GUID of the object to clean up
         */
        void RemoveObjectEvents(ObjectGuid objectGuid);

        /**
         * @brief Cancel all global events
         */
        void RemoveAllGlobalEvents();

        /**
         * @brief Update all global timed events (hot path)
         *
         * Called every tick from WorldHooks::OnUpdate.
         *
         * @param diff Time elapsed since last update (milliseconds)
         */
        void Update(uint32 diff);

        /**
         * @brief Update events for a specific object (hot path)
         *
         * Called from object-specific hooks (OnPlayerUpdate, OnCreatureUpdate, etc.).
         * Object is passed directly.
         *
         * @param obj WorldObject to update events for
         * @param diff Time elapsed since last update (milliseconds)
         */
        void UpdateObjectEvents(WorldObject* obj, uint32 diff);

        /**
         * @brief Get total number of active timed events
         * @return Count of all events (global + object)
         */
        uint32 GetActiveEventCount() const { return static_cast<uint32>(m_events.size()); }

        /**
         * @brief Get number of active global events
         * @return Count of global events only
         */
        uint32 GetGlobalEventCount() const { return static_cast<uint32>(m_globalEvents.size()); }

        /**
         * @brief Get number of events for a specific object
         * @param objectGuid GUID to query
         * @return Count of events for this object
         */
        uint32 GetObjectEventCount(ObjectGuid objectGuid) const;

        /**
         * @brief Clear all events (shutdown)
         *
         * Releases all Lua callback references and clears internal structures.
         */
        void Clear();

    private:
        TimedEventManager(const TimedEventManager&) = delete;
        TimedEventManager& operator=(const TimedEventManager&) = delete;
        TimedEventManager(TimedEventManager&&) = delete;
        TimedEventManager& operator=(TimedEventManager&&) = delete;

        /**
         * @brief Execute a timed event callback
         * @param event Event to execute
         * @param obj Object context (nullptr for global events)
         */
        void ExecuteEvent(TimedEvent& event, WorldObject* obj = nullptr);

        /**
         * @brief Update a list of events
         * @tparam Container Container type
         * @param eventIds Event IDs to update
         * @param obj Object context
         * @param diff Time delta in milliseconds
         */
        template<typename Container>
        void UpdateEventList(const Container& eventIds, WorldObject* obj, uint32 diff);

        // Storage: Events by ID for fast lookup/removal
        std::unordered_map<uint64, TimedEvent> m_events;

        // Index: Events by object GUID for UpdateObjectEvents
        std::unordered_map<ObjectGuid, std::unordered_set<uint64>> m_objectEvents;

        // Set: Global event IDs for Update
        std::unordered_set<uint64> m_globalEvents;

        uint64 m_nextEventId;  // < Monotonic counter for unique IDs
        int32 m_mapId;         // < Map ID this manager belongs to
    };

} // namespace ALE::Core

#endif // _ALE_TIMED_EVENT_MANAGER_H
