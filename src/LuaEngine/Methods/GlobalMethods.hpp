/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_GLOBAL_METHODS_H
#define _ALE_GLOBAL_METHODS_H

#include <sol/sol.hpp>
#include "EventManager.h"
#include "Common.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"

namespace ALE::Core
{
    class TimedEventManager;
    enum class TimedEventObjectType : uint8;
}

namespace ALE::Methods
{
    /**
     * @brief Global Lua functions for ALE event system
     */
    namespace GlobalMethods
    {
        /**
         * @brief Convert uint32 event ID to WorldEvent enum
         *
         * Server events use range 1-50 in EventManager.
         * Direct cast from uint32 to enum for type safety.
         */
        Hooks::WorldEvent ServerEventToType(uint32 event)
        {
            return static_cast<Hooks::WorldEvent>(event);
        }

        /**
         * @brief Convert uint32 event ID to PlayerEvent enum
         *
         * Player events start at 100 in EventManager.
         * Direct cast from uint32 to enum for type safety.
         */
        Hooks::PlayerEvent PlayerEventToType(uint32 event)
        {
            return static_cast<Hooks::PlayerEvent>(event);
        }

        /**
         * @brief Register a server/world event handler
         *
         * Registers a Lua callback for world events (server start, config reload, etc.).
         *
         * @param event Event ID (WorldEvent enum as uint32)
         * @param handler Lua callback function(eventId, ...)
         * @param shots Number of times to call (0 = infinite)
         * @return Handler ID for cancellation
         *
         * @example Lua:
         * ```lua
         * RegisterServerEvent(1, function(eventId)
         *     print("Server event triggered!")
         * end)
         * ```
         */
        uint64 RegisterServerEvent(uint32 event, sol::protected_function handler, sol::optional<uint32> shots)
        {
            auto eventType = ServerEventToType(event);
            return Core::EventManager::GetInstance().RegisterGlobalEvent(
                eventType,
                std::move(handler),
                shots.value_or(0)
            );
        }

        /**
         * @brief Register a player event handler
         *
         * Registers a Lua callback for player events (login, logout, level up, etc.).
         *
         * @param event Event ID (PlayerEvent enum as uint32)
         * @param handler Lua callback function(eventId, player, ...)
         * @param shots Number of times to call (0 = infinite)
         * @return Handler ID for cancellation
         *
         * @example Lua:
         * ```lua
         * RegisterPlayerEvent(2, function(eventId, player)
         *     print(player:GetName() .. " logged in!")
         * end)
         * ```
         */
        uint64 RegisterPlayerEvent(uint32 event, sol::protected_function handler, sol::optional<uint32> shots)
        {
            auto eventType = PlayerEventToType(event);
            return Core::EventManager::GetInstance().RegisterGlobalEvent(
                eventType,
                std::move(handler),
                shots.value_or(0)
            );
        }

        /**
         * @brief Cancel a specific event handler by ID
         *
         * Removes a single handler registered via RegisterXEvent().
         *
         * @param handlerId Handler ID returned by RegisterXEvent
         * @return true if handler was found and cancelled, false otherwise
         *
         * @example Lua:
         * ```lua
         * local id = RegisterPlayerEvent(2, callback)
         * CancelEvent(id)  -- Remove this specific handler
         * ```
         */
        bool CancelEvent(uint64 handlerId)
        {
            return Core::EventManager::GetInstance().CancelEvent(handlerId);
        }

        /**
         * @brief Cancel all handlers for a server event
         *
         * Removes ALL handlers registered for a specific server event ID.
         *
         * @param event Event ID (WorldEvent enum as uint32)
         *
         * @example Lua:
         * ```lua
         * CancelServerEvent(1)  -- Remove all handlers for event 1
         * ```
         */
        void CancelServerEvent(uint32 event)
        {
            auto eventType = ServerEventToType(event);
            Core::EventManager::GetInstance().CancelGlobalEvent(eventType);
        }

