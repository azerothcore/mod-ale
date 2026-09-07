/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef PLAYERBOTMGRMETHODS_H
#define PLAYERBOTMGRMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "Chat.h"
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"

/***
 * Inherits all methods from: [PlayerbotHolder]
 */
namespace LuaPlayerBotMgr
{
    /**
     * Returns the [Player] this manager belongs to.
     *
     * @return [Player] master
     */
    int GetMaster(lua_State* L, PlayerbotMgr* mgr)
    {
        ALE::Push(L, mgr->GetMaster());
        return 1;
    }

    /**
     * Forwards a chat command to every bot owned by the master.
     *
     * @param uint32 chatType
     * @param string text
     */
    int HandleCommand(lua_State* L, PlayerbotMgr* mgr)
    {
        uint32 type = ALE::CHECKVAL<uint32>(L, 2);
        std::string text = ALE::CHECKVAL<std::string>(L, 3);

        mgr->HandleCommand(type, text);
        return 0;
    }

    /**
     * Cancels a pending logout of the master.
     */
    int CancelLogout(lua_State* /*L*/, PlayerbotMgr* mgr)
    {
        mgr->CancelLogout();
        return 0;
    }

    /**
     * Reports an error coming from one of the bots to the master.
     *
     * @param string botName
     * @param string text
     */
    int TellError(lua_State* L, PlayerbotMgr* mgr)
    {
        std::string botName = ALE::CHECKVAL<std::string>(L, 2);
        std::string text = ALE::CHECKVAL<std::string>(L, 3);

        mgr->TellError(botName, text);
        return 0;
    }

    /**
     * Saves the playerbot data of the master to the database.
     */
    int SaveToDB(lua_State* /*L*/, PlayerbotMgr* mgr)
    {
        mgr->SaveToDB();
        return 0;
    }

    /**
     * Sets the security key used to link other accounts to this one.
     *
     * @param [Player] player
     * @param string key
     */
    int HandleSetSecurityKeyCommand(lua_State* L, PlayerbotMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        std::string key = ALE::CHECKVAL<std::string>(L, 3);

        mgr->HandleSetSecurityKeyCommand(player, key);
        return 0;
    }

    /**
     * Links the given account to this one, so its characters can be used as bots.
     *
     * @param [Player] player
     * @param string accountName
     * @param string key : the security key of that account
     */
    int HandleLinkAccountCommand(lua_State* L, PlayerbotMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        std::string accountName = ALE::CHECKVAL<std::string>(L, 3);
        std::string key = ALE::CHECKVAL<std::string>(L, 4);

        mgr->HandleLinkAccountCommand(player, accountName, key);
        return 0;
    }

    /**
     * Unlinks the given account from this one.
     *
     * @param [Player] player
     * @param string accountName
     */
    int HandleUnlinkAccountCommand(lua_State* L, PlayerbotMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        std::string accountName = ALE::CHECKVAL<std::string>(L, 3);

        mgr->HandleUnlinkAccountCommand(player, accountName);
        return 0;
    }

    /**
     * Sends the list of the accounts linked to this one to the given [Player].
     *
     * @param [Player] player
     */
    int HandleViewLinkedAccountsCommand(lua_State* L, PlayerbotMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        mgr->HandleViewLinkedAccountsCommand(player);
        return 0;
    }

    /**
     * Runs a `.playerbotmgr` command through the given [ChatHandler].
     *
     * @param [ChatHandler] handler
     * @param string args
     * @return bool handled
     */
    int HandlePlayerbotMgrCommand(lua_State* L, PlayerbotMgr* /*mgr*/)
    {
        ChatHandler* handler = ALE::CHECKOBJ<ChatHandler>(L, 2);
        std::string args = ALE::CHECKVAL<std::string>(L, 3, "");

        ALE::Push(L, PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args.c_str()));
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // PLAYERBOTMGRMETHODS_H
