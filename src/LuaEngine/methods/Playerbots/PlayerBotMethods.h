/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef PLAYERBOTMETHODS_H
#define PLAYERBOTMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"

/***
 * Inherits all methods from: [Object], [WorldObject], [Unit], [Player]
 */
namespace LuaPlayerBot
{

    /**
     * Returns `true` if the [Player] is driven by a playerbot AI, `false` otherwise.
     *
     * Unlike [Player]:IsBot, which only looks at the session, this checks that a
     * playerbot AI is actually attached to the [Player].
     *
     * @return bool isPlayerbot
     */
    int IsPlayerbot(lua_State* L, Player* player)
    {
        ALE::Push(L, player && sPlayerbotsMgr.GetPlayerbotAI(player) != nullptr);
        return 1;
    }

    /**
     * Returns the [PlayerbotAI] driving the [Player], or `nil` when it is not a bot.
     *
     * The returned object is only valid for the duration of the current call, do not
     * store it across hooks, fetch it again instead.
     *
     * @return [PlayerbotAI] botAI
     */
    int GetBotAI(lua_State* L, Player* player)
    {
        ALE::Push(L, player ? sPlayerbotsMgr.GetPlayerbotAI(player) : nullptr);
        return 1;
    }

    /**
     * Returns `true` if the [Player] is a real (human controlled) player, `false` otherwise.
     *
     * @return bool isRealPlayer
     */
    int IsRealPlayer(lua_State* L, Player* player)
    {
        ALE::Push(L, ::IsRealPlayer(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is a self bot, a bot logged in on its owner's own account.
     *
     * @return bool isSelfBot
     */
    int IsSelfBot(lua_State* L, Player* player)
    {
        ALE::Push(L, ::IsSelfBot(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is a random bot handled by the RandomPlayerbotMgr.
     *
     * @return bool isRandomBot
     */
    int IsRandomBot(lua_State* L, Player* player)
    {
        ALE::Push(L, sRandomPlayerbotMgr.IsRandomBot(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is a bot summoned through the addclass command.
     *
     * @return bool isAddclassBot
     */
    int IsAddclassBot(lua_State* L, Player* player)
    {
        ALE::Push(L, sRandomPlayerbotMgr.IsAddclassBot(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is an alt bot summoned by a real player.
     *
     * @return bool isAltBot
     */
    int IsAltBot(lua_State* L, Player* player)
    {
        PlayerbotAI* botAI = player ? sPlayerbotsMgr.GetPlayerbotAI(player) : nullptr;
        ALE::Push(L, botAI ? botAI->IsAltBot() : false);
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a tank by the playerbot AI.
     *
     * @param bool bySpec = false : judge by talent spec instead of the active strategies
     * @return bool isTank
     */
    int IsTank(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsTank(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a healer by the playerbot AI.
     *
     * @param bool bySpec = false
     * @return bool isHealer
     */
    int IsHealer(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsHeal(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a dps by the playerbot AI.
     *
     * @param bool bySpec = false
     * @return bool isDps
     */
    int IsDps(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsDps(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a ranged by the playerbot AI.
     *
     * @param bool bySpec = false
     * @return bool isRanged
     */
    int IsRanged(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsRanged(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a melee by the playerbot AI.
     *
     * @param bool bySpec = false
     * @return bool isMelee
     */
    int IsMelee(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsMelee(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a caster by the playerbot AI.
     *
     * @param bool bySpec = false
     * @return bool isCaster
     */
    int IsCaster(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsCaster(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is considered a ranged dps by the playerbot AI.
     *
     * @param bool bySpec = false
     * @return bool isRangedDps
     */
    int IsRangedDps(lua_State* L, Player* player)
    {
        bool bySpec = ALE::CHECKVAL<bool>(L, 2, false);
        ALE::Push(L, PlayerbotAI::IsRangedDps(player, bySpec));
        return 1;
    }

    /**
     * Returns `true` if the [Player] uses combo points.
     *
     * @return bool isCombo
     */
    int IsCombo(lua_State* L, Player* player)
    {
        ALE::Push(L, PlayerbotAI::IsCombo(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is the main tank of its group.
     *
     * @return bool isMainTank
     */
    int IsMainTank(lua_State* L, Player* player)
    {
        ALE::Push(L, PlayerbotAI::IsMainTank(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is explicitly flagged as main tank of its group.
     *
     * @return bool isExplicitMainTank
     */
    int IsExplicitMainTank(lua_State* L, Player* player)
    {
        ALE::Push(L, PlayerbotAI::IsExplicitMainTank(player));
        return 1;
    }

    /**
     * Returns `true` if the [Player] is an assist tank of its group.
     *
     * @return bool isAssistTank
     */
    int IsAssistTank(lua_State* L, Player* player)
    {
        ALE::Push(L, PlayerbotAI::IsAssistTank(player));
        return 1;
    }

    /**
     * Returns the assist tank index of the [Player] in its group, or -1.
     *
     * @return int32 index
     */
    int GetAssistTankIndex(lua_State* L, Player* player)
    {
        ALE::Push(L, PlayerbotAI::GetAssistTankIndex(player));
        return 1;
    }

    /**
     * Returns the amount of tanks in the [Player]s group.
     *
     * @return uint32 tankCount
     */
    int GetGroupTankCount(lua_State* L, Player* player)
    {
        ALE::Push(L, PlayerbotAI::GetGroupTankNum(player));
        return 1;
    }

    /**
     * Returns the GUID of the main tank of the [Player]s group, or `nil` when ungrouped.
     *
     * @return ObjectGuid mainTankGUID
     */
    int GetGroupMainTankGUID(lua_State* L, Player* player)
    {
        Group* group = player->GetGroup();
        if (!group)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, PlayerbotAI::GetMainTankGuid(group));
        return 1;
    }

    /**
     * Returns the gear score of the [Player] as computed by the playerbot AI.
     *
     * @param bool withBags = false : also weigh the items carried in the bags
     * @param bool withBank = false : also weigh the items stored in the bank
     * @param uint32 topN = 0 : only keep the topN best items, 0 to keep them all
     * @return uint32 gearScore
     */
    int GetMixedGearScore(lua_State* L, Player* player)
    {
        bool withBags = ALE::CHECKVAL<bool>(L, 2, false);
        bool withBank = ALE::CHECKVAL<bool>(L, 3, false);
        uint32 topN = ALE::CHECKVAL<uint32>(L, 4, 0);

        ALE::Push(L, PlayerbotAI::GetMixedGearScore(player, withBags, withBank, topN));
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // PLAYERBOTMETHODS_H