        /**
         * @brief Cancel all handlers for a player event
         *
         * Removes ALL handlers registered for a specific player event ID.
         *
         * @param event Event ID (PlayerEvent enum as uint32)
         *
         * @example Lua:
         * ```lua
         * CancelPlayerEvent(2)  -- Remove all login handlers
         * ```
         */
        void CancelPlayerEvent(uint32 event)
        {
            auto eventType = PlayerEventToType(event);
            Core::EventManager::GetInstance().CancelGlobalEvent(eventType);
        }

        /**
         * @brief Create a global timed event (delayed/repeating callback)
         *
         * Registers a callback to execute after a delay, optionally repeating.
         *
         * @param callback Lua function to call
         * @param delay Delay in milliseconds before first execution
         * @param repeats Number of times to repeat (1 = once, 0 = infinite)
         * @param mgr TimedEventManager for this state
         * @return Event ID for cancellation
         *
         * @example Lua:
         * ```lua
         * CreateLuaEvent(function()
         *     print("5 seconds elapsed!")
         * end, 5000, 1)  -- 5000ms delay, execute once
         * ```
         */
        uint64 CreateLuaEvent(sol::function callback, uint32 delay, sol::optional<uint32> repeats, Core::TimedEventManager* mgr)
        {
            uint32 repeatCount = repeats.value_or(1);
            sol::protected_function pf(callback);
            return mgr->RegisterGlobalEvent(std::move(pf), delay, repeatCount);
        }

        /**
         * @brief Remove a specific timed event
         *
         * @param eventId Event ID returned by CreateLuaEvent
         * @param mgr TimedEventManager for this state
         * @return true if event was found and removed
         */
        bool RemoveTimedEvent(uint64 eventId, Core::TimedEventManager* mgr)
        {
            return mgr->RemoveEvent(eventId);
        }

        /**
         * @brief Remove all global timed events
         *
         * Clears all events registered via CreateLuaEvent.
         * Does NOT affect player/creature/gameobject events.
         *
         * @param mgr TimedEventManager for this state
         */
        void RemoveAllTimedEvents(Core::TimedEventManager* mgr)
        {
            mgr->RemoveAllGlobalEvents();
        }

        /**
         * @brief Get count of active global timed events
         *
         * @param mgr TimedEventManager for this state
         * @return Number of active global events
         */
        uint32 GetTimedEventCount(Core::TimedEventManager* mgr)
        {
            return mgr->GetGlobalEventCount();
        }

        // ========================================================================
        // OBJECT TIMED EVENT HELPERS (Player/Creature/GameObject)
        // ========================================================================

        /**
         * @brief Template helper to register timed event on an object
         *
         * Reduces code duplication across Player/Creature/GameObject.
         */
        template<typename T, Core::TimedEventObjectType ObjectType>
        uint64 RegisterObjectEvent(T* object, sol::function callback, uint32 delay, sol::optional<uint32> repeats, Core::TimedEventManager* mgr)
        {
            if (!object)
            {
                LOG_ERROR("ale.methods", "RegisterObjectEvent: Object is null");
                return 0;
            }

            uint32 repeatCount = repeats.value_or(1);
            sol::protected_function pf(callback);
            return mgr->RegisterObjectEvent(object->GetGUID(), std::move(pf), delay, repeatCount, ObjectType);
        }

        /**
         * @brief Template helper to remove a specific object event
         */
        template<typename T>
        bool RemoveObjectEvent(T* object, uint64 eventId, Core::TimedEventManager* mgr)
        {
            if (!object) return false;
            return mgr->RemoveEvent(eventId);
        }

        /**
         * @brief Template helper to remove all events from an object
         */
        template<typename T>
        void RemoveObjectEvents(T* object, Core::TimedEventManager* mgr)
        {
            if (!object) return;
            mgr->RemoveObjectEvents(object->GetGUID());
        }

        /**
         * @brief Template helper to get event count for an object
         */
        template<typename T>
        uint32 GetObjectEventCount(T* object, Core::TimedEventManager* mgr)
        {
            if (!object) return 0;
            return mgr->GetObjectEventCount(object->GetGUID());
        }
    } // namespace GlobalMethods

