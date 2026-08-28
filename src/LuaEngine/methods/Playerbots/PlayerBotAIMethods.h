/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef PLAYERBOTAIMETHODS_H
#define PLAYERBOTAIMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"
#include "PlayerbotSecurity.h"
#include "RandomPlayerbotMgr.h"
#include "Strategy.h"

namespace LuaPlayerBotAI
{
    inline Player* GetBotOrNull(PlayerbotAI* botAI)
    {
        Player* bot = botAI->GetBot();
        return (bot && bot->IsInWorld() && !bot->IsDuringRemoveFromWorld()) ? bot : nullptr;
    }

    inline BotState CheckBotState(lua_State* L, int narg, BotState def = BOT_STATE_NON_COMBAT)
    {
        uint32 state = ALE::CHECKVAL<uint32>(L, narg, static_cast<uint32>(def));
        if (state >= static_cast<uint32>(BOT_STATE_MAX))
            state = static_cast<uint32>(def);
        return static_cast<BotState>(state);
    }

    inline PlayerbotSecurityLevel CheckSecurityLevel(lua_State* L, int narg)
    {
        uint32 level = ALE::CHECKVAL<uint32>(L, narg, static_cast<uint32>(PLAYERBOT_SECURITY_ALLOW_ALL));
        if (level > static_cast<uint32>(PLAYERBOT_SECURITY_ALLOW_ALL))
            level = static_cast<uint32>(PLAYERBOT_SECURITY_ALLOW_ALL);
        return static_cast<PlayerbotSecurityLevel>(level);
    }

