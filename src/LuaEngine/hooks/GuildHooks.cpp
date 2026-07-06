/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"

using namespace Hooks;

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<GuildEvents>(EVENT);\
    if (!GuildEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

void ALE::OnAddMember(Guild* guild, Player* player, uint32 plRank)
{
    START_HOOK(GUILD_EVENT_ON_ADD_MEMBER);
    CallAll(*GuildEventBindings, key, guild, player, plRank);
}

void ALE::OnRemoveMember(Guild* guild, Player* player, bool isDisbanding)
{
    START_HOOK(GUILD_EVENT_ON_REMOVE_MEMBER);
    CallAll(*GuildEventBindings, key, guild, player, isDisbanding);
}

void ALE::OnMOTDChanged(Guild* guild, const std::string& newMotd)
{
    START_HOOK(GUILD_EVENT_ON_MOTD_CHANGE);
    CallAll(*GuildEventBindings, key, guild, newMotd);
}

void ALE::OnInfoChanged(Guild* guild, const std::string& newInfo)
{
    START_HOOK(GUILD_EVENT_ON_INFO_CHANGE);
    CallAll(*GuildEventBindings, key, guild, newInfo);
}

void ALE::OnCreate(Guild* guild, Player* leader, const std::string& name)
{
    START_HOOK(GUILD_EVENT_ON_CREATE);
    CallAll(*GuildEventBindings, key, guild, leader, name);
}

void ALE::OnDisband(Guild* guild)
{
    START_HOOK(GUILD_EVENT_ON_DISBAND);
    CallAll(*GuildEventBindings, key, guild);
}

void ALE::OnMemberWitdrawMoney(Guild* guild, Player* player, uint32& amount, bool isRepair)
{
    START_HOOK(GUILD_EVENT_ON_MONEY_WITHDRAW);

    // A handler that returns a number changes the amount, for the handlers
    // after it and for the withdrawal itself.
    amount = CallAllFold(*GuildEventBindings, key, amount, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, guild, player, current, isRepair);
    });
}

void ALE::OnMemberDepositMoney(Guild* guild, Player* player, uint32& amount)
{
    START_HOOK(GUILD_EVENT_ON_MONEY_DEPOSIT);

    // A handler that returns a number changes the amount, for the handlers
    // after it and for the deposit itself.
    amount = CallAllFold(*GuildEventBindings, key, amount, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, guild, player, current);
    });
}

void ALE::OnItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId,
    bool isDestBank, uint8 destContainer, uint8 destSlotId)
{
    START_HOOK(GUILD_EVENT_ON_ITEM_MOVE);
    CallAll(*GuildEventBindings, key, guild, player, pItem, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId);
}

void ALE::OnEvent(Guild* guild, uint8 eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank)
{
    START_HOOK(GUILD_EVENT_ON_EVENT);
    CallAll(*GuildEventBindings, key, guild, eventType, playerGuid1, playerGuid2, newRank);
}

void ALE::OnBankEvent(Guild* guild, uint8 eventType, uint8 tabId, uint32 playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId)
{
    START_HOOK(GUILD_EVENT_ON_BANK_EVENT);
    CallAll(*GuildEventBindings, key, guild, eventType, tabId, playerGuid, itemOrMoney, itemStackCount, destTabId);
}