    /**
     * @brief Register global timed event functions to Lua state
     *
     * Registers CreateLuaEvent, RemoveTimedEvent, RemoveAllTimedEvents, GetTimedEventCount.
     */
    inline void RegisterGlobalTimedEventFunctions(sol::state& state, Core::TimedEventManager* mgr)
    {
        state["CreateLuaEvent"] = [mgr](sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            return GlobalMethods::CreateLuaEvent(std::move(callback), delay, repeats, mgr);
        };

        state["RemoveTimedEvent"] = [mgr](uint64 eventId) -> bool {
            return GlobalMethods::RemoveTimedEvent(eventId, mgr);
        };

        state["RemoveAllTimedEvents"] = [mgr]() {
            GlobalMethods::RemoveAllTimedEvents(mgr);
        };

        state["GetTimedEventCount"] = [mgr]() -> uint32 {
            return GlobalMethods::GetTimedEventCount(mgr);
        };
    }

    /**
     * @brief Helper to register timed event methods for an object type
     *
     * Template function to reduce code duplication across Player/Creature/GameObject.
     */
    template<typename T, Core::TimedEventObjectType ObjectType>
    inline void RegisterObjectTimedEventMethods(sol::state& state, const char* typeName, Core::TimedEventManager* mgr)
    {
        auto objectType = state[typeName].get_or_create<sol::table>();

        objectType["RegisterEvent"] = [mgr](T* obj, sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            return GlobalMethods::RegisterObjectEvent<T, ObjectType>(obj, std::move(callback), delay, repeats, mgr);
        };

        objectType["RemoveEvent"] = [mgr](T* obj, uint64 eventId) -> bool {
            return GlobalMethods::RemoveObjectEvent(obj, eventId, mgr);
        };

        objectType["RemoveEvents"] = [mgr](T* obj) {
            GlobalMethods::RemoveObjectEvents(obj, mgr);
        };

        objectType["GetEventCount"] = [mgr](T* obj) -> uint32 {
            return GlobalMethods::GetObjectEventCount(obj, mgr);
        };
    }

    /**
     * @brief Register all global functions to Lua state
     *
     * Registers all global event functions, timed event functions, and object methods.
     * Must be called with a valid TimedEventManager instance for timed events.
     *
     * @param state Lua state to register functions to
     * @param mgr TimedEventManager for timed events (nullptr = skip timed events)
     */
    inline void RegisterGlobalMethods(sol::state& state, Core::TimedEventManager* mgr = nullptr)
    {
        // Get the global table (_G)
        auto globalTable = state.globals();

        // Event system functions
        globalTable["RegisterServerEvent"] = &GlobalMethods::RegisterServerEvent;
        globalTable["RegisterPlayerEvent"] = &GlobalMethods::RegisterPlayerEvent;

        globalTable["CancelEvent"] = &GlobalMethods::CancelEvent;
        globalTable["CancelServerEvent"] = &GlobalMethods::CancelServerEvent;
        globalTable["CancelPlayerEvent"] = &GlobalMethods::CancelPlayerEvent;

        // Global timed events
        globalTable["CreateLuaEvent"] = [mgr](sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            return GlobalMethods::CreateLuaEvent(std::move(callback), delay, repeats, mgr);
        };

        globalTable["RemoveTimedEvent"] = [mgr](uint64 eventId) -> bool {
            return GlobalMethods::RemoveTimedEvent(eventId, mgr);
        };

        globalTable["RemoveAllTimedEvents"] = [mgr]() {
            GlobalMethods::RemoveAllTimedEvents(mgr);
        };

        globalTable["GetTimedEventCount"] = [mgr]() -> uint32 {
            return GlobalMethods::GetTimedEventCount(mgr);
        };

        // Object-specific timed event methods (Player:RegisterEvent, etc.)
        RegisterObjectTimedEventMethods<Player, Core::TimedEventObjectType::PLAYER>(state, "Player", mgr);
        RegisterObjectTimedEventMethods<Creature, Core::TimedEventObjectType::CREATURE>(state, "Creature", mgr);
        RegisterObjectTimedEventMethods<GameObject, Core::TimedEventObjectType::GAMEOBJECT>(state, "GameObject", mgr);
    }

} // namespace ALE::Methods

#endif // _ALE_GLOBAL_METHODS_H
