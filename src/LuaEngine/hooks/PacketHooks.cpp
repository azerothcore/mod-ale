/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"

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

namespace
{
    // Handlers receive a copy of the packet; a handler returning false blocks it.
    template<typename ALEType, typename BindingsType, typename KeyType>
    void DispatchPacketEvent(ALEType* engine, BindingsType& bindings, KeyType const& key,
        WorldPacket const& packet, Player* player, bool& result)
    {
        for (sol::protected_function const& callback : bindings.GetCallbacksFor(key))
        {
            sol::protected_function_result callResult = engine->Call(callback, key.event_id, WorldPacket(packet), player);
            if (!callResult.valid())
                continue;

            if (sol::optional<bool> value = callResult.template get<sol::optional<bool>>(0))
                if (!*value)
                    result = false;
        }
    }
}

bool ALE::OnPacketSend(WorldSession* session, const WorldPacket& packet)
{
    bool result = true;
    Player* player = nullptr;
    if (session)
        player = session->GetPlayer();
    OnPacketSendAny(player, packet, result);
    OnPacketSendOne(player, packet, result);
    return result;
}

void ALE::OnPacketSendAny(Player* player, const WorldPacket& packet, bool& result)
{
    START_HOOK_SERVER(SERVER_EVENT_ON_PACKET_SEND);
    DispatchPacketEvent(this, *ServerEventBindings, key, packet, player, result);
}

void ALE::OnPacketSendOne(Player* player, const WorldPacket& packet, bool& result)
{
    START_HOOK_PACKET(PACKET_EVENT_ON_PACKET_SEND, packet.GetOpcode());
    DispatchPacketEvent(this, *PacketEventBindings, key, packet, player, result);
}

bool ALE::OnPacketReceive(WorldSession* session, WorldPacket const& packet)
{
    bool result = true;
    Player* player = nullptr;
    if (session)
        player = session->GetPlayer();
    OnPacketReceiveAny(player, packet, result);
    OnPacketReceiveOne(player, packet, result);
    return result;
}

void ALE::OnPacketReceiveAny(Player* player, WorldPacket const& packet, bool& result)
{
    START_HOOK_SERVER(SERVER_EVENT_ON_PACKET_RECEIVE);
    DispatchPacketEvent(this, *ServerEventBindings, key, packet, player, result);
}

void ALE::OnPacketReceiveOne(Player* player, WorldPacket const& packet, bool& result)
{
    START_HOOK_PACKET(PACKET_EVENT_ON_PACKET_RECEIVE, packet.GetOpcode());
    DispatchPacketEvent(this, *PacketEventBindings, key, packet, player, result);
}
