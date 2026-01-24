/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_PLAYER_METHODS_H
#define _ALE_PLAYER_METHODS_H

#include <sol/sol.hpp>
#include "Player.h"
#include "Unit.h"
#include "Chat.h"
#include "Group.h"
#include "Log.h"

namespace ALE::Methods
{
    namespace PlayerMethods
    {
        std::string GetName(Player* player)
        {
            return player->GetName();
        }

        uint64 GetGUID(Player* player)
        {
            return player->GetGUID().GetRawValue();
        }

        uint8 GetRace(Player* player)
        {
            return player->getRace();
        }
    } // namespace PlayerMethods

    /**
     * @brief Register Player methods to Lua state
     *
     * IMPORTANT: Player inherits from Unit via sol::bases<Unit>()
     * This means ALL Unit methods are automatically available on Player objects!
     *
     * MUST be called AFTER RegisterUnitMethods()!
     *
     * @param state Lua state to register Player type to
     */
    inline void RegisterPlayerMethods(sol::state& state)
    {
        auto playerType = state.new_usertype<Player>("Player",
            sol::no_constructor,
            sol::base_classes,
            sol::bases<Unit>()
        );

        // Getters
        playerType["GetName"] = &PlayerMethods::GetName;
        playerType["GetGUID"] = &PlayerMethods::GetGUID;
        playerType["GetRace"] = &PlayerMethods::GetRace;

        // Boolean
        // playerType["HasItem"] = &PlayerMethods::HasItem;
        // playerType["HasQuest"] = &PlayerMethods::HasQuest;
        // playerType["HasQuestCompleted"] = &PlayerMethods::HasQuestCompleted;
        // playerType["HasAchieved"] = &PlayerMethods::HasAchieved;
        // playerType["HasSpell"] = &PlayerMethods::HasSpell;
        // playerType["HasTalent"] = &PlayerMethods::HasTalent;

        // playerType["IsGM"] = &PlayerMethods::IsGM;
        // playerType["IsInGroup"] = &PlayerMethods::IsInGroup;
        // playerType["IsInRaid"] = &PlayerMethods::IsInRaid;

        // Setters
        // playerType["SetMoney"] = &PlayerMethods::SetMoney;

        // Actions
        // playerType["ModifyMoney"] = &PlayerMethods::ModifyMoney;
        // playerType["SendBroadcastMessage"] = &PlayerMethods::SendBroadcastMessage;
        // playerType["SendNotification"] = &PlayerMethods::SendNotification;
        // playerType["SendAreaTriggerMessage"] = &PlayerMethods::SendAreaTriggerMessage;
        // playerType["Teleport"] = &PlayerMethods::Teleport;
        // playerType["ResurrectPlayer"] = &PlayerMethods::ResurrectPlayer;
        // playerType["Kill"] = &PlayerMethods::Kill;
        // playerType["SaveToDB"] = &PlayerMethods::SaveToDB;
    }

} // namespace ALE::Methods

#endif // _ALE_PLAYER_METHODS_H
