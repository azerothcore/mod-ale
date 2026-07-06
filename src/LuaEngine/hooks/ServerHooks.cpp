/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEEventMgr.h"
#include "AuctionHouseMgr.h"
#include "Channel.h"
#include "GameEventMgr.h"
#include "ObjectAccessor.h"

using namespace Hooks;

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<ServerEvents>(EVENT);\
    if (!ServerEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EventKey<ServerEvents>(EVENT);\
    if (!ServerEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

bool ALE::OnAddonMessage(Player* sender, uint32 type, std::string& msg, Player* receiver, Guild* guild, Group* group, Channel* channel)
{
    START_HOOK_WITH_RETVAL(ADDON_EVENT_ON_MESSAGE, true);

    // Addon messages are "prefix\tcontent"; the handler receives them split.
    sol::object prefix;
    sol::object content = sol::make_object(lua, sol::nil);

    auto delimiterPosition = msg.find('\t');
    if (delimiterPosition == std::string::npos)
        prefix = sol::make_object(lua, msg);
    else
    {
        prefix = sol::make_object(lua, msg.substr(0, delimiterPosition));
        content = sol::make_object(lua, msg.substr(delimiterPosition + 1));
    }

    // The last argument is whoever the message was sent to:
    // a player, a guild, a group, a channel id, or nil.
    sol::object target = sol::make_object(lua, sol::nil);
    if (receiver)
        target = sol::make_object(lua, PlayerRef(receiver));
    else if (guild)
        target = sol::make_object(lua, GuildRef(guild));
    else if (group)
        target = sol::make_object(lua, GroupRef(group));
    else if (channel)
        target = sol::make_object(lua, channel->GetChannelId());

    return CallAllBool(*ServerEventBindings, key, true, sender, type, prefix, content, target);
}

void ALE::OnTimedEvent(sol::protected_function const& callback, uint64 eventId, uint32 delay, uint32 calls, WorldObject* obj)
{
    LOCK_ALE;
    CallFunction(callback, eventId, delay, calls, ALEBind::ToLuaDynamic(lua, obj));
}

void ALE::OnGameEventStart(uint32 eventid)
{
    START_HOOK(GAME_EVENT_START);
    CallAll(*ServerEventBindings, key, eventid);
}

void ALE::OnGameEventStop(uint32 eventid)
{
    START_HOOK(GAME_EVENT_STOP);
    CallAll(*ServerEventBindings, key, eventid);
}

void ALE::OnLuaStateClose()
{
    START_HOOK(ALE_EVENT_ON_LUA_STATE_CLOSE);
    CallAll(*ServerEventBindings, key);
}

void ALE::OnLuaStateOpen()
{
    START_HOOK(ALE_EVENT_ON_LUA_STATE_OPEN);
    CallAll(*ServerEventBindings, key);
}

// AreaTrigger
bool ALE::OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* pTrigger)
{
    START_HOOK_WITH_RETVAL(TRIGGER_EVENT_ON_TRIGGER, false);
    return CallAllBool(*ServerEventBindings, key, false, pPlayer, pTrigger->entry);
}

// Weather
void ALE::OnChange(Weather* /*weather*/, uint32 zone, WeatherState state, float grade)
{
    START_HOOK(WEATHER_EVENT_ON_CHANGE);
    CallAll(*ServerEventBindings, key, zone, state, grade);
}

// Auction House
void ALE::OnAdd(AuctionHouseObject* /*ah*/, AuctionEntry* entry)
{
    Player* owner = ObjectAccessor::FindPlayer(entry->owner);
    Item* item = sAuctionMgr->GetAItem(entry->item_guid);
    if (!owner || !item)
        return;

    START_HOOK(AUCTION_EVENT_ON_ADD);
    CallAll(*ServerEventBindings, key, entry->Id, owner, item, entry->expire_time, entry->buyout, entry->startbid, entry->bid, entry->bidder);
}

void ALE::OnRemove(AuctionHouseObject* /*ah*/, AuctionEntry* entry)
{
    Player* owner = ObjectAccessor::FindPlayer(entry->owner);
    Item* item = sAuctionMgr->GetAItem(entry->item_guid);
    if (!owner || !item)
        return;

    START_HOOK(AUCTION_EVENT_ON_REMOVE);
    CallAll(*ServerEventBindings, key, entry->Id, owner, item, entry->expire_time, entry->buyout, entry->startbid, entry->bid, entry->bidder);
}

void ALE::OnSuccessful(AuctionHouseObject* /*ah*/, AuctionEntry* entry)
{
    Player* owner = ObjectAccessor::FindPlayer(entry->owner);
    Item* item = sAuctionMgr->GetAItem(entry->item_guid);
    if (!owner || !item)
        return;

    START_HOOK(AUCTION_EVENT_ON_SUCCESSFUL);
    CallAll(*ServerEventBindings, key, entry->Id, owner, item, entry->expire_time, entry->buyout, entry->startbid, entry->bid, entry->bidder);
}

void ALE::OnExpire(AuctionHouseObject* /*ah*/, AuctionEntry* entry)
{
    Player* owner = ObjectAccessor::FindPlayer(entry->owner);
    Item* item = sAuctionMgr->GetAItem(entry->item_guid);
    if (!owner || !item)
        return;

    START_HOOK(AUCTION_EVENT_ON_EXPIRE);
    CallAll(*ServerEventBindings, key, entry->Id, owner, item, entry->expire_time, entry->buyout, entry->startbid, entry->bid, entry->bidder);
}

void ALE::OnOpenStateChange(bool open)
{
    START_HOOK(WORLD_EVENT_ON_OPEN_STATE_CHANGE);
    CallAll(*ServerEventBindings, key, open);
}

void ALE::OnConfigLoad(bool reload, bool isBefore)
{
    START_HOOK(WORLD_EVENT_ON_CONFIG_LOAD);
    CallAll(*ServerEventBindings, key, reload, isBefore);
}

void ALE::OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask)
{
    START_HOOK(WORLD_EVENT_ON_SHUTDOWN_INIT);
    CallAll(*ServerEventBindings, key, code, mask);
}