    /**
     * Returns the bot [Player] this [PlayerbotAI] drives.
     *
     * @return [Player] bot
     */
    int GetBot(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->GetBot());
        return 1;
    }

    /**
     * Returns the [Player] that masters this bot, or `nil`.
     *
     * @return [Player] master
     */
    int GetMaster(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->GetMaster());
        return 1;
    }

    /**
     * Sets the master of this bot.
     *
     * @param [Player] master : can be nil to clear it
     */
    int SetMaster(lua_State* L, PlayerbotAI* botAI)
    {
        Player* master = ALE::CHECKOBJ<Player>(L, 2, false);
        botAI->SetMaster(master);
        return 0;
    }

    /**
     * Looks for a new master for this bot and returns it, or `nil`.
     *
     * @return [Player] master
     */
    int FindNewMaster(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->FindNewMaster());
        return 1;
    }

    /**
     * Returns the group leader of the bot, or its master when it is not grouped.
     *
     * @return [Player] leader
     */
    int GetGroupLeader(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->GetGroupLeader());
        return 1;
    }

    /**
     * Returns `true` if the bot currently has a master using a real game client.
     *
     * @return bool hasGameClientMaster
     */
    int HasGameClientMaster(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->HasGameClientMaster());
        return 1;
    }

    /**
     * Returns a stable pseudo random number for this bot in the range [0, maxNum).
     *
     * @param uint32 maxNum = 100
     * @return uint32 number
     */
    int GetFixedBotNumber(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 maxNum = ALE::CHECKVAL<uint32>(L, 2, 100);
        if (maxNum == 0)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetFixedBotNumber(maxNum));
        return 1;
    }

    /**
     * Returns the grouper type of the bot.
     *
     *     enum GrouperType
     *     {
     *         SOLO     = 0,
     *         MEMBER   = 1,
     *         LEADER_2 = 2,
     *         LEADER_3 = 3,
     *         LEADER_4 = 4,
     *         LEADER_5 = 5
     *     };
     *
     * @return uint32 grouperType
     */
    int GetGrouperType(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, static_cast<uint32>(botAI->GetGrouperType()));
        return 1;
    }

    /**
     * Returns the guilder type of the bot.
     *
     * @return uint32 guilderType
     */
    int GetGuilderType(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, static_cast<uint32>(botAI->GetGuilderType()));
        return 1;
    }

    /**
     * Returns the current state of the bot AI.
     *
     *     enum BotState
     *     {
     *         BOT_STATE_COMBAT     = 0,
     *         BOT_STATE_NON_COMBAT = 1,
     *         BOT_STATE_DEAD       = 2
     *     };
     *
     * @return uint32 botState
     */
    int GetState(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, static_cast<uint32>(botAI->GetState()));
        return 1;
    }

    /**
     * Switches the bot to the engine of the given state.
     *
     * @param uint32 botState : see [PlayerbotAI]:GetState
     */
    int ChangeEngine(lua_State* L, PlayerbotAI* botAI)
    {
        BotState state = CheckBotState(L, 2);
        botAI->ChangeEngine(state);
        return 0;
    }

    /**
     * Switches the bot to its combat engine.
     */
    int ChangeEngineOnCombat(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->ChangeEngineOnCombat();
        return 0;
    }

    /**
     * Switches the bot to its non combat engine.
     */
    int ChangeEngineOnNonCombat(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->ChangeEngineOnNonCombat();
        return 0;
    }

    /**
     * Reinitializes the currently active engine of the bot.
     */
    int ReInitCurrentEngine(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->ReInitCurrentEngine();
        return 0;
    }

    /**
     * Resets the bot AI.
     *
     * @param bool full = false : also reset strategies and cached values
     */
    int Reset(lua_State* L, PlayerbotAI* botAI)
    {
        bool full = ALE::CHECKVAL<bool>(L, 2, false);
        botAI->Reset(full);
        return 0;
    }

    /**
     * Makes the bot immediately evaluate and run its next action.
     *
     * @param bool minimal = false
     */
    int DoNextAction(lua_State* L, PlayerbotAI* botAI)
    {
        bool minimal = ALE::CHECKVAL<bool>(L, 2, false);
        botAI->DoNextAction(minimal);
        return 0;
    }

    /**
     * Runs a specific bot action by name, for example "attack my target" or "follow".
     *
     * @param string action
     * @param bool silent = false : do not report a failure to the master
     * @param string qualifier = "" : optional action qualifier
     * @return bool executed
     */
    int DoSpecificAction(lua_State* L, PlayerbotAI* botAI)
    {
        std::string action = ALE::CHECKVAL<std::string>(L, 2);
        bool silent = ALE::CHECKVAL<bool>(L, 3, false);
        std::string qualifier = ALE::CHECKVAL<std::string>(L, 4, "");

        ALE::Push(L, botAI->DoSpecificAction(action, Event(), silent, qualifier));
        return 1;
    }

    /**
     * Runs a remote playerbot command on this bot and returns its textual answer.
     *
     * @param string command
     * @return string result
     */
    int HandleRemoteCommand(lua_State* L, PlayerbotAI* botAI)
    {
        std::string command = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->HandleRemoteCommand(command));
        return 1;
    }

    /**
     * Sends a chat command to this bot as if another [Player] had said it.
     *
     * @param uint32 chatType : the ChatMsg type of the command
     * @param string text
     * @param [Player] from
     */
    int HandleCommand(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 type = ALE::CHECKVAL<uint32>(L, 2);
        std::string text = ALE::CHECKVAL<std::string>(L, 3);
        Player* from = ALE::CHECKOBJ<Player>(L, 4);

        botAI->HandleCommand(type, text, from);
        return 0;
    }

    /**
     * Returns `true` if the given command is allowed for this bot.
     *
     * @param string command
     * @return bool allowed
     */
    int IsAllowedCommand(lua_State* L, PlayerbotAI* botAI)
    {
        std::string command = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->IsAllowedCommand(command));
        return 1;
    }

    /**
     * Returns the reaction delay of the bot in milliseconds.
     *
     * @return uint32 reactDelay
     */
    int GetReactDelay(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->GetReactDelay());
        return 1;
    }

    /**
     * Returns a table with the names of all strategies active in the given state.
     *
     * @param uint32 botState = 1 : see [PlayerbotAI]:GetState
     * @return table strategies : a table of strings
     */
    int GetStrategies(lua_State* L, PlayerbotAI* botAI)
    {
        BotState state = CheckBotState(L, 2);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (std::string const& strategy : botAI->GetStrategies(state))
        {
            ALE::Push(L, strategy);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns `true` if the given strategy is active in the given state.
     *
     * @param string name
     * @param uint32 botState = 1 : see [PlayerbotAI]:GetState
     * @return bool hasStrategy
     */
    int HasStrategy(lua_State* L, PlayerbotAI* botAI)
    {
        std::string name = ALE::CHECKVAL<std::string>(L, 2);
        BotState state = CheckBotState(L, 3);

        ALE::Push(L, botAI->HasStrategy(name, state));
        return 1;
    }

    /**
     * Adds, removes or toggles a strategy for the given state.
     *
     * The name uses the usual playerbot syntax: `+name` to add, `-name` to remove
     * and `~name` to toggle.
     *
     * @param string name
     * @param uint32 botState = 1 : see [PlayerbotAI]:GetState
     */
    int ChangeStrategy(lua_State* L, PlayerbotAI* botAI)
    {
        std::string name = ALE::CHECKVAL<std::string>(L, 2);
        BotState state = CheckBotState(L, 3);

        botAI->ChangeStrategy(name, state);
        return 0;
    }

    /**
     * Removes every strategy of the given state.
     *
     * @param uint32 botState = 1 : see [PlayerbotAI]:GetState
     */
    int ClearStrategies(lua_State* L, PlayerbotAI* botAI)
    {
        BotState state = CheckBotState(L, 2);
        botAI->ClearStrategies(state);
        return 0;
    }

    /**
     * Resets the strategies of the given state, keeping the ones set by the master.
     *
     * @param uint32 botState = 1 : see [PlayerbotAI]:GetState
     */
    int SelectiveResetStrategies(lua_State* L, PlayerbotAI* botAI)
    {
        BotState state = CheckBotState(L, 2);
        botAI->SelectiveResetStrategies(state);
        return 0;
    }

    /**
     * Restores the default strategies of the bot.
     *
     * @param bool load = false : reload the saved strategies from the database
     */
    int ResetStrategies(lua_State* L, PlayerbotAI* botAI)
    {
        bool load = ALE::CHECKVAL<bool>(L, 2, false);
        botAI->ResetStrategies(load);
        return 0;
    }

    /**
     * Applies the instance specific strategies of the given map to the bot.
     *
     * @param uint32 mapId
     * @param bool tellMaster = false
     */
    int ApplyInstanceStrategies(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 mapId = ALE::CHECKVAL<uint32>(L, 2);
        bool tellMaster = ALE::CHECKVAL<bool>(L, 3, false);

        botAI->ApplyInstanceStrategies(mapId, tellMaster);
        return 0;
    }

    /**
     * Returns `true` if the bot has any strategy of the given type.
     *
     *     enum StrategyType
     *     {
     *          STRATEGY_TYPE_GENERIC = 0,
     *          STRATEGY_TYPE_COMBAT = 1,
     *          STRATEGY_TYPE_NONCOMBAT = 2,
     *          STRATEGY_TYPE_TANK = 4,
     *          STRATEGY_TYPE_DPS = 8,
     *          STRATEGY_TYPE_HEAL = 16,
     *          STRATEGY_TYPE_RANGED = 32,
     *          STRATEGY_TYPE_MELEE = 64
     *     };
     *
     * @param uint32 strategyType
     * @return bool containsStrategy
     */
    int ContainsStrategy(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 type = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->ContainsStrategy(static_cast<StrategyType>(type)));
        return 1;
    }

    /**
     * Re-evaluates whether a healer bot should also dps.
     */
    int EvaluateHealerDpsStrategy(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->EvaluateHealerDpsStrategy();
        return 0;
    }

    /**
     * Makes the bot leave its group, or disband it when it is the leader.
     */
    int LeaveOrDisbandGroup(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->LeaveOrDisbandGroup();
        return 0;
    }

    /**
     * Returns a table of every [Player] in the bots group, bots included.
     *
     * @return table players
     */
    int GetAllPlayersInGroup(lua_State* L, PlayerbotAI* botAI)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (Player* member : botAI->GetAllPlayersInGroup())
        {
            ALE::Push(L, member);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns a table of every real (human controlled) [Player] in the bots group.
     *
     * @return table players
     */
    int GetRealPlayersInGroup(lua_State* L, PlayerbotAI* botAI)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (Player* member : botAI->GetRealPlayersInGroup())
        {
            ALE::Push(L, member);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns the amount of group members near the bot.
     *
     * @param float distance = sight distance
     * @return int32 count
     */
    int GetNearGroupMemberCount(lua_State* L, PlayerbotAI* botAI)
    {
        float distance = ALE::CHECKVAL<float>(L, 2, sPlayerbotAIConfig.sightDistance);
        ALE::Push(L, botAI->GetNearGroupMemberCount(distance));
        return 1;
    }

    /**
     * Returns `true` if a real player is within the given range of the bot.
     *
     * @param float range = react distance
     * @return bool hasPlayerNearby
     */
    int HasPlayerNearby(lua_State* L, PlayerbotAI* botAI)
    {
        float range = ALE::CHECKVAL<float>(L, 2, sPlayerbotAIConfig.reactDistance);
        ALE::Push(L, botAI->HasPlayerNearby(range));
        return 1;
    }

    /**
     * Returns the group slot index of the given [Player], or of the bot itself.
     *
     * @param [Player] member = the bot itself
     * @return int32 index
     */
    int GetGroupSlotIndex(lua_State* L, PlayerbotAI* botAI)
    {
        Player* member = ALE::CHECKOBJ<Player>(L, 2, false);
        if (!member)
            member = GetBotOrNull(botAI);

        if (!member)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetGroupSlotIndex(member));
        return 1;
    }

    /**
     * Returns the melee index of the given [Player] in the group.
     *
     * @param [Player] member = the bot itself
     * @return int32 index
     */
    int GetMeleeIndex(lua_State* L, PlayerbotAI* botAI)
    {
        Player* member = ALE::CHECKOBJ<Player>(L, 2, false);
        if (!member)
            member = GetBotOrNull(botAI);

        if (!member)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetMeleeIndex(member));
        return 1;
    }

    /**
     * Returns the ranged index of the given [Player] in the group.
     *
     * @param [Player] member = the bot itself
     * @return int32 index
     */
    int GetRangedIndex(lua_State* L, PlayerbotAI* botAI)
    {
        Player* member = ALE::CHECKOBJ<Player>(L, 2, false);
        if (!member)
            member = GetBotOrNull(botAI);

        if (!member)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetRangedIndex(member));
        return 1;
    }

    /**
     * Returns the ranged dps index of the given [Player] in the group.
     *
     * @param [Player] member = the bot itself
     * @return int32 index
     */
    int GetRangedDpsIndex(lua_State* L, PlayerbotAI* botAI)
    {
        Player* member = ALE::CHECKOBJ<Player>(L, 2, false);
        if (!member)
            member = GetBotOrNull(botAI);

        if (!member)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetRangedDpsIndex(member));
        return 1;
    }

    /**
     * Returns the index of the given [Player] among the group members of a class.
     *
     * @param uint8 class
     * @param [Player] member = the bot itself
     * @return int32 index
     */
    int GetClassIndex(lua_State* L, PlayerbotAI* botAI)
    {
        uint8 cls = ALE::CHECKVAL<uint8>(L, 2);
        Player* member = ALE::CHECKOBJ<Player>(L, 3, false);
        if (!member)
            member = GetBotOrNull(botAI);

        if (!member)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetClassIndex(member, cls));
        return 1;
    }

    /**
     * Makes the bot whisper the given text to its master.
     *
     * @param string text
     * @param uint32 securityLevel = 3 : minimum security level required to receive it
     * @return bool sent
     */
    int TellMaster(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        PlayerbotSecurityLevel level = CheckSecurityLevel(L, 3);

        ALE::Push(L, botAI->TellMaster(text, level));
        return 1;
    }

    /**
     * Same as [PlayerbotAI]:TellMaster but the bot does not turn towards its master.
     *
     * @param string text
     * @param uint32 securityLevel = 3
     * @return bool sent
     */
    int TellMasterNoFacing(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        PlayerbotSecurityLevel level = CheckSecurityLevel(L, 3);

        ALE::Push(L, botAI->TellMasterNoFacing(text, level));
        return 1;
    }

    /**
     * Reports an error to the master of the bot.
     *
     * @param string text
     * @param uint32 securityLevel = 3
     * @return bool sent
     */
    int TellError(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        PlayerbotSecurityLevel level = CheckSecurityLevel(L, 3);

        ALE::Push(L, botAI->TellError(text, level));
        return 1;
    }

    /**
     * Makes the bot say the given text, going through the playerbot chat filters.
     *
     * @param string text
     * @return bool sent
     */
    int Say(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->Say(text));
        return 1;
    }

    /**
     * Makes the bot yell the given text.
     *
     * @param string text
     * @return bool sent
     */
    int Yell(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->Yell(text));
        return 1;
    }

    /**
     * Makes the bot whisper the given text to a player by name.
     *
     * @param string text
     * @param string receiverName
     * @return bool sent
     */
    int Whisper(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        std::string receiver = ALE::CHECKVAL<std::string>(L, 3);

        ALE::Push(L, botAI->Whisper(text, receiver));
        return 1;
    }

    /**
     * Makes the bot talk in its guild chat, checking its guild rank rights first.
     *
     * @param string text
     * @return bool sent
     */
    int SayToGuild(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->SayToGuild(text));
        return 1;
    }

    /**
     * Makes the bot talk in its party chat.
     *
     * @param string text
     * @return bool sent
     */
    int SayToParty(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->SayToParty(text));
        return 1;
    }

    /**
     * Makes the bot talk in its raid chat.
     *
     * @param string text
     * @return bool sent
     */
    int SayToRaid(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->SayToRaid(text));
        return 1;
    }

    /**
     * Makes the bot talk in the world channel.
     *
     * @param string text
     * @return bool sent
     */
    int SayToWorld(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->SayToWorld(text));
        return 1;
    }

    /**
     * Plays a sound for the bot.
     *
     * @param uint32 soundId
     * @return bool played
     */
    int PlaySound(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 soundId = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->PlaySound(soundId));
        return 1;
    }

    /**
     * Makes the bot perform an emote.
     *
     * @param uint32 emoteId
     * @return bool played
     */
    int PlayEmote(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 emoteId = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->PlayEmote(emoteId));
        return 1;
    }

    /**
     * Makes the bot ping the given world coordinates for its group.
     *
     * @param float x
     * @param float y
     */
    int Ping(lua_State* L, PlayerbotAI* botAI)
    {
        float x = ALE::CHECKVAL<float>(L, 2);
        float y = ALE::CHECKVAL<float>(L, 3);

        botAI->Ping(x, y);
        return 0;
    }

    /**
     * Returns `true` if the bot can currently cast the given spell on the target.
     *
     * Unlike [Unit]:CastSpell this weighs facing, range, cooldowns and the bot state.
     * The spell can be given either by ID or by name.
     *
     * @param uint32|string spell
     * @param [Unit] target = the bots current target
     * @return bool canCast
     */
    int CanCastSpell(lua_State* L, PlayerbotAI* botAI)
    {
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3, false);
        if (!target)
        {
            if (Player* bot = GetBotOrNull(botAI))
                target = bot->GetSelectedUnit();
        }

        if (lua_isnumber(L, 2))
        {
            uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
            ALE::Push(L, botAI->CanCastSpell(spellId, target));
        }
        else
        {
            std::string spellName = ALE::CHECKVAL<std::string>(L, 2);
            ALE::Push(L, botAI->CanCastSpell(spellName, target));
        }
        return 1;
    }

    /**
     * Makes the bot cast the given spell on the target.
     *
     * Unlike [Unit]:CastSpell this handles facing, the global cooldown and the
     * bot spell selection. The spell can be given either by ID or by name.
     *
     * @param uint32|string spell
     * @param [Unit] target = the bots current target
     * @return bool casted
     */
    int CastSpell(lua_State* L, PlayerbotAI* botAI)
    {
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3, false);
        if (!target)
        {
            if (Player* bot = GetBotOrNull(botAI))
                target = bot->GetSelectedUnit();
        }

        if (lua_isnumber(L, 2))
        {
            uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
            ALE::Push(L, botAI->CastSpell(spellId, target));
        }
        else
        {
            std::string spellName = ALE::CHECKVAL<std::string>(L, 2);
            ALE::Push(L, botAI->CastSpell(spellName, target));
        }
        return 1;
    }

    /**
     * Interrupts every spell the bot is currently casting and notifies its client.
     */
    int InterruptSpell(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->InterruptSpell();
        return 0;
    }

    /**
     * Asks the bot AI to interrupt its current cast on the next update.
     */
    int RequestSpellInterrupt(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->RequestSpellInterrupt();
        return 0;
    }

    /**
     * Tells the bot AI that the given spell was interrupted.
     *
     * @param uint32 spellId
     */
    int SpellInterrupted(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
        botAI->SpellInterrupted(spellId);
        return 0;
    }

    /**
     * Returns the global cooldown the given spell would trigger, in milliseconds.
     *
     * @param uint32 spellId
     * @return int32 globalCooldown
     */
    int GetGlobalCooldown(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->CalculateGlobalCooldown(spellId));
        return 1;
    }

    /**
     * Returns `true` if the given [Unit] has the named aura.
     *
     * Unlike [Unit]:HasAura this takes a spell name and can weigh the stack count
     * and the remaining duration.
     *
     * @param string spellName
     * @param [Unit] target = the bot itself
     * @param bool maxStack = false
     * @param bool checkIsOwner = false
     * @param int32 maxAmount = -1
     * @param bool checkDuration = false
     * @return bool hasAura
     */
    int HasAura(lua_State* L, PlayerbotAI* botAI)
    {
        std::string spellName = ALE::CHECKVAL<std::string>(L, 2);
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3, false);
        bool maxStack = ALE::CHECKVAL<bool>(L, 4, false);
        bool checkIsOwner = ALE::CHECKVAL<bool>(L, 5, false);
        int maxAmount = ALE::CHECKVAL<int>(L, 6, -1);
        bool checkDuration = ALE::CHECKVAL<bool>(L, 7, false);

        if (!target)
            target = GetBotOrNull(botAI);

        if (!target)
        {
            ALE::Push(L, false);
            return 1;
        }

        ALE::Push(L, botAI->HasAura(spellName, target, maxStack, checkIsOwner, maxAmount, checkDuration));
        return 1;
    }

    /**
     * Returns `true` if the given [Unit] has an aura the bot can dispel.
     *
     * @param [Unit] target
     * @param uint32 dispelType
     * @return bool hasAuraToDispel
     */
    int HasAuraToDispel(lua_State* L, PlayerbotAI* botAI)
    {
        Unit* target = ALE::CHECKOBJ<Unit>(L, 2);
        uint32 dispelType = ALE::CHECKVAL<uint32>(L, 3);

        ALE::Push(L, botAI->HasAuraToDispel(target, dispelType));
        return 1;
    }

    /**
     * Returns `true` if the given [Unit] is casting a spell the bot could interrupt.
     *
     * @param [Unit] target
     * @param string spellName
     * @return bool isInterruptable
     */
    int IsInterruptableSpellCasting(lua_State* L, PlayerbotAI* botAI)
    {
        Unit* target = ALE::CHECKOBJ<Unit>(L, 2);
        std::string spellName = ALE::CHECKVAL<std::string>(L, 3);

        ALE::Push(L, botAI->IsInterruptableSpellCasting(target, spellName));
        return 1;
    }

    /**
     * Removes the current shapeshift form of the bot.
     */
    int RemoveShapeshift(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->RemoveShapeshift();
        return 0;
    }

    /**
     * Returns `true` if the bot currently holds aggro on the given [Unit].
     *
     * @param [Unit] target
     * @return bool hasAggro
     */
    int HasAggro(lua_State* L, PlayerbotAI* botAI)
    {
        Unit* target = ALE::CHECKOBJ<Unit>(L, 2);
        ALE::Push(L, botAI->HasAggro(target));
        return 1;
    }

    /**
     * Returns `true` if the given [Unit] is movement impaired, snared or rooted.
     *
     * @param [Unit] target = the bot itself
     * @return bool isImpaired
     */
    int IsMovementImpaired(lua_State* L, PlayerbotAI* botAI)
    {
        Unit* target = ALE::CHECKOBJ<Unit>(L, 2, false);
        if (!target)
            target = GetBotOrNull(botAI);

        if (!target)
        {
            ALE::Push(L, false);
            return 1;
        }

        ALE::Push(L, botAI->IsMovementImpaired(target));
        return 1;
    }

    /**
     * Returns a configured playerbot range by name.
     *
     * Known types are for example "spell", "shoot", "heal", "follow", "attack" and "flee".
     *
     * @param string type
     * @return float range
     */
    int GetRange(lua_State* L, PlayerbotAI* botAI)
    {
        std::string type = ALE::CHECKVAL<std::string>(L, 2);
        ALE::Push(L, botAI->GetRange(type));
        return 1;
    }

    /**
     * Returns `true` if the bot has target exclusions configured.
     *
     * @return bool hasTargetExclusions
     */
    int HasTargetExclusions(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->HasTargetExclusions());
        return 1;
    }

    /**
     * Returns the equipped gear score of the given [Player] as computed by the bot AI.
     *
     * @param [Player] target = the bot itself
     * @return uint32 gearScore
     */
    int GetEquipGearScore(lua_State* L, PlayerbotAI* botAI)
    {
        Player* target = ALE::CHECKOBJ<Player>(L, 2, false);
        if (!target)
            target = GetBotOrNull(botAI);

        if (!target)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetEquipGearScore(target));
        return 1;
    }

    /**
     * Returns a table with every [Item] in the bags of the bot.
     *
     * @param bool withEquipped = false : also include the equipped items
     * @return table items
     */
    int GetInventoryItems(lua_State* L, PlayerbotAI* botAI)
    {
        bool withEquipped = ALE::CHECKVAL<bool>(L, 2, false);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        std::vector<Item*> items = withEquipped ? botAI->GetInventoryAndEquippedItems() : botAI->GetInventoryItems();
        for (Item* item : items)
        {
            ALE::Push(L, item);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns how many of the given item the bot carries in its inventory.
     *
     * @param uint32 entry
     * @return uint32 count
     */
    int GetInventoryItemsCount(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->GetInventoryItemsCountWithId(entry));
        return 1;
    }

    /**
     * Returns `true` if the bot carries the given item in its inventory.
     *
     * @param uint32 entry
     * @return bool hasItem
     */
    int HasItemInInventory(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->HasItemInInventory(entry));
        return 1;
    }

    /**
     * Returns the poison the bot would use, or `nil`.
     *
     * @return [Item] poison
     */
    int FindPoison(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->FindPoison());
        return 1;
    }

    /**
     * Returns the ammo the bot would use, or `nil`.
     *
     * @return [Item] ammo
     */
    int FindAmmo(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->FindAmmo());
        return 1;
    }

    /**
     * Returns the bandage the bot would use, or `nil`.
     *
     * @return [Item] bandage
     */
    int FindBandage(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->FindBandage());
        return 1;
    }

    /**
     * Returns an openable item carried by the bot, or `nil`.
     *
     * @return [Item] item
     */
    int FindOpenableItem(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->FindOpenableItem());
        return 1;
    }

    /**
     * Returns a locked item carried by the bot, or `nil`.
     *
     * @return [Item] item
     */
    int FindLockedItem(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->FindLockedItem());
        return 1;
    }

    /**
     * Returns the given consumable carried by the bot, or `nil`.
     *
     * @param uint32 entry
     * @return [Item] item
     */
    int FindConsumable(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->FindConsumable(entry));
        return 1;
    }

    /**
     * Returns the sharpening stone the bot would apply to the given weapon, or `nil`.
     *
     * @param [Item] weapon
     * @return [Item] stone
     */
    int FindStoneFor(lua_State* L, PlayerbotAI* botAI)
    {
        Item* weapon = ALE::CHECKOBJ<Item>(L, 2);
        ALE::Push(L, botAI->FindStoneFor(weapon));
        return 1;
    }

    /**
     * Returns the weapon oil the bot would apply to the given weapon, or `nil`.
     *
     * @param [Item] weapon
     * @return [Item] oil
     */
    int FindOilFor(lua_State* L, PlayerbotAI* botAI)
    {
        Item* weapon = ALE::CHECKOBJ<Item>(L, 2);
        ALE::Push(L, botAI->FindOilFor(weapon));
        return 1;
    }

    /**
     * Makes the bot use the given item, on itself or on a target.
     *
     * @param [Item] item
     * @param [Unit] target = nil
     */
    int ImbueItem(lua_State* L, PlayerbotAI* botAI)
    {
        Item* item = ALE::CHECKOBJ<Item>(L, 2);
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3, false);

        if (target)
            botAI->ImbueItem(item, target);
        else
            botAI->ImbueItem(item);
        return 0;
    }

    /**
     * Applies a permanent enchant to the item in the given equipment slot.
     *
     * @param uint32 spellId
     * @param uint8 slot
     */
    int EnchantItemT(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
        uint8 slot = ALE::CHECKVAL<uint8>(L, 3);

        botAI->EnchantItemT(spellId, slot);
        return 0;
    }

    /**
     * Returns a table with the IDs of every quest currently in the bots log.
     *
     * @return table questIds
     */
    int GetQuestIds(lua_State* L, PlayerbotAI* botAI)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (uint32 questId : botAI->GetAllCurrentQuestIds())
        {
            ALE::Push(L, questId);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns a table with the IDs of the quests the bot has not completed yet.
     *
     * @return table questIds
     */
    int GetIncompleteQuestIds(lua_State* L, PlayerbotAI* botAI)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (uint32 questId : botAI->GetCurrentIncompleteQuestIds())
        {
            ALE::Push(L, questId);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns `true` if the given activity type is currently allowed for the bot.
     *
     *     enum ActivityType
     *     {
     *         GRIND_ACTIVITY         = 1,
     *         RPG_ACTIVITY           = 2,
     *         TRAVEL_ACTIVITY        = 3,
     *         OUT_OF_PARTY_ACTIVITY  = 4,
     *         PACKET_ACTIVITY        = 5,
     *         DETAILED_MOVE_ACTIVITY = 6,
     *         PARTY_ACTIVITY         = 7,
     *         ALL_ACTIVITY           = 8
     *     };
     *
     * @param uint32 activityType = 8
     * @param bool checkNow = false : force a re-evaluation instead of using the cache
     * @return bool allowed
     */
    int IsActivityAllowed(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 activity = ALE::CHECKVAL<uint32>(L, 2, static_cast<uint32>(ALL_ACTIVITY));
        bool checkNow = ALE::CHECKVAL<bool>(L, 3, false);

        if (activity >= static_cast<uint32>(MAX_ACTIVITY_TYPE))
            activity = static_cast<uint32>(ALL_ACTIVITY);

        ALE::Push(L, botAI->AllowActivity(static_cast<ActivityType>(activity), checkNow));
        return 1;
    }

    /**
     * Returns the cached "is active" flag of the bot without re-evaluating it.
     *
     * @return bool active
     */
    int IsActive(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->IsActivityAllowedCached());
        return 1;
    }

    /**
     * Scales the given value by the current bot activity modifier.
     *
     * @param uint32 value
     * @return uint32 scaled
     */
    int AutoScaleActivity(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 value = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->AutoScaleActivity(value));
        return 1;
    }

    /**
     * Returns `true` if the bot has the given cheat enabled.
     *
     *     enum BotCheatMask
     *     {
     *         none   = 0,
     *         taxi   = 1,
     *         gold   = 2,
     *         health = 4,
     *         mana   = 8,
     *         power  = 16,
     *         raid   = 32,
     *         food   = 64
     *     };
     *
     * @param uint32 cheatMask
     * @return bool hasCheat
     */
    int HasCheat(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 mask = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, botAI->HasCheat(static_cast<BotCheatMask>(mask)));
        return 1;
    }

    /**
     * Returns the cheat mask of the bot.
     *
     * @return uint32 cheatMask
     */
    int GetCheat(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, static_cast<uint32>(botAI->GetCheat()));
        return 1;
    }

    /**
     * Sets the cheat mask of the bot.
     *
     * @param uint32 cheatMask : see [PlayerbotAI]:HasCheat
     */
    int SetCheat(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 mask = ALE::CHECKVAL<uint32>(L, 2);
        botAI->SetCheat(static_cast<BotCheatMask>(mask));
        return 0;
    }

    /**
     * Returns `true` if the bot is currently allowed to move.
     *
     * @return bool canMove
     */
    int CanMove(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->CanMove());
        return 1;
    }

    /**
     * Returns `true` if the bot is inside a non raid dungeon.
     *
     * @return bool inNonRaidDungeon
     */
    int IsInNonRaidDungeon(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->IsInNonRaidDungeon());
        return 1;
    }

    /**
     * Returns `true` if the bot is in a guild that contains real players.
     *
     * @return bool inRealGuild
     */
    int IsInRealGuild(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, botAI->IsInRealGuild());
        return 1;
    }

    /**
     * Returns `true` if the given [Player] belongs to the opposing faction.
     *
     * @param [Player] target
     * @return bool isOpposing
     */
    int IsOpposing(lua_State* L, PlayerbotAI* botAI)
    {
        Player* target = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, botAI->IsOpposing(target));
        return 1;
    }

    /**
     * Returns `true` if the given [WorldObject] is safe for the bot to interact with.
     *
     * @param [WorldObject] object
     * @return bool isSafe
     */
    int IsSafe(lua_State* L, PlayerbotAI* botAI)
    {
        WorldObject* object = ALE::CHECKOBJ<WorldObject>(L, 2);
        ALE::Push(L, botAI->IsSafe(object));
        return 1;
    }

    /**
     * Returns the localized name of the area the bot is standing in.
     *
     * @return string areaName
     */
    int GetCurrentAreaName(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, PlayerbotAI::GetLocalizedAreaName(botAI->GetCurrentArea()));
        return 1;
    }

    /**
     * Returns the localized name of the zone the bot is standing in.
     *
     * @return string zoneName
     */
    int GetCurrentZoneName(lua_State* L, PlayerbotAI* botAI)
    {
        ALE::Push(L, PlayerbotAI::GetLocalizedAreaName(botAI->GetCurrentZone()));
        return 1;
    }

    /**
     * Returns the [Unit] with the given GUID as seen by the bot, or `nil`.
     *
     * @param ObjectGuid guid
     * @return [Unit] unit
     */
    int GetUnit(lua_State* L, PlayerbotAI* botAI)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        ALE::Push(L, botAI->GetUnit(guid));
        return 1;
    }

    /**
     * Returns the [Creature] with the given GUID as seen by the bot, or `nil`.
     *
     * @param ObjectGuid guid
     * @return [Creature] creature
     */
    int GetCreature(lua_State* L, PlayerbotAI* botAI)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        ALE::Push(L, botAI->GetCreature(guid));
        return 1;
    }

    /**
     * Returns the [Player] with the given GUID as seen by the bot, or `nil`.
     *
     * @param ObjectGuid guid
     * @return [Player] player
     */
    int GetPlayer(lua_State* L, PlayerbotAI* botAI)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        ALE::Push(L, botAI->GetPlayer(guid));
        return 1;
    }

    /**
     * Returns the [GameObject] with the given GUID as seen by the bot, or `nil`.
     *
     * @param ObjectGuid guid
     * @return [GameObject] gameObject
     */
    int GetGameObject(lua_State* L, PlayerbotAI* botAI)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        ALE::Push(L, botAI->GetGameObject(guid));
        return 1;
    }

    /**
     * Returns the jump destination currently set for the bot.
     *
     * @return float x
     * @return float y
     * @return float z
     * @return float o
     */
    int GetJumpDestination(lua_State* L, PlayerbotAI* botAI)
    {
        Position pos = botAI->GetJumpDestination();
        ALE::Push(L, pos.GetPositionX());
        ALE::Push(L, pos.GetPositionY());
        ALE::Push(L, pos.GetPositionZ());
        ALE::Push(L, pos.GetOrientation());
        return 4;
    }

    /**
     * Sets the jump destination of the bot.
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float o = 0
     */
    int SetJumpDestination(lua_State* L, PlayerbotAI* botAI)
    {
        float x = ALE::CHECKVAL<float>(L, 2);
        float y = ALE::CHECKVAL<float>(L, 3);
        float z = ALE::CHECKVAL<float>(L, 4);
        float o = ALE::CHECKVAL<float>(L, 5, 0.0f);

        botAI->SetJumpDestination(Position(x, y, z, o));
        return 0;
    }

    /**
     * Clears the jump destination of the bot.
     */
    int ResetJumpDestination(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->ResetJumpDestination();
        return 0;
    }

    /**
     * Makes the pet of the bot follow it again.
     */
    int PetFollow(lua_State* /*L*/, PlayerbotAI* botAI)
    {
        botAI->PetFollow();
        return 0;
    }

    /**
     * Returns the named [Aura] carried by the given [Unit], or `nil`.
     *
     * @param string spellName
     * @param [Unit] target = the bot itself
     * @param bool checkIsOwner = false
     * @param bool checkDuration = false
     * @param int32 checkStack = -1
     * @return [Aura] aura
     */
    int GetAura(lua_State* L, PlayerbotAI* botAI)
    {
        std::string spellName = ALE::CHECKVAL<std::string>(L, 2);
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3, false);
        bool checkIsOwner = ALE::CHECKVAL<bool>(L, 4, false);
        bool checkDuration = ALE::CHECKVAL<bool>(L, 5, false);
        int checkStack = ALE::CHECKVAL<int>(L, 6, -1);

        if (!target)
            target = GetBotOrNull(botAI);

        if (!target)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, botAI->GetAura(spellName, target, checkIsOwner, checkDuration, checkStack));
        return 1;
    }

    /**
     * Returns the [WorldObject] with the given GUID as seen by the bot, or `nil`.
     *
     * @param ObjectGuid guid
     * @return [WorldObject] object
     */
    int GetWorldObject(lua_State* L, PlayerbotAI* botAI)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        ALE::Push(L, botAI->GetWorldObject(guid));
        return 1;
    }

    /**
     * Returns a table with the [Quest]s currently in the bots log.
     *
     * @param bool incompleteOnly = false : only keep the quests that are not complete yet
     * @return table quests
     */
    int GetQuests(lua_State* L, PlayerbotAI* botAI)
    {
        bool incompleteOnly = ALE::CHECKVAL<bool>(L, 2, false);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        std::vector<Quest const*> quests =
            incompleteOnly ? botAI->GetCurrentIncompleteQuests() : botAI->GetAllCurrentQuests();
        for (Quest const* quest : quests)
        {
            ALE::Push(L, quest);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns the quests of the bot that still need the given item.
     *
     * Each entry of the returned table is a table holding the [Quest] and the
     * amount of that item the quest still requires.
     *
     * @param uint32 entry
     * @return table quests
     */
    int GetQuestsRequiringItem(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (auto const& pair : botAI->GetCurrentQuestsRequiringItemId(entry))
        {
            lua_newtable(L);
            int row = lua_gettop(L);

            ALE::Push(L, pair.first);
            lua_rawseti(L, row, 1);
            ALE::Push(L, pair.second);
            lua_rawseti(L, row, 2);

            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns the localized name of the given creature entry.
     *
     * @param uint32 entry
     * @return string name
     */
    int GetLocalizedCreatureName(lua_State* L, PlayerbotAI* /*botAI*/)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, PlayerbotAI::GetLocalizedCreatureName(entry));
        return 1;
    }

    /**
     * Returns the localized name of the given gameobject entry.
     *
     * @param uint32 entry
     * @return string name
     */
    int GetLocalizedGameObjectName(lua_State* L, PlayerbotAI* /*botAI*/)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, PlayerbotAI::GetLocalizedGameObjectName(entry));
        return 1;
    }

    /**
     * Makes the bot talk in the given global channel.
     *
     *     enum ChatChannelId
     *     {
     *         GENERAL           = 1,
     *         TRADE             = 2,
     *         LOCAL_DEFENSE     = 22,
     *         GUILD_RECRUITMENT = 25,
     *         LOOKING_FOR_GROUP = 26,
     *         WORLD_DEFENSE     = 23
     *     };
     *
     * @param string text
     * @param uint32 channelId
     * @return bool sent
     */
    int SayToChannel(lua_State* L, PlayerbotAI* botAI)
    {
        std::string text = ALE::CHECKVAL<std::string>(L, 2);
        uint32 channelId = ALE::CHECKVAL<uint32>(L, 3);

        ALE::Push(L, botAI->SayToChannel(text, static_cast<ChatChannelId>(channelId)));
        return 1;
    }

    /**
     * Returns which chat source a message of the given type and channel maps to.
     *
     * @param uint32 chatType
     * @param string channelName = ""
     * @return uint32 chatChannelSource
     */
    int GetChatChannelSource(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 type = ALE::CHECKVAL<uint32>(L, 2);
        std::string channelName = ALE::CHECKVAL<std::string>(L, 3, "");

        Player* bot = GetBotOrNull(botAI);
        if (!bot)
        {
            ALE::Push(L);
            return 1;
        }

        ALE::Push(L, static_cast<uint32>(botAI->GetChatChannelSource(bot, type, channelName)));
        return 1;
    }

    /**
     * Returns `true` if the bot is on a vehicle matching every requested capability.
     *
     * @param bool canControl = false
     * @param bool canCast = false
     * @param bool canAttack = false
     * @param bool canTurn = false
     * @param bool fixed = false
     * @return bool isInVehicle
     */
    int IsInVehicle(lua_State* L, PlayerbotAI* botAI)
    {
        bool canControl = ALE::CHECKVAL<bool>(L, 2, false);
        bool canCast = ALE::CHECKVAL<bool>(L, 3, false);
        bool canAttack = ALE::CHECKVAL<bool>(L, 4, false);
        bool canTurn = ALE::CHECKVAL<bool>(L, 5, false);
        bool fixed = ALE::CHECKVAL<bool>(L, 6, false);

        ALE::Push(L, botAI->IsInVehicle(canControl, canCast, canAttack, canTurn, fixed));
        return 1;
    }

    /**
     * Returns `true` if the bot can cast the given vehicle spell on the target.
     *
     * @param uint32 spellId
     * @param [Unit] target
     * @return bool canCast
     */
    int CanCastVehicleSpell(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3);

        ALE::Push(L, botAI->CanCastVehicleSpell(spellId, target));
        return 1;
    }

    /**
     * Makes the bot cast the given vehicle spell on the target.
     *
     * @param uint32 spellId
     * @param [Unit] target
     * @return bool casted
     */
    int CastVehicleSpell(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
        Unit* target = ALE::CHECKOBJ<Unit>(L, 3);

        ALE::Push(L, botAI->CastVehicleSpell(spellId, target));
        return 1;
    }

    /**
     * Returns `true` if the given [Player] is a bot acting as main tank of its group.
     *
     * @param [Player] target = the bot itself
     * @return bool isBotMainTank
     */
    int IsBotMainTank(lua_State* L, PlayerbotAI* botAI)
    {
        Player* target = ALE::CHECKOBJ<Player>(L, 2, false);
        if (!target)
            target = GetBotOrNull(botAI);

        ALE::Push(L, target ? PlayerbotAI::IsBotMainTank(target) : false);
        return 1;
    }

    /**
     * Returns `true` if the given [Player] is the assist tank of the given index.
     *
     * @param uint8 index
     * @param [Player] target = the bot itself
     * @param bool ignoreDeadPlayers = false
     * @return bool isAssistTankOfIndex
     */
    int IsAssistTankOfIndex(lua_State* L, PlayerbotAI* botAI)
    {
        uint8 index = ALE::CHECKVAL<uint8>(L, 2);
        Player* target = ALE::CHECKOBJ<Player>(L, 3, false);
        bool ignoreDeadPlayers = ALE::CHECKVAL<bool>(L, 4, false);

        if (!target)
            target = GetBotOrNull(botAI);

        ALE::Push(L, target ? PlayerbotAI::IsAssistTankOfIndex(target, index, ignoreDeadPlayers) : false);
        return 1;
    }

    /**
     * Returns `true` if the given [Player] is the assist healer of the given index.
     *
     * @param uint8 index
     * @param [Player] target = the bot itself
     * @param bool ignoreDeadPlayers = false
     * @return bool isAssistHealOfIndex
     */
    int IsAssistHealOfIndex(lua_State* L, PlayerbotAI* botAI)
    {
        uint8 index = ALE::CHECKVAL<uint8>(L, 2);
        Player* target = ALE::CHECKOBJ<Player>(L, 3, false);
        bool ignoreDeadPlayers = ALE::CHECKVAL<bool>(L, 4, false);

        if (!target)
            target = GetBotOrNull(botAI);

        ALE::Push(L, target ? PlayerbotAI::IsAssistHealOfIndex(target, index, ignoreDeadPlayers) : false);
        return 1;
    }

    /**
     * Returns `true` if the given [Player] is the assist ranged dps of the given index.
     *
     * @param uint8 index
     * @param [Player] target = the bot itself
     * @param bool ignoreDeadPlayers = false
     * @return bool isAssistRangedDpsOfIndex
     */
    int IsAssistRangedDpsOfIndex(lua_State* L, PlayerbotAI* botAI)
    {
        uint8 index = ALE::CHECKVAL<uint8>(L, 2);
        Player* target = ALE::CHECKOBJ<Player>(L, 3, false);
        bool ignoreDeadPlayers = ALE::CHECKVAL<bool>(L, 4, false);

        if (!target)
            target = GetBotOrNull(botAI);

        ALE::Push(L, target ? PlayerbotAI::IsAssistRangedDpsOfIndex(target, index, ignoreDeadPlayers) : false);
        return 1;
    }

    /**
     * Returns `true` if the bot can dispel the given spell.
     *
     * @param [SpellInfo] spellInfo
     * @param uint32 dispelType
     * @return bool canDispel
     */
    int CanDispel(lua_State* L, PlayerbotAI* botAI)
    {
        SpellInfo const* spellInfo = ALE::CHECKOBJ<SpellInfo>(L, 2);
        uint32 dispelType = ALE::CHECKVAL<uint32>(L, 3);

        ALE::Push(L, botAI->canDispel(spellInfo, dispelType));
        return 1;
    }

    /**
     * Checks whether the bot could equip the given [Item] in the given slot.
     *
     * @param uint8 slot
     * @param [Item] item
     * @param bool swap = false
     * @param bool notLoading = true
     * @return uint32 inventoryResult : 0 when the item can be equipped
     * @return uint16 dest : the destination slot the item would go to
     */
    int CanEquipItem(lua_State* L, PlayerbotAI* botAI)
    {
        uint8 slot = ALE::CHECKVAL<uint8>(L, 2);
        Item* item = ALE::CHECKOBJ<Item>(L, 3);
        bool swap = ALE::CHECKVAL<bool>(L, 4, false);
        bool notLoading = ALE::CHECKVAL<bool>(L, 5, true);

        uint16 dest = 0;
        InventoryResult result = botAI->CanEquipItem(slot, dest, item, swap, notLoading);

        ALE::Push(L, static_cast<uint32>(result));
        ALE::Push(L, dest);
        return 2;
    }

    /**
     * Returns the equipment slot the given [ItemTemplate] would go to.
     *
     * @param [ItemTemplate] itemTemplate
     * @param uint32 slot
     * @param bool swap = false
     * @return uint8 slot
     */
    int FindEquipSlot(lua_State* L, PlayerbotAI* botAI)
    {
        ItemTemplate const* proto = ALE::CHECKOBJ<ItemTemplate>(L, 2);
        uint32 slot = ALE::CHECKVAL<uint32>(L, 3);
        bool swap = ALE::CHECKVAL<bool>(L, 4, false);

        ALE::Push(L, botAI->FindEquipSlot(proto, slot, swap));
        return 1;
    }

    /**
     * Returns the raw activity flag of the bot for the given type, without re-evaluating it.
     *
     * @param uint32 activityType : see [PlayerbotAI]:IsActivityAllowed
     * @return bool allowed
     */
    int AllowActive(lua_State* L, PlayerbotAI* botAI)
    {
        uint32 activity = ALE::CHECKVAL<uint32>(L, 2, static_cast<uint32>(ALL_ACTIVITY));
        if (activity >= static_cast<uint32>(MAX_ACTIVITY_TYPE))
            activity = static_cast<uint32>(ALL_ACTIVITY);

        ALE::Push(L, botAI->AllowActive(static_cast<ActivityType>(activity)));
        return 1;
    }

    /**
     * Returns `true` if the given location is far enough for a starting level bot.
     *
     * @param [Player] target
     * @param uint32 mapId
     * @param float x
     * @param float y
     * @param float z
     * @param bool fromStartUp = false
     * @return bool passes
     */
    int StarterLevelDistanceCheck(lua_State* L, PlayerbotAI* botAI)
    {
        Player* target = ALE::CHECKOBJ<Player>(L, 2);
        uint32 mapId = ALE::CHECKVAL<uint32>(L, 3);
        float x = ALE::CHECKVAL<float>(L, 4);
        float y = ALE::CHECKVAL<float>(L, 5);
        float z = ALE::CHECKVAL<float>(L, 6);
        bool fromStartUp = ALE::CHECKVAL<bool>(L, 7, false);

        ALE::Push(L, botAI->StarterLevelDistanceCheck(target, WorldLocation(mapId, x, y, z), fromStartUp));
        return 1;
    }

    /**
     * Returns `true` if both names are equal, ignoring the case.
     *
     * @param string first
     * @param string second
     * @return bool equal
     */
    int EqualLowercaseName(lua_State* L, PlayerbotAI* botAI)
    {
        std::string first = ALE::CHECKVAL<std::string>(L, 2);
        std::string second = ALE::CHECKVAL<std::string>(L, 3);

        ALE::Push(L, botAI->EqualLowercaseName(first, second));
        return 1;
    }

    /**
     * Returns `true` if the given spell family flags describe a healing spell.
     *
     * @param uint32 spellFamilyName
     * @param uint32 flags0 = 0
     * @param uint32 flags1 = 0
     * @param uint32 flags2 = 0
     * @return bool isHealingSpell
     */
    int IsHealingSpell(lua_State* L, PlayerbotAI* /*botAI*/)
    {
        uint32 family = ALE::CHECKVAL<uint32>(L, 2);
        uint32 flags0 = ALE::CHECKVAL<uint32>(L, 3, 0);
        uint32 flags1 = ALE::CHECKVAL<uint32>(L, 4, 0);
        uint32 flags2 = ALE::CHECKVAL<uint32>(L, 5, 0);

        ALE::Push(L, PlayerbotAI::IsHealingSpell(family, flag96(flags0, flags1, flags2)));
        return 1;
    }

    /**
     * Returns the spell family matching the given class.
     *
     * @param uint8 class
     * @return uint32 spellFamilyName
     */
    int Class2SpellFamilyName(lua_State* L, PlayerbotAI* /*botAI*/)
    {
        uint8 cls = ALE::CHECKVAL<uint8>(L, 2);
        ALE::Push(L, static_cast<uint32>(PlayerbotAI::Class2SpellFamilyName(cls)));
        return 1;
    }

    /**
     * Returns the score multiplier the bot AI applies to items of the given quality.
     *
     * @param uint32 quality
     * @return float multiplier
     */
    int GetItemScoreMultiplier(lua_State* L, PlayerbotAI* /*botAI*/)
    {
        uint32 quality = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, PlayerbotAI::GetItemScoreMultiplier(static_cast<ItemQualities>(quality)));
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // PLAYERBOTAIMETHODS_H
