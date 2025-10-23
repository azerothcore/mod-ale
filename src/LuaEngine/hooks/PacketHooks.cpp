/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEIncludes.h"
#include "ALETemplate.h"

using namespace Hooks;

#define START_HOOK_SERVER(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<ServerEvents>(EVENT);\
    if (!ServerEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_PACKET(EVENT, OPCODE) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<PacketEvents>(EVENT, OPCODE);\
    if (!PacketEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

bool ALE::OnPacketSend(WorldSession* session, const WorldPacket& packet)
{
    bool result = true;
    Player* player = NULL;
    if (session)
        player = session->GetPlayer();
    OnPacketSendAny(player, packet, result);
    OnPacketSendOne(player, packet, result);
    return result;
}
void ALE::OnPacketSendAny(Player* player, const WorldPacket& packet, bool& result)
{
    START_HOOK_SERVER(SERVER_EVENT_ON_PACKET_SEND);
    Push(new WorldPacket(packet));
    Push(player);
    int n = SetupStack(ServerEventBindings, key, 2);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 2, 1);

        if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
            result = false;

        lua_pop(L, 1);
    }

    CleanUpStack(2);
}

void ALE::OnPacketSendOne(Player* player, const WorldPacket& packet, bool& result)
{
    START_HOOK_PACKET(PACKET_EVENT_ON_PACKET_SEND, packet.GetOpcode());
    Push(new WorldPacket(packet));
    Push(player);
    int n = SetupStack(PacketEventBindings, key, 2);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 2, 1);

        if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
            result = false;

        lua_pop(L, 1);
    }

    CleanUpStack(2);
}

bool ALE::OnPacketReceive(WorldSession* session, WorldPacket& packet)
{
    bool result = true;
    Player* player = NULL;
    if (session)
        player = session->GetPlayer();
    OnPacketReceiveAny(player, packet, result);
    OnPacketReceiveOne(player, packet, result);
    return result;
}

void ALE::OnPacketReceiveAny(Player* player, WorldPacket& packet, bool& result)
{
    START_HOOK_SERVER(SERVER_EVENT_ON_PACKET_RECEIVE);
    Push(new WorldPacket(packet));
    Push(player);
    int n = SetupStack(ServerEventBindings, key, 2);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 2, 2);

        if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
            result = false;

        if (lua_isuserdata(L, r + 1))
            if (WorldPacket* data = CHECKOBJ<WorldPacket>(L, r + 1, false))
                packet = *data;

        lua_pop(L, 2);
    }

    CleanUpStack(2);
}

void ALE::OnPacketReceiveOne(Player* player, WorldPacket& packet, bool& result)
{
    START_HOOK_PACKET(PACKET_EVENT_ON_PACKET_RECEIVE, packet.GetOpcode());
    Push(new WorldPacket(packet));
    Push(player);
    int n = SetupStack(PacketEventBindings, key, 2);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 2, 2);

        if (lua_isboolean(L, r + 0) && !lua_toboolean(L, r + 0))
            result = false;

        if (lua_isuserdata(L, r + 1))
            if (WorldPacket* data = CHECKOBJ<WorldPacket>(L, r + 1, false))
                packet = *data;

        lua_pop(L, 2);
    }

    CleanUpStack(2);
}