void ALE::OnShutdownCancel()
{
    START_HOOK(WORLD_EVENT_ON_SHUTDOWN_CANCEL);
    CallAll(*ServerEventBindings, key);
}

void ALE::OnWorldUpdate(uint32 diff)
{
    {
        LOCK_ALE;
        if (ShouldReload())
            _ReloadALE();
    }

    eventMgr->globalProcessor->Update(diff);
    httpManager.HandleHttpResponses();
    queryProcessor.ProcessReadyCallbacks();

    START_HOOK(WORLD_EVENT_ON_UPDATE);
    CallAll(*ServerEventBindings, key, diff);
}

void ALE::OnStartup()
{
    START_HOOK(WORLD_EVENT_ON_STARTUP);
    CallAll(*ServerEventBindings, key);
}

void ALE::OnShutdown()
{
    START_HOOK(WORLD_EVENT_ON_SHUTDOWN);
    CallAll(*ServerEventBindings, key);
}

/* Map */
void ALE::OnCreate(Map* map)
{
    START_HOOK(MAP_EVENT_ON_CREATE);
    CallAll(*ServerEventBindings, key, map);
}

void ALE::OnDestroy(Map* map)
{
    START_HOOK(MAP_EVENT_ON_DESTROY);
    CallAll(*ServerEventBindings, key, map);
}

void ALE::OnPlayerEnter(Map* map, Player* player)
{
    START_HOOK(MAP_EVENT_ON_PLAYER_ENTER);
    CallAll(*ServerEventBindings, key, map, player);
}

void ALE::OnPlayerLeave(Map* map, Player* player)
{
    START_HOOK(MAP_EVENT_ON_PLAYER_LEAVE);
    CallAll(*ServerEventBindings, key, map, player);
}

void ALE::OnUpdate(Map* map, uint32 diff)
{
    START_HOOK(MAP_EVENT_ON_UPDATE);
    CallAll(*ServerEventBindings, key, map, diff);
}

void ALE::OnRemove(GameObject* gameobject)
{
    START_HOOK(WORLD_EVENT_ON_DELETE_GAMEOBJECT);
    CallAll(*ServerEventBindings, key, gameobject);
}

void ALE::OnRemove(Creature* creature)
{
    START_HOOK(WORLD_EVENT_ON_DELETE_CREATURE);
    CallAll(*ServerEventBindings, key, creature);
}
