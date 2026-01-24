/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_PLAYER_HOOKS_H
#define _ALE_PLAYER_HOOKS_H

#include "ScriptMgr.h"
#include "Player.h"

#include "TimedEventManager.h"
#include "EventManager.h"

namespace ALE::Hooks
{
    /**
     * @brief Trigger player event helper (template implementation)
     *
     * Converts enum to uint32 and forwards to EventManager.
     *
     * @tparam Args Variadic argument types
     * @param eventType PlayerEvent enum value
     * @param args Arguments to forward to Lua callbacks
     */
    template<typename... Args>
    void TriggerPlayerEvent(Hooks::PlayerEvent eventType, Args&&... args)
    {
        // Cast enum to uint32
        uint32 eventId = static_cast<uint32>(eventType);

        // Trigger event
        Core::EventManager::GetInstance().TriggerGlobalEvent(
            eventType,
            eventId,
            std::forward<Args>(args)...
        );
    }

    /**
     * @class PlayerHooks
     * @brief AzerothCore player event interceptor for ALE Lua system
     * @note Registered in ALEScriptLoader via sScriptMgr->AddScript<PlayerHooks>()
     */
    class PlayerHooks : public PlayerScript
    {
    public:
        /**
         * @brief Constructor - registers PlayerHooks with AzerothCore
         */
        PlayerHooks() : PlayerScript("ALE_PlayerHooks") { }

        /**
         * @brief Player login event (PlayerEvent::ON_LOGIN (ID = 2))
         *
         * Called when player finishes loading into world.
         * Lua signature: function(eventId, player)
         *
         * @param player
         */
        void OnPlayerLogin(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LOGIN, player);
        }

        /**
         * @brief Player logout event (PlayerEvent::ON_LOGOUT (ID = 3))
         *
         * Called when player disconnects.
         * Lua signature: function(eventId, player)
         *
         * @param player
         */
        void OnPlayerLogout(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LOGOUT, player);
        }

        /**
         * @brief Level change event (PlayerEvent::ON_LEVEL_CHANGE (ID = 5))
         *
         * Called when player gains a level.
         * Lua signature: function(eventId, player, oldLevel)
         *
         * @param player
         * @param oldLevel
         */
        void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LEVEL_CHANGE, player, oldLevel);
        }

        /**
         * @brief Money change event (PlayerEvent::ON_MONEY_CHANGE (ID = 6))
         *
         * Called before player's money changes.
         * Lua signature: function(eventId, player, amount)
         *
         * @param player
         * @param amount
         */
        void OnPlayerMoneyChanged(Player* player, int32& amount) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_MONEY_CHANGE);
            uint32 modifiedAmount = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_MONEY_CHANGE,
                amount,        // Default value if no handlers
                eventId,
                player,
                amount
            );

            // Apply modified amount from Lua
            amount = modifiedAmount;
        }

        /**
         * @brief XP gain event (PlayerEvent::ON_GIVE_XP (ID = 7))
         *
         * Called when player is about to receive XP.
         * Lua can modify amount by reference.
         *
         * Lua signature: function(eventId, player, amount, victim, xpSource)
         *
         * @param player
         * @param amount
         * @param victim
         * @param xpSource
         */
        void OnPlayerGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GIVE_XP);
            uint32 modifiedAmount = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_GIVE_XP,
                amount,        // Default value if no handlers
                eventId,
                player,
                amount,
                victim,
                xpSource
            );

            // Apply modified amount from Lua
            amount = modifiedAmount;
        }

        /**
         * @brief Player update tick event (triggers ON_UPDATE)
         *
         * Called every world tick (typically ~100ms) per player.
         *
         * Lua signature: function(eventId, player, diff)
         *
         * @param player Player being updated
         * @param diff Time delta since last update (milliseconds)
         */
        void OnPlayerUpdate(Player* player, uint32 diff) override
        {
            // Early exit: player not valid or not in world
            if (!player || !player->IsInWorld())
                return;

            auto& stateMgr = Core::StateManager::GetInstance();
            Core::TimedEventManager* mgr = stateMgr.GetTimedEventManager(-1);

            // Execute timed events for this player
            if (mgr)
                mgr->UpdateObjectEvents(player, diff);
        }
    };

} // namespace ALE::Hooks

#endif // _ALE_PLAYER_HOOKS_H
