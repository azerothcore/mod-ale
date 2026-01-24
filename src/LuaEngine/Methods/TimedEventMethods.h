/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_TIMED_EVENT_METHODS_H
#define _ALE_TIMED_EVENT_METHODS_H

#include <sol/sol.hpp>
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"

namespace ALE::Core {
    class TimedEventManager;
}

namespace ALE::Methods {

    /**
     * @brief Global timed event methods exposed to Lua
     */
    namespace TimedEventMethods {
        
        // =======================================================================
        // GLOBAL TIMED EVENTS
        // =======================================================================
        
        /**
         * @brief Create a global timed event
         * @param callback Lua function to execute
         * @param delay Delay in milliseconds
         * @param repeats Number of times to repeat (0 = infinite)
         * @return Event ID
         * 
         * Example:
         *   local eventId = CreateLuaEvent(function(eventId, delay, repeats)
         *       print("Event executed!")
         *   end, 5000, 1) -- Execute once after 5 seconds
         */
        uint64 CreateLuaEvent(sol::protected_function callback, uint32 delay, sol::optional<uint32> repeats);
        
        /**
         * @brief Remove a global timed event
         * @param eventId Event ID returned by CreateLuaEvent
         * @return true if event was removed
         */
        bool RemoveTimedEvent(uint64 eventId);
        
        /**
         * @brief Remove all global timed events
         */
        void RemoveAllTimedEvents();
        
        /**
         * @brief Get the number of active global timed events
         */
        uint32 GetTimedEventCount();
        
        // =======================================================================
        // PLAYER TIMED EVENTS
        // =======================================================================
        
        /**
         * @brief Register a timed event for a player
         * @param player Player object
         * @param callback Lua function to execute
         * @param delay Delay in milliseconds
         * @param repeats Number of times to repeat (0 = infinite)
         * @return Event ID
         * 
         * Example:
         *   player:RegisterEvent(function(eventId, delay, repeats, player)
         *       player:SendBroadcastMessage("Buff reminder!")
         *   end, 60000, 0) -- Every minute indefinitely
         */
        uint64 Player_RegisterEvent(Player* player, sol::protected_function callback, uint32 delay, sol::optional<uint32> repeats);
        
        /**
         * @brief Remove a specific event from a player
         * @param player Player object
         * @param eventId Event ID
         * @return true if event was removed
         */
        bool Player_RemoveEvent(Player* player, uint64 eventId);
        
        /**
         * @brief Remove all events from a player
         * @param player Player object
         */
        void Player_RemoveEvents(Player* player);
        
        /**
         * @brief Get the number of active events for a player
         * @param player Player object
         */
        uint32 Player_GetEventCount(Player* player);
        
        // =======================================================================
        // CREATURE TIMED EVENTS
        // =======================================================================
        
        /**
         * @brief Register a timed event for a creature
         * @param creature Creature object
         * @param callback Lua function to execute
         * @param delay Delay in milliseconds
         * @param repeats Number of times to repeat (0 = infinite)
         * @return Event ID
         * 
         * Example:
         *   creature:RegisterEvent(function(eventId, delay, repeats, creature)
         *       creature:SendChatMessage("Patrol message")
         *   end, 30000, 0) -- Every 30 seconds
         */
        uint64 Creature_RegisterEvent(Creature* creature, sol::protected_function callback, uint32 delay, sol::optional<uint32> repeats);
        
        /**
         * @brief Remove a specific event from a creature
         * @param creature Creature object
         * @param eventId Event ID
         * @return true if event was removed
         */
        bool Creature_RemoveEvent(Creature* creature, uint64 eventId);
        
        /**
         * @brief Remove all events from a creature
         * @param creature Creature object
         */
        void Creature_RemoveEvents(Creature* creature);
        
        /**
         * @brief Get the number of active events for a creature
         * @param creature Creature object
         */
        uint32 Creature_GetEventCount(Creature* creature);
        
        // =======================================================================
        // GAMEOBJECT TIMED EVENTS
        // =======================================================================
        
        /**
         * @brief Register a timed event for a GameObject
         * @param gameobject GameObject object
         * @param callback Lua function to execute
         * @param delay Delay in milliseconds
         * @param repeats Number of times to repeat (0 = infinite)
         * @return Event ID
         * 
         * Example:
         *   gameobject:RegisterEvent(function(eventId, delay, repeats, gameobject)
         *       gameobject:SetGoState(1)
         *   end, 10000, 1) -- Open door after 10 seconds
         */
        uint64 GameObject_RegisterEvent(GameObject* gameobject, sol::protected_function callback, uint32 delay, sol::optional<uint32> repeats);
        
        /**
         * @brief Remove a specific event from a GameObject
         * @param gameobject GameObject object
         * @param eventId Event ID
         * @return true if event was removed
         */
        bool GameObject_RemoveEvent(GameObject* gameobject, uint64 eventId);
        
        /**
         * @brief Remove all events from a GameObject
         * @param gameobject GameObject object
         */
        void GameObject_RemoveEvents(GameObject* gameobject);
        
        /**
         * @brief Get the number of active events for a GameObject
         * @param gameobject GameObject object
         */
        uint32 GameObject_GetEventCount(GameObject* gameobject);
        
        // =======================================================================
        // REGISTRATION
        // =======================================================================
        
        /**
         * @brief Register all timed event methods with Lua state
         * @param state The Lua state to register methods with
         * @param timedEventManager The TimedEventManager instance for this state (per-state, not global)
         */
        void RegisterTimedEventMethods(sol::state& state, Core::TimedEventManager* timedEventManager);
        
    } // namespace TimedEventMethods

} // namespace ALE::Methods

#endif // _ALE_TIMED_EVENT_METHODS_H
