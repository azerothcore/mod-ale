/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef RANDOMPLAYERBOTMGRMETHODS_H
#define RANDOMPLAYERBOTMGRMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"

namespace LuaRandomPlayerBotMgr
{
    /**
     * Returns `true` if the given [Player] is a random bot.
     *
     * @param [Player]|uint32 bot : a [Player] or a character low GUID
     * @return bool isRandomBot
     */
    int IsRandomBot(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        if (lua_isnumber(L, 2))
        {
            uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
            ALE::Push(L, mgr->IsRandomBot(lowGuid));
        }
        else
        {
            Player* bot = ALE::CHECKOBJ<Player>(L, 2);
            ALE::Push(L, mgr->IsRandomBot(bot));
        }
        return 1;
    }

    /**
     * Returns `true` if the given [Player] is an addclass bot.
     *
     * @param [Player]|uint32 bot : a [Player] or a character low GUID
     * @return bool isAddclassBot
     */
    int IsAddclassBot(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        if (lua_isnumber(L, 2))
        {
            uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
            ALE::Push(L, mgr->IsAddclassBot(lowGuid));
        }
        else
        {
            Player* bot = ALE::CHECKOBJ<Player>(L, 2);
            ALE::Push(L, mgr->IsAddclassBot(bot));
        }
        return 1;
    }

    /**
     * Fully randomizes the given bot: level, gear, talents and strategies.
     *
     * @param [Player] bot
     */
    int Randomize(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->Randomize(bot);
        return 0;
    }

    /**
     * Randomizes the given bot as if it had just logged in for the first time.
     *
     * @param [Player] bot
     */
    int RandomizeFirst(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->RandomizeFirst(bot);
        return 0;
    }

    /**
     * Randomizes the given bot at the minimum configured level.
     *
     * @param [Player] bot
     */
    int RandomizeMin(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->RandomizeMin(bot);
        return 0;
    }

    /**
     * Clears the stored randomization data of the given bot.
     *
     * @param [Player] bot
     */
    int Clear(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->Clear(bot);
        return 0;
    }

    /**
     * Refreshes the given bot: heals it and restores its consumables and buffs.
     *
     * @param [Player] bot
     */
    int Refresh(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->Refresh(bot);
        return 0;
    }

    /**
     * Raises the level of the given bot by one and re-equips it accordingly.
     *
     * @param [Player] bot
     */
    int IncreaseLevel(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->IncreaseLevel(bot);
        return 0;
    }

    /**
     * Revives the given bot at the closest graveyard.
     *
     * @param [Player] bot
     */
    int Revive(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->Revive(bot);
        return 0;
    }

    /**
     * Removes the given bot from the random bot pool.
     *
     * @param [Player] bot
     */
    int Remove(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->Remove(bot);
        return 0;
    }

