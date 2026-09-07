/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef PLAYERBOTHOLDERMETHODS_H
#define PLAYERBOTHOLDERMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"

namespace LuaPlayerBotHolder
{
    /**
     * Returns how many bots this holder currently owns.
     *
     * @return uint32 count
     */
    int GetPlayerbotsCount(lua_State* L, PlayerbotHolder* holder)
    {
        ALE::Push(L, holder->GetPlayerbotsCount());
        return 1;
    }

    /**
     * Returns how many bots of the given class this holder currently owns.
     *
     * @param uint32 class
     * @return uint32 count
     */
    int GetPlayerbotsCountByClass(lua_State* L, PlayerbotHolder* holder)
    {
        uint32 cls = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, holder->GetPlayerbotsCountByClass(cls));
        return 1;
    }

    /**
     * Returns a table with every bot [Player] owned by this holder.
     *
     * @return table bots
     */
    int GetPlayerBots(lua_State* L, PlayerbotHolder* holder)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (PlayerBotMap::const_iterator itr = holder->GetPlayerBotsBegin();
             itr != holder->GetPlayerBotsEnd(); ++itr)
        {
            ALE::Push(L, itr->second);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }

    /**
     * Returns the bot [Player] with the given GUID owned by this holder, or `nil`.
     *
     * @param ObjectGuid|uint32 bot : an ObjectGuid or a character low GUID
     * @return [Player] bot
     */
    int GetPlayerBot(lua_State* L, PlayerbotHolder* holder)
    {
        if (lua_isnumber(L, 2))
        {
            uint32 lowGuid = ALE::CHECKVAL<uint32>(L, 2);
            ALE::Push(L, holder->GetPlayerBot(ObjectGuid::LowType(lowGuid)));
        }
        else
        {
            ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
            ALE::Push(L, holder->GetPlayerBot(guid));
        }
        return 1;
    }

    /**
     * Logs in the character with the given GUID as a bot of this holder.
     *
     * @param ObjectGuid guid
     * @param uint32 masterAccountId
     */
    int AddPlayerBot(lua_State* L, PlayerbotHolder* holder)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        uint32 masterAccountId = ALE::CHECKVAL<uint32>(L, 3);

        holder->AddPlayerBot(guid, masterAccountId);
        return 0;
    }

    /**
     * Logs out the bot with the given GUID.
     *
     * @param ObjectGuid guid
     */
    int LogoutPlayerBot(lua_State* L, PlayerbotHolder* holder)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        holder->LogoutPlayerBot(guid);
        return 0;
    }

    /**
     * Logs out every bot owned by this holder.
     */
    int LogoutAllBots(lua_State* /*L*/, PlayerbotHolder* holder)
    {
        holder->LogoutAllBots();
        return 0;
    }

    /**
     * Disables the bot with the given GUID without logging it out.
     *
     * @param ObjectGuid guid
     */
    int DisablePlayerBot(lua_State* L, PlayerbotHolder* holder)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        holder->DisablePlayerBot(guid);
        return 0;
    }

    /**
     * Drops the bot with the given GUID from the holder map.
     *
     * @param ObjectGuid guid
     */
    int RemoveFromPlayerbotsMap(lua_State* L, PlayerbotHolder* holder)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        holder->RemoveFromPlayerbotsMap(guid);
        return 0;
    }

    /**
     * Returns `true` if the given account is linked to the given master account.
     *
     * @param uint32 accountId
     * @param uint32 masterAccountId
     * @return bool isLinked
     */
    int IsAccountLinked(lua_State* L, PlayerbotHolder* holder)
    {
        uint32 accountId = ALE::CHECKVAL<uint32>(L, 2);
        uint32 masterAccountId = ALE::CHECKVAL<uint32>(L, 3);

        ALE::Push(L, holder->IsAccountLinked(accountId, masterAccountId));
        return 1;
    }

    /**
     * Returns the bots of the given master as the `.playerbot list` command would.
     *
     * @param [Player] master
     * @return string list
     */
    int ListBots(lua_State* L, PlayerbotHolder* holder)
    {
        Player* master = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, holder->ListBots(master));
        return 1;
    }

    /**
     * Returns the characters the given master may use as bots.
     *
     * @param [Player] master
     * @return string list
     */
    int LookupBots(lua_State* L, PlayerbotHolder* holder)
    {
        Player* master = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, holder->LookupBots(master));
        return 1;
    }
    /**
     * Runs a `.playerbot` command and returns the answer lines.
     *
     * @param string command : for example "add Botname" or "co +tank"
     * @param [Player] master = nil
     * @return table lines : a table of strings
     */
    int HandlePlayerbotCommand(lua_State* L, PlayerbotHolder* holder)
    {
        std::string command = ALE::CHECKVAL<std::string>(L, 2);
        Player* master = ALE::CHECKOBJ<Player>(L, 3, false);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (std::string const& line : holder->HandlePlayerbotCommand(command.c_str(), master))
        {
            ALE::Push(L, line);
            lua_rawseti(L, tbl, ++i);
        }

        lua_settop(L, tbl);
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // PLAYERBOTHOLDERMETHODS_H
