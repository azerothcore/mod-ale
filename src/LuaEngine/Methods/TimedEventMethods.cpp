/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "TimedEventMethods.h"
#include "TimedEventManager.h"
#include "Log.h"

namespace Eclipse::Methods::TimedEventMethods {

    // NOTE: All implementations use lambda closures to capture the per-state TimedEventManager*
    // This is set up in RegisterTimedEventMethods()

    void RegisterTimedEventMethods(sol::state& state, Core::TimedEventManager* mgr) {
        // Capture the manager pointer in lambdas so each state has its own manager
        
        // NOTE: Global timed event functions (CreateLuaEvent, RemoveTimedEvent, etc.)
        //       are now registered in GlobalMethods to keep all global functions centralized.
        //       This file only handles Player/Creature/GameObject method bindings.
        
        // =======================================================================
        // PLAYER TIMED EVENTS
        // =======================================================================
        
        state["Player"]["RegisterEvent"] = [mgr](Player* player, sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            if (!player) {
                LOG_ERROR("scripts.eclipse", "Player_RegisterEvent: Player is null");
                return 0;
            }
            uint32 repeatCount = repeats.value_or(1);
            LOG_INFO("scripts.eclipse", "Player:RegisterEvent called for player {}: delay={}ms, repeats={}", 
                     player->GetName(), delay, repeatCount);
            
            sol::protected_function pf(callback);
            return mgr->RegisterObjectEvent(player->GetGUID(), std::move(pf), delay, repeatCount, Core::TimedEventObjectType::PLAYER);
        };
        
        state["Player"]["RemoveEvent"] = [mgr](Player* player, uint64 eventId) -> bool {
            if (!player) return false;
            return mgr->RemoveEvent(eventId);
        };
        
        state["Player"]["RemoveEvents"] = [mgr](Player* player) {
            if (!player) return;
            mgr->RemoveObjectEvents(player->GetGUID());
        };
        
        state["Player"]["GetEventCount"] = [mgr](Player* player) -> uint32 {
            if (!player) return 0;
            return mgr->GetObjectEventCount(player->GetGUID());
        };

        // =======================================================================
        // CREATURE TIMED EVENTS
        // =======================================================================
        
        auto creatureType = state["Creature"].get_or_create<sol::table>();
        
        creatureType["RegisterEvent"] = [mgr](Creature* creature, sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            if (!creature) {
                LOG_ERROR("scripts.eclipse", "Creature_RegisterEvent: Creature is null");
                return 0;
            }
            uint32 repeatCount = repeats.value_or(1);
            
            sol::protected_function pf(callback);
            return mgr->RegisterObjectEvent(creature->GetGUID(), std::move(pf), delay, repeatCount, Core::TimedEventObjectType::CREATURE);
        };
        
        creatureType["RemoveEvent"] = [mgr](Creature* creature, uint64 eventId) -> bool {
            if (!creature) return false;
            return mgr->RemoveEvent(eventId);
        };
        
        creatureType["RemoveEvents"] = [mgr](Creature* creature) {
            if (!creature) return;
            mgr->RemoveObjectEvents(creature->GetGUID());
        };
        
        creatureType["GetEventCount"] = [mgr](Creature* creature) -> uint32 {
            if (!creature) return 0;
            return mgr->GetObjectEventCount(creature->GetGUID());
        };

        // =======================================================================
        // GAMEOBJECT TIMED EVENTS
        // =======================================================================
        
        auto gameobjectType = state["GameObject"].get_or_create<sol::table>();
        
        gameobjectType["RegisterEvent"] = [mgr](GameObject* gameobject, sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            if (!gameobject) {
                LOG_ERROR("scripts.eclipse", "GameObject_RegisterEvent: GameObject is null");
                return 0;
            }
            uint32 repeatCount = repeats.value_or(1);
            
            sol::protected_function pf(callback);
            return mgr->RegisterObjectEvent(gameobject->GetGUID(), std::move(pf), delay, repeatCount, Core::TimedEventObjectType::GAMEOBJECT);
        };

        gameobjectType["RemoveEvent"] = [mgr](GameObject* gameobject, uint64 eventId) -> bool {
            if (!gameobject) return false;
            return mgr->RemoveEvent(eventId);
        };
        
        gameobjectType["RemoveEvents"] = [mgr](GameObject* gameobject) {
            if (!gameobject) return;
            mgr->RemoveObjectEvents(gameobject->GetGUID());
        };
        
        gameobjectType["GetEventCount"] = [mgr](GameObject* gameobject) -> uint32 {
            if (!gameobject) return 0;
            return mgr->GetObjectEventCount(gameobject->GetGUID());
        };

        LOG_DEBUG("scripts.eclipse", "Registered Player/Creature/GameObject timed event methods");
    }

} // namespace Eclipse::Methods::TimedEventMethods