    /**
     * Runs one AI processing pass on the given bot.
     *
     * @param [Player] bot
     * @return bool processed
     */
    int ProcessBot(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, mgr->ProcessBot(bot));
        return 1;
    }

    /**
     * Randomly teleports the given bot to a spot suited to its level.
     *
     * @param [Player] bot
     */
    int RandomTeleportForLevel(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->RandomTeleportForLevel(bot);
        return 0;
    }

    /**
     * Randomly teleports the given bot to a grinding spot suited to its level.
     *
     * @param [Player] bot
     */
    int RandomTeleportGrindForLevel(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->RandomTeleportGrindForLevel(bot);
        return 0;
    }

    /**
     * Randomly teleports the given bot to an rpg spot.
     *
     * @param [Player] bot
     */
    int RandomTeleportForRpg(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->RandomTeleportForRpg(bot);
        return 0;
    }

    /**
     * Schedules a random teleport for the given bot.
     *
     * @param uint32 lowGuid : the character low GUID of the bot
     * @param uint32 delay = 0 : delay in seconds
     */
    int ScheduleTeleport(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
        uint32 delay = ALE::CHECKVAL<uint32>(L, 3, 0);

        mgr->ScheduleTeleport(lowGuid, delay);
        return 0;
    }

    /**
     * Schedules a strategy change for the given bot.
     *
     * @param uint32 lowGuid : the character low GUID of the bot
     * @param uint32 delay = 0 : delay in seconds
     */
    int ScheduleChangeStrategy(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
        uint32 delay = ALE::CHECKVAL<uint32>(L, 3, 0);

        mgr->ScheduleChangeStrategy(lowGuid, delay);
        return 0;
    }

    /**
     * Randomly changes the strategies of the given bot.
     *
     * @param [Player] bot
     */
    int ChangeStrategy(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->ChangeStrategy(bot);
        return 0;
    }

    /**
     * Applies a one shot random strategy change to the given bot.
     *
     * @param [Player] bot
     */
    int ChangeStrategyOnce(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        mgr->ChangeStrategyOnce(bot);
        return 0;
    }

    /**
     * Sends a chat command to every random bot as if a [Player] had said it.
     *
     * @param uint32 chatType
     * @param string text
     * @param [Player] from
     * @param string channelName = ""
     */
    int HandleCommand(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        uint32 type = ALE::CHECKVAL<uint32>(L, 2);
        std::string text = ALE::CHECKVAL<std::string>(L, 3);
        Player* from = ALE::CHECKOBJ<Player>(L, 4);
        std::string channelName = ALE::CHECKVAL<std::string>(L, 5, "");

        mgr->HandleCommand(type, text, from, channelName);
        return 0;
    }

    /**
     * Runs a remote random bot command and returns its textual answer.
     *
     * @param string request
     * @return string result
     */
    int HandleRemoteCommand(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        std::string request = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, mgr->HandleRemoteCommand(request));
        return 1;
    }

    /**
     * Returns a random real [Player] currently online, or `nil`.
     *
     * @return [Player] player
     */
    int GetRandomPlayer(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        ALE::Push(L, mgr->GetRandomPlayer());
        return 1;
    }

    /**
     * Returns a table with the real [Player]s tracked by the random bot manager.
     *
     * @return table players
     */
    int GetPlayers(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (Player* player : mgr->GetPlayers())
        {
            ALE::Push(L, player);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns a table with every random bot [Player] currently logged in.
     *
     * @return table bots
     */
    int GetAllBots(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (auto const& pair : mgr->GetAllBots())
        {
            ALE::Push(L, pair.second);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns how many random bots are currently active.
     *
     * @return uint32 activeBots
     */
    int GetActiveBotsCount(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        ALE::Push(L, mgr->activeBots);
        return 1;
    }

    /**
     * Returns the maximum amount of random bots allowed by the configuration.
     *
     * @return uint32 maxBotCount
     */
    int GetMaxAllowedBotCount(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        ALE::Push(L, mgr->GetMaxAllowedBotCount());
        return 1;
    }

    /**
     * Returns the price multiplier the given bot applies when buying.
     *
     * @param [Player] bot
     * @return double multiplier
     */
    int GetBuyMultiplier(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, mgr->GetBuyMultiplier(bot));
        return 1;
    }

    /**
     * Returns the price multiplier the given bot applies when selling.
     *
     * @param [Player] bot
     * @return double multiplier
     */
    int GetSellMultiplier(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, mgr->GetSellMultiplier(bot));
        return 1;
    }

    /**
     * Returns the trade discount the given bot grants to the given master.
     *
     * @param [Player] bot
     * @param [Player] master
     * @return uint32 discount
     */
    int GetTradeDiscount(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        Player* master = ALE::CHECKOBJ<Player>(L, 3);

        ALE::Push(L, mgr->GetTradeDiscount(bot, master));
        return 1;
    }

    /**
     * Sets the trade discount the given bot grants to the given master.
     *
     * @param [Player] bot
     * @param [Player] master
     * @param uint32 value
     */
    int SetTradeDiscount(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        Player* master = ALE::CHECKOBJ<Player>(L, 3);
        uint32 value = ALE::CHECKVAL<uint32>(L, 4);

        mgr->SetTradeDiscount(bot, master, value);
        return 0;
    }

    /**
     * Adds the given amount to the trade discount the bot grants to the master.
     *
     * @param [Player] bot
     * @param [Player] master
     * @param int32 value
     */
    int AddTradeDiscount(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        Player* master = ALE::CHECKOBJ<Player>(L, 3);
        int32 value = ALE::CHECKVAL<int32>(L, 4);

        mgr->AddTradeDiscount(bot, master, value);
        return 0;
    }

    /**
     * Returns a value stored by the random bot manager for the given bot.
     *
     * Known types are for example "add", "logout", "teleport", "randomize" or "level".
     *
     * @param [Player]|uint32 bot : a [Player] or a character low GUID
     * @param string type
     * @return uint32 value
     */
    int GetValue(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        std::string type = ALE::CHECKVAL<std::string>(L, 3);

        if (lua_isnumber(L, 2))
        {
            uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
            ALE::Push(L, mgr->GetValue(lowGuid, type));
        }
        else
        {
            Player* bot = ALE::CHECKOBJ<Player>(L, 2);
            ALE::Push(L, mgr->GetValue(bot, type));
        }
        return 1;
    }

    /**
     * Returns the free form data stored alongside a value for the given bot.
     *
     * @param uint32 lowGuid : the character low GUID of the bot
     * @param string type
     * @return string data
     */
    int GetData(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
        std::string type = ALE::CHECKVAL<std::string>(L, 3);

        ALE::Push(L, mgr->GetData(lowGuid, type));
        return 1;
    }

    /**
     * Stores a value for the given bot in the random bot manager.
     *
     * @param [Player]|uint32 bot : a [Player] or a character low GUID
     * @param string type
     * @param uint32 value
     * @param string data = ""
     */
    int SetValue(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        std::string type = ALE::CHECKVAL<std::string>(L, 3);
        uint32 value = ALE::CHECKVAL<uint32>(L, 4);
        std::string data = ALE::CHECKVAL<std::string>(L, 5, "");

        if (lua_isnumber(L, 2))
        {
            uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
            mgr->SetValue(lowGuid, type, value, data);
        }
        else
        {
            Player* bot = ALE::CHECKOBJ<Player>(L, 2);
            mgr->SetValue(bot, type, value, data);
        }
        return 0;
    }

    /**
     * Returns `true` if the given bot uses a pvp spec.
     *
     * @param uint32 lowGuid : the character low GUID of the bot
     * @param uint8 class
     * @return bool isSpecPvp
     */
    int IsSpecPvp(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
        uint8 cls = ALE::CHECKVAL<uint8>(L, 3);

        ALE::Push(L, mgr->IsSpecPvp(lowGuid, cls));
        return 1;
    }

    /**
     * Returns the GUID of the battlemaster the given bot would queue with.
     *
     * @param [Player] bot
     * @param uint32 bgTypeId
     * @return ObjectGuid battleMasterGUID
     */
    int GetBattleMasterGUID(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        Player* bot = ALE::CHECKOBJ<Player>(L, 2);
        uint32 bgTypeId = ALE::CHECKVAL<uint32>(L, 3);

        ALE::Push(L, mgr->GetBattleMasterGUID(bot, static_cast<BattlegroundTypeId>(bgTypeId)));
        return 1;
    }

    /**
     * Returns the current activity modifier applied to every random bot.
     *
     * @return float activityMod
     */
    int GetActivityMod(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        ALE::Push(L, mgr->getActivityMod());
        return 1;
    }

    /**
     * Returns the current activity modifier as a percentage.
     *
     * @return float activityPercentage
     */
    int GetActivityPercentage(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        ALE::Push(L, mgr->getActivityPercentage());
        return 1;
    }

    /**
     * Sets the activity modifier applied to every random bot, as a percentage.
     *
     * @param float percentage
     */
    int SetActivityPercentage(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        float percentage = ALE::CHECKVAL<float>(L, 2);
        mgr->setActivityPercentage(percentage);
        return 0;
    }

    /**
     * Returns the cache index matching the given faction and class.
     *
     * @param bool isAlliance
     * @param uint8 class
     * @return uint8 index
     */
    int GetTeamClassIdx(lua_State* L, RandomPlayerbotMgr* /*mgr*/)
    {
        bool isAlliance = ALE::CHECKVAL<bool>(L, 2);
        uint8 cls = ALE::CHECKVAL<uint8>(L, 3);

        ALE::Push(L, RandomPlayerbotMgr::GetTeamClassIdx(isAlliance, cls));
        return 1;
    }

    /**
     * Returns `true` if the given account is of the given playerbot account type.
     *
     * @param uint32 accountId
     * @param uint8 accountType
     * @return bool isAccountType
     */
    int IsAccountType(lua_State* L, RandomPlayerbotMgr* mgr)
    {
        uint32 accountId = ALE::CHECKVAL<uint32>(L, 2);
        uint8 accountType = ALE::CHECKVAL<uint8>(L, 3);

        ALE::Push(L, mgr->IsAccountType(accountId, accountType));
        return 1;
    }

    /**
     * Assigns the playerbot account types to every bot account.
     */
    int AssignAccountTypes(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->AssignAccountTypes();
        return 0;
    }

    /**
     * Rebuilds the addclass bot cache.
     */
    int PrepareAddclassCache(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->PrepareAddclassCache();
        return 0;
    }

    /**
     * Reloads the battlemaster cache from the world database.
     */
    int LoadBattleMastersCache(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->LoadBattleMastersCache();
        return 0;
    }

    /**
     * Creates the arena teams the random bots compete in.
     */
    int InitArenaTeams(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->InitArenaTeams();
        return 0;
    }

    /**
     * Re-evaluates the battleground queues of the random bots.
     */
    int CheckBgQueue(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->CheckBgQueue();
        return 0;
    }

    /**
     * Re-evaluates the lfg queues of the random bots.
     */
    int CheckLfgQueue(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->CheckLfgQueue();
        return 0;
    }

    /**
     * Re-evaluates the list of real players the random bots react to.
     */
    int CheckPlayers(lua_State* /*L*/, RandomPlayerbotMgr* mgr)
    {
        mgr->CheckPlayers();
        return 0;
    }
};

namespace LuaRandomPlayerBotMgrGlobal
{
    /**
     * Returns the [RandomPlayerbotMgr] singleton, which drives the random bot pool.
     *
     * @return [RandomPlayerbotMgr] randomPlayerbotMgr
     */
    int GetRandomPlayerbotMgr(lua_State* L)
    {
        ALE::Push(L, &sRandomPlayerbotMgr);
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // RANDOMPLAYERBOTMGRMETHODS_H
