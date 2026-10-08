/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef PLAYERBOTSMGRMETHODS_H
#define PLAYERBOTSMGRMETHODS_H

#ifdef MOD_PLAYERBOTS
#include "PlayerbotAI.h"
#include "PlayerbotMgr.h"

namespace LuaPlayerBotsMgr
{
    /**
     * Returns the [PlayerbotAI] attached to the given [Player], or `nil`.
     *
     * @param [Player] player
     * @return [PlayerbotAI] botAI
     */
    int GetPlayerbotAI(lua_State* L, PlayerbotsMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, mgr->GetPlayerbotAI(player));
        return 1;
    }

    /**
     * Returns the [PlayerbotMgr] of the given [Player], or `nil` when it owns no bots.
     *
     * @param [Player] player
     * @return [PlayerbotMgr] playerbotMgr
     */
    int GetPlayerbotMgr(lua_State* L, PlayerbotsMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        ALE::Push(L, mgr->GetPlayerbotMgr(player));
        return 1;
    }

    /**
     * Attaches playerbot data to the given [Player].
     *
     * This is bot lifetime management normally driven by the module itself. Calling
     * it on a [Player] that already carries the matching data leaks the previous one.
     *
     * @param [Player] player
     * @param bool isBotAI : true to attach a bot AI, false to attach a bot manager
     */
    int AddPlayerbotData(lua_State* L, PlayerbotsMgr* mgr)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 2);
        bool isBotAI = ALE::CHECKVAL<bool>(L, 3);

        mgr->AddPlayerbotData(player, isBotAI);
        return 0;
    }

    /**
     * Detaches the playerbot data of the given GUID.
     *
     * @param ObjectGuid guid
     * @param bool isAI : true to drop the bot AI, false to drop the bot manager
     */
    int RemovePlayerbotData(lua_State* L, PlayerbotsMgr* mgr)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        bool isAI = ALE::CHECKVAL<bool>(L, 3);

        mgr->RemovePlayerBotData(guid, isAI);
        return 0;
    }
};

namespace LuaPlayerBotsMgrGlobal
{
    /**
     * Returns the [PlayerbotsMgr] singleton, which owns the bot AI and bot manager maps.
     *
     * @return [PlayerbotsMgr] playerbotsMgr
     */
    int GetPlayerbotsMgr(lua_State* L)
    {
        ALE::Push(L, &sPlayerbotsMgr);
        return 1;
    }
};
#endif // MOD_PLAYERBOTS
#endif // PLAYERBOTSMGRMETHODS_H
