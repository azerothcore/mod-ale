/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef GLOBALMETHODS_H
#define GLOBALMETHODS_H

#include "BindingMap.h"
#include "ALEDBCRegistry.h"

#include "BanMgr.h"
#include "GameTime.h"
#include "SharedDefines.h"
#include "OutdoorPvPMgr.h"
#include "../../../../src/server/scripts/OutdoorPvP/OutdoorPvPNA.h"


enum BanMode
{
    BAN_ACCOUNT = 1,
    BAN_CHARACTER = 2,
    BAN_IP = 3
};

/***
 * These functions can be used anywhere at any time, including at start-up.
 */
namespace LuaGlobalFunctions
{
    /**
     * Returns Lua engine's name.
     *
     * Always returns "ALEEngine" on ALE.
     *
     * @return string engineName
     */
    int GetLuaEngine(lua_State* L)
    {
        ALE::Push(L, "ALEEngine");
        return 1;
    }

    /**
     * Returns emulator's name.
     *
     * The result will be either `MaNGOS`, `cMaNGOS`, or `TrinityCore`.
     *
     * @return string coreName
     */
    int GetCoreName(lua_State* L)
    {
        ALE::Push(L, CORE_NAME);
        return 1;
    }

    /**
     * Returns config value as a string.
     *
     * @param string name : name of the value
     * @return string value
     */
    int GetConfigValue(lua_State* L)
    {
        const char* key = ALE::CHECKVAL<const char*>(L, 1);
        if (!key) return 0;
        
        std::string val = sConfigMgr->GetOption<std::string>(key, "", false);

        if (val.empty())
        {
            ALE::Push(L, val);
            return 1;
        }

        std::string lower = val;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        if (lower == "true")
        {
            ALE::Push(L, true);
            return 1;
        }
        else if (lower == "false")
        {
            ALE::Push(L, false);
            return 1;
        }
        
        auto intVal = Acore::StringTo<uint32>(val);
        if (intVal) {
            ALE::Push(L, *intVal);
            return 1;
        }
        
        ALE::Push(L, val);
        return 1;
    }

    /**
     * Returns emulator .conf RealmID
     *
     * - for MaNGOS returns the realmID as it is stored in the core.
     * - for TrinityCore returns the realmID as it is in the conf file.
     * @return uint32 realm ID
     */
    int GetRealmID(lua_State* L)
    {
        ALE::Push(L, sConfigMgr->GetOption<uint32>("RealmID", 1));
        return 1;
    }

    /**
     * Returns emulator version
     *
     * - For TrinityCore returns the date of the last revision, e.g. `2015-08-26 22:53:12 +0300`
     * - For cMaNGOS returns the date and time of the last revision, e.g. `2015-09-06 13:18:50`
     * - for MaNGOS returns the version number as string, e.g. `21000`
     *
     * @return string version
     */
    int GetCoreVersion(lua_State* L)
    {
        ALE::Push(L, CORE_VERSION);
        return 1;
    }

    /**
     * Returns emulator's supported expansion.
     *
     * Expansion is 0 for pre-TBC, 1 for TBC, 2 for WotLK, and 3 for Cataclysm.
     *
     * @return int32 expansion
     */
    int GetCoreExpansion(lua_State* L)
    {
        ALE::Push(L, 2);
        return 1;
    }
    
    /**
     * Returns the [Map] pointer of the Lua state. Returns null for the "World" state. 
     *
     * In multistate, this method is only available in the MAP state
     *
     * @return [Map] map
     */
    int GetStateMap(lua_State* L)
    {
        ALE* E = ALE::GetALE(L);
        if (E->GetStateMapId() == ALE_GLOBAL_STATE)
        {
            ALE::Push(L);
            return 1;
        }
        
        // Get the map object and push it
        Map* map = sMapMgr->FindMap(E->GetStateMapId(), 0);
        ALE::Push(L, map);
        return 1;
    }

    /**
     * Returns the map ID of the Lua state. Returns -1 for the "World" state.
     *
     * @return int32 mapId
     */
    int GetStateMapId(lua_State* L)
    {
        ALE* E = ALE::GetALE(L);
        if (E->GetStateMapId() == ALE_GLOBAL_STATE)
        {
            ALE::Push(L, -1);
            return 1;
        }
        ALE::Push(L, (int32)E->GetStateMapId());
        return 1;
    }

    /**
     * Returns the instance ID of the Lua state. Returns 0 for continent maps and the world state.
     *
     * @return uint32 instanceId
     */
    int GetStateInstanceId(lua_State* L)
    {
        ALE* E = ALE::GetALE(L);
        ALE::Push(L, E->GetStateInstanceId());
        return 1;
    }

    /**
     * Returns [Quest] template
     *
     * @param uint32 questId : [Quest] entry ID
     * @return [Quest] quest
     */
    int GetQuest(lua_State* L)
    {
        uint32 questId = ALE::CHECKVAL<uint32>(L, 1);

        ALE::Push(L, eObjectMgr->GetQuestTemplate(questId));
        return 1;
    }

    /**
     * Finds and Returns [Player] by guid if found
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param ObjectGuid guid : guid of the [Player], you can get it with [Object:GetGUID]
     * @return [Player] player
     */
    int GetPlayerByGUID(lua_State* L)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
        ALE::Push(L, eObjectAccessor()FindPlayer(guid));
        return 1;
    }

    /**
     * Finds and Returns [Player] by name if found
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param string name : name of the [Player]
     * @return [Player] player
     */
    int GetPlayerByName(lua_State* L)
    {
        const char* name = ALE::CHECKVAL<const char*>(L, 1);
        ALE::Push(L, eObjectAccessor()FindPlayerByName(name));
        return 1;
    }

    /**
     * Returns game time in seconds
     *
     * @return uint32 time
     */
    int GetGameTime(lua_State* L)
    {
        ALE::Push(L, GameTime::GetGameTime().count());
        return 1;
    }

    /**
     * Returns a table with all the current [Player]s in the world
     *
     * Does not return players that may be teleporting or otherwise not on any map.
     *
     *     enum TeamId
     *     {
     *         TEAM_ALLIANCE = 0,
     *         TEAM_HORDE = 1,
     *         TEAM_NEUTRAL = 2
     *     };
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param [TeamId] team = TEAM_NEUTRAL : optional check team of the [Player], Alliance, Horde or Neutral (All)
     * @param bool onlyGM = false : optional check if GM only
     * @return table worldPlayers
     */
    int GetPlayersInWorld(lua_State* L)
    {
        uint32 team = ALE::CHECKVAL<uint32>(L, 1, TEAM_NEUTRAL);
        bool onlyGM = ALE::CHECKVAL<bool>(L, 2, false);

        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        {
            std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
            const HashMapHolder<Player>::MapType& m = eObjectAccessor()GetPlayers();
            for (HashMapHolder<Player>::MapType::const_iterator it = m.begin(); it != m.end(); ++it)
            {
                if (Player* player = it->second)
                {
                    if (!player->IsInWorld())
                        continue;

                    if ((team == TEAM_NEUTRAL || player->GetTeamId() == team) && (!onlyGM || player->IsGameMaster()))
                    {
                        ALE::Push(L, player);
                        lua_rawseti(L, tbl, ++i);
                    }
                }
            }
        }

        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a [Guild] by name.
     *
     * @param string name
     * @return [Guild] guild : the Guild, or `nil` if it doesn't exist
     */
    int GetGuildByName(lua_State* L)
    {
        const char* name = ALE::CHECKVAL<const char*>(L, 1);
        ALE::Push(L, eGuildMgr->GetGuildByName(name));
        return 1;
    }

    /**
     * Returns a [Map] by ID.
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param uint32 mapId : see [Map.dbc](https://github.com/cmangos/issues/wiki/Map.dbc)
     * @param uint32 instanceId = 0 : required if the map is an instance, otherwise don't pass anything
     * @return [Map] map : the Map, or `nil` if it doesn't exist
     */
    int GetMapById(lua_State* L)
    {
        uint32 mapid = ALE::CHECKVAL<uint32>(L, 1);
        uint32 instance = ALE::CHECKVAL<uint32>(L, 2, 0);

        ALE::Push(L, eMapMgr->FindMap(mapid, instance));
        return 1;
    }

    /**
     * Returns [Guild] by the leader's GUID
     *
     * @param ObjectGuid guid : the guid of a [Guild] leader
     * @return [Guild] guild, or `nil` if it doesn't exist
     */
    int GetGuildByLeaderGUID(lua_State* L)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);

        ALE::Push(L, eGuildMgr->GetGuildByLeader(guid));
        return 1;
    }

    /**
     * Returns the amount of [Player]s in the world.
     *
     * @return uint32 count
     */
    int GetPlayerCount(lua_State* L)
    {
        ALE::Push(L, eWorldSessionMgr->GetActiveSessionCount());
        return 1;
    }

    /**
     * Builds a [Player]'s GUID
     *
     * [Player] GUID consist of low GUID and type ID
     *
     * [Player] and [Creature] for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [Player]
     * @return ObjectGuid guid
     */
    int GetPlayerGUID(lua_State* L)
    {
        uint32 lowguid = ALE::CHECKVAL<uint32>(L, 1);
        ALE::Push(L, MAKE_NEW_GUID(lowguid, 0, HIGHGUID_PLAYER));
        return 1;
    }

    /**
     * Builds an [Item]'s GUID.
     *
     * [Item] GUID consist of low GUID and type ID
     * [Player] and [Item] for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [Item]
     * @return ObjectGuid guid
     */
    int GetItemGUID(lua_State* L)
    {
        uint32 lowguid = ALE::CHECKVAL<uint32>(L, 1);
        ALE::Push(L, MAKE_NEW_GUID(lowguid, 0, HIGHGUID_ITEM));
        return 1;
    }

    /**
    * Returns the [ItemTemplate] for the specified item ID.  The ItemTemplate contains all static data about an item, such as name, quality, stats, required level, and more.
    *
    * @param uint32 itemID : the item entry ID from `item_template` to look up
    * @return [ItemTemplate] itemTemplate
    */
    int GetItemTemplate(lua_State* L)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
        ALE::Push(L, eObjectMgr->GetItemTemplate(entry));
        return 1;
    }

    /**
     * Builds a [GameObject]'s GUID.
     *
     * A GameObject's GUID consist of entry ID, low GUID and type ID
     *
     * A [Player] and GameObject for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [GameObject]
     * @param uint32 entry : entry ID of the [GameObject]
     * @return ObjectGuid guid
     */
    int GetObjectGUID(lua_State* L)
    {
        uint32 lowguid = ALE::CHECKVAL<uint32>(L, 1);
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, MAKE_NEW_GUID(lowguid, entry, HIGHGUID_GAMEOBJECT));
        return 1;
    }

    /**
     * Builds a [Creature]'s GUID.
     *
     * [Creature] GUID consist of entry ID, low GUID and type ID
     *
     * [Player] and [Creature] for example can have the same low GUID but not GUID.
     *
     * @param uint32 lowguid : low GUID of the [Creature]
     * @param uint32 entry : entry ID of the [Creature]
     * @return ObjectGuid guid
     */
    int GetUnitGUID(lua_State* L)
    {
        uint32 lowguid = ALE::CHECKVAL<uint32>(L, 1);
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, MAKE_NEW_GUID(lowguid, entry, HIGHGUID_UNIT));
        return 1;
    }

    /**
     * Returns the low GUID from a GUID.
     *
     * A GUID consists of a low GUID, type ID, and possibly an entry ID depending on the type ID.
     *
     * Low GUID is an ID to distinct the objects of the same type.
     *
     * [Player] and [Creature] for example can have the same low GUID but not GUID.
     *
     * On TrinityCore all low GUIDs are different for all objects of the same type.
     * For example creatures in instances are assigned new GUIDs when the Map is created.
     *
     * On MaNGOS and cMaNGOS low GUIDs are unique only on the same map.
     * For example creatures in instances use the same low GUID assigned for that spawn in the database.
     * This is why to identify a creature you have to know the instanceId and low GUID. See [Map:GetIntstanceId]
     *
     * @param ObjectGuid guid : GUID of an [Object]
     * @return uint32 lowguid : low GUID of the [Object]
     */
    int GetGUIDLow(lua_State* L)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);

        ALE::Push(L, guid.GetCounter());
        return 1;
    }

    /**
     * Returns an chat link for an [Item].
     *
     *     enum LocaleConstant
     *     {
     *         LOCALE_enUS = 0,
     *         LOCALE_koKR = 1,
     *         LOCALE_frFR = 2,
     *         LOCALE_deDE = 3,
     *         LOCALE_zhCN = 4,
     *         LOCALE_zhTW = 5,
     *         LOCALE_esES = 6,
     *         LOCALE_esMX = 7,
     *         LOCALE_ruRU = 8
     *     };
     *
     * @param uint32 entry : entry ID of an [Item]
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [Item] name in
     * @return string itemLink
     */
    int GetItemLink(lua_State* L)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
        uint8 locale = ALE::CHECKVAL<uint8>(L, 2, DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            return luaL_argerror(L, 2, "valid LocaleConstant expected");

        const ItemTemplate* temp = eObjectMgr->GetItemTemplate(entry);
        if (!temp)
            return luaL_argerror(L, 1, "valid ItemEntry expected");

        std::string name = temp->Name1;
        if (ItemLocale const* il = eObjectMgr->GetItemLocale(entry))
            ObjectMgr::GetLocaleString(il->Name, static_cast<LocaleConstant>(locale), name);

        std::ostringstream oss;
        oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
            "|Hitem:" << entry << ":0:" <<
            "0:0:0:0:" <<
            "0:0:0:0|h[" << name << "]|h|r";

        ALE::Push(L, oss.str());
        return 1;
    }

    /**
     * Returns the type ID from a GUID.
     *
     * Type ID is different for each type ([Player], [Creature], [GameObject], etc.).
     *
     * GUID consist of entry ID, low GUID, and type ID.
     *
     * @param ObjectGuid guid : GUID of an [Object]
     * @return int32 typeId : type ID of the [Object]
     */
    int GetGUIDType(lua_State* L)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
        ALE::Push(L, static_cast<int>(guid.GetHigh()));
        return 1;
    }

    /**
     * Returns the entry ID from a GUID.
     *
     * GUID consist of entry ID, low GUID, and type ID.
     *
     * @param ObjectGuid guid : GUID of an [Creature] or [GameObject]
     * @return uint32 entry : entry ID, or `0` if `guid` is not a [Creature] or [GameObject]
     */
    int GetGUIDEntry(lua_State* L)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
        ALE::Push(L, guid.GetEntry());
        return 1;
    }

    /**
     * Returns the byte size in bytes (2-9) of the ObjectGuid when packed.
     *
     * @param ObjectGuid guid : the ObjectGuid to get packed size for
     * @return number size
     */
    int GetPackedGUIDSize(lua_State* L)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
        PackedGuid packedGuid(guid);
        ALE::Push(L, static_cast<int>(packedGuid.size()));
        return 1;
    }

    /**
     * Returns the area or zone's name.
     *
     *     enum LocaleConstant
     *     {
     *         LOCALE_enUS = 0,
     *         LOCALE_koKR = 1,
     *         LOCALE_frFR = 2,
     *         LOCALE_deDE = 3,
     *         LOCALE_zhCN = 4,
     *         LOCALE_zhTW = 5,
     *         LOCALE_esES = 6,
     *         LOCALE_esMX = 7,
     *         LOCALE_ruRU = 8
     *     };
     *
     * @param uint32 areaOrZoneId : area ID or zone ID
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the name in
     * @return string areaOrZoneName
     */
    int GetAreaName(lua_State* L)
    {
        uint32 areaOrZoneId = ALE::CHECKVAL<uint32>(L, 1);
        uint8 locale = ALE::CHECKVAL<uint8>(L, 2, DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            return luaL_argerror(L, 2, "valid LocaleConstant expected");

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaOrZoneId);

        if (!areaEntry)
            return luaL_argerror(L, 1, "valid Area or Zone ID expected");

        ALE::Push(L, areaEntry->area_name[locale]);
        return 1;
    }

    /**
     * Returns the currently active game events.
     *
     * @return table activeEvents
     */
    int GetActiveGameEvents(lua_State* L)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 counter = 1;
        GameEventMgr::ActiveEvents const& activeEvents = eGameEventMgr->GetActiveEventList();

        for (GameEventMgr::ActiveEvents::const_iterator i = activeEvents.begin(); i != activeEvents.end(); ++i)
        {
            ALE::Push(L, *i);
            lua_rawseti(L, tbl, counter);

            counter++;
        }

        lua_settop(L, tbl);
        return 1;
    }

    static int RegisterEntryHelper(lua_State* L, int regtype)
    {
        uint32 id = ALE::CHECKVAL<uint32>(L, 1);
        uint32 ev = ALE::CHECKVAL<uint32>(L, 2);
        luaL_checktype(L, 3, LUA_TFUNCTION);
        uint32 shots = ALE::CHECKVAL<uint32>(L, 4, 0);

        lua_pushvalue(L, 3);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (functionRef >= 0)
            return ALE::GetALE(L)->Register(L, regtype, id, ObjectGuid(), 0, ev, functionRef, shots);
        else
            luaL_argerror(L, 3, "unable to make a ref to function");
        return 0;
    }

    static int RegisterEventHelper(lua_State* L, int regtype)
    {
        uint32 ev = ALE::CHECKVAL<uint32>(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);
        uint32 shots = ALE::CHECKVAL<uint32>(L, 3, 0);

        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (functionRef >= 0)
            return ALE::GetALE(L)->Register(L, regtype, 0, ObjectGuid(), 0, ev, functionRef, shots);
        else
            luaL_argerror(L, 2, "unable to make a ref to function");
        return 0;
    }

    static int RegisterUniqueHelper(lua_State* L, int regtype)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
        uint32 instanceId = ALE::CHECKVAL<uint32>(L, 2);
        uint32 ev = ALE::CHECKVAL<uint32>(L, 3);
        luaL_checktype(L, 4, LUA_TFUNCTION);
        uint32 shots = ALE::CHECKVAL<uint32>(L, 5, 0);

        lua_pushvalue(L, 4);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (functionRef >= 0)
            return ALE::GetALE(L)->Register(L, regtype, 0, guid, instanceId, ev, functionRef, shots);
        else
            luaL_argerror(L, 4, "unable to make a ref to function");
        return 0;
    }

    /**
     * Registers a server event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, SERVER_EVENT_ON_NETWORK_START, "", "", "Not Implemented"]
     * @values [2, SERVER_EVENT_ON_NETWORK_STOP, "", "", "Not Implemented"]
     * @values [3, SERVER_EVENT_ON_SOCKET_OPEN, "", "", "Not Implemented"]
     * @values [4, SERVER_EVENT_ON_SOCKET_CLOSE, "", "", "Not Implemented"]
     * @values [5, SERVER_EVENT_ON_PACKET_RECEIVE, "WORLD", <event: number, packet: WorldPacket, player: Player>, "Player only if accessible. Can return false, newPacket"]
     * @values [6, SERVER_EVENT_ON_PACKET_RECEIVE_UNKNOWN, "", "", "Not Implemented"]
     * @values [7, SERVER_EVENT_ON_PACKET_SEND, "WORLD", <event: number, packet: WorldPacket, player: Player>, "Player only if accessible. Can return false, newPacket"]
     * @values [8, WORLD_EVENT_ON_OPEN_STATE_CHANGE, "WORLD", <event: number, open: boolean>, "Needs core support on Mangos"]
     * @values [9, WORLD_EVENT_ON_CONFIG_LOAD, "WORLD", <event: number, reload: boolean>, ""]
     * @values [11, WORLD_EVENT_ON_SHUTDOWN_INIT, "WORLD", <event: number, code: number, mask: number>, ""]
     * @values [12, WORLD_EVENT_ON_SHUTDOWN_CANCEL, "WORLD", <event: number>, ""]
     * @values [13, WORLD_EVENT_ON_UPDATE, "WORLD", <event: number, diff: number>, ""]
     * @values [14, WORLD_EVENT_ON_STARTUP, "WORLD", <event: number>, ""]
     * @values [15, WORLD_EVENT_ON_SHUTDOWN, "WORLD", <event: number>, ""]
     * @values [16, ALE_EVENT_ON_LUA_STATE_CLOSE, "ALL", <event: number>, "Triggers just before shutting down ALE (on shutdown and restart)"]
     * @values [17, MAP_EVENT_ON_CREATE, "MAP", <event: number, map: Map>, ""]
     * @values [18, MAP_EVENT_ON_DESTROY, "MAP", <event: number, map: Map>, ""]
     * @values [19, MAP_EVENT_ON_GRID_LOAD, "", "", "Not Implemented"]
     * @values [20, MAP_EVENT_ON_GRID_UNLOAD, "", "", "Not Implemented"]
     * @values [21, MAP_EVENT_ON_PLAYER_ENTER, "MAP", <event: number, map: Map, player: Player>, ""]
     * @values [22, MAP_EVENT_ON_PLAYER_LEAVE, "MAP", <event: number, map: Map, player: Player>, ""]
     * @values [23, MAP_EVENT_ON_UPDATE, "MAP", <event: number, map: Map, diff: number>, ""]
     * @values [24, TRIGGER_EVENT_ON_TRIGGER, "MAP", <event: number, player: Player, triggerId: number>, "Can return true"]
     * @values [25, WEATHER_EVENT_ON_CHANGE, "WORLD", <event: number, zoneId: number, state: number, grade: number>, ""]
     * @values [26, AUCTION_EVENT_ON_ADD, "WORLD", <event: number, auctionId: number, owner: Player, item: Item, expireTime: number, buyout: number, startBid: number, currentBid: number, bidderGUIDLow: number>, ""]
     * @values [27, AUCTION_EVENT_ON_REMOVE, "WORLD", <event: number, auctionId: number, owner: Player, item: Item, expireTime: number, buyout: number, startBid: number, currentBid: number, bidderGUIDLow: number>, ""]
     * @values [28, AUCTION_EVENT_ON_SUCCESSFUL, "WORLD", <event: number, auctionId: number, owner: Player, item: Item, expireTime: number, buyout: number, startBid: number, currentBid: number, bidderGUIDLow: number>, ""]
     * @values [29, AUCTION_EVENT_ON_EXPIRE, "WORLD", <event: number, auctionId: number, owner: Player, item: Item, expireTime: number, buyout: number, startBid: number, currentBid: number, bidderGUIDLow: number>, ""]
     * @values [30, ADDON_EVENT_ON_MESSAGE, "WORLD", <event: number, sender: Player, type: number, prefix: string, msg: string, target: nil|Player|Guild|Group|number>, "Target can be nil/whisper_target/guild/group/channel. Can return false"]
     * @values [31, WORLD_EVENT_ON_DELETE_CREATURE, "MAP", <event: number, creature: Creature>, ""]
     * @values [32, WORLD_EVENT_ON_DELETE_GAMEOBJECT, "MAP", <event: number, gameobject: GameObject>, ""]
     * @values [33, ALE_EVENT_ON_LUA_STATE_OPEN, "ALL", <event: number>, "Triggers after all scripts are loaded"]
     * @values [34, GAME_EVENT_START, "WORLD", <event: number, gameeventid: number>, ""]
     * @values [35, GAME_EVENT_STOP, "WORLD", <event: number, gameeventid: number>, ""]
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : server event ID, refer to table above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterServerEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_SERVER);
    }

    /**
     * Registers a [Player] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, PLAYER_EVENT_ON_CHARACTER_CREATE, "WORLD", <event: number, player: Player>, ""]
     * @values [2, PLAYER_EVENT_ON_CHARACTER_DELETE, "WORLD", <event: number, guid: number>, ""]
     * @values [3, PLAYER_EVENT_ON_LOGIN, "WORLD", <event: number, player: Player>, ""]
     * @values [4, PLAYER_EVENT_ON_LOGOUT, "WORLD", <event: number, player: Player>, ""]
     * @values [5, PLAYER_EVENT_ON_SPELL_CAST, "MAP", <event: number, player: Player, spell: Spell, skipCheck: boolean>, ""]
     * @values [6, PLAYER_EVENT_ON_KILL_PLAYER, "MAP", <event: number, killer: Player, killed: Player>, ""]
     * @values [7, PLAYER_EVENT_ON_KILL_CREATURE, "MAP", <event: number, killer: Player, killed: Creature>, ""]
     * @values [8, PLAYER_EVENT_ON_KILLED_BY_CREATURE, "MAP", <event: number, killer: Creature, killed: Player>, ""]
     * @values [9, PLAYER_EVENT_ON_DUEL_REQUEST, "MAP", <event: number, target: Player, challenger: Player>, ""]
     * @values [10, PLAYER_EVENT_ON_DUEL_START, "MAP", <event: number, player1: Player, player2: Player>, ""]
     * @values [11, PLAYER_EVENT_ON_DUEL_END, "MAP", <event: number, winner: Player, loser: Player, type: number>, ""]
     * @values [12, PLAYER_EVENT_ON_GIVE_XP, "MAP", <event: number, player: Player, amount: number, victim: Unit, source: number>, "Can return new XP amount"]
     * @values [13, PLAYER_EVENT_ON_LEVEL_CHANGE, "MAP", <event: number, player: Player, oldLevel: number>, ""]
     * @values [14, PLAYER_EVENT_ON_MONEY_CHANGE, "MAP", <event: number, player: Player, amount: number>, "Can return new money amount"]
     * @values [15, PLAYER_EVENT_ON_REPUTATION_CHANGE, "MAP", <event: number, player: Player, factionId: number, standing: number, incremental: boolean>, "Can return new standing. If standing == -1 it will prevent default action (rep gain)"]
     * @values [16, PLAYER_EVENT_ON_TALENTS_CHANGE, "MAP", <event: number, player: Player, points: number>, ""]
     * @values [17, PLAYER_EVENT_ON_TALENTS_RESET, "MAP", <event: number, player: Player, noCost: boolean>, ""]
     * @values [18, PLAYER_EVENT_ON_CHAT, "WORLD", <event: number, player: Player, msg: string, Type: number, lang: number>, "Can return false, newMessage"]
     * @values [19, PLAYER_EVENT_ON_WHISPER, "WORLD", <event: number, player: Player, msg: string, Type: number, lang: number, receiver: Player>, "Can return false, newMessage"]
     * @values [20, PLAYER_EVENT_ON_GROUP_CHAT, "WORLD", <event: number, player: Player, msg: string, Type: number, lang: number, group: Group>, "Can return false, newMessage"]
     * @values [21, PLAYER_EVENT_ON_GUILD_CHAT, "WORLD", <event: number, player: Player, msg: string, Type: number, lang: number, guild: Guild>, "Can return false, newMessage"]
     * @values [22, PLAYER_EVENT_ON_CHANNEL_CHAT, "WORLD", <event: number, player: Player, msg: string, Type: number, lang: number, channel: number>, "Channel is negative for custom channels. Can return false, newMessage"]
     * @values [23, PLAYER_EVENT_ON_EMOTE, "MAP", <event: number, player: Player, emote: number>, "Not triggered on any known emote"]
     * @values [24, PLAYER_EVENT_ON_TEXT_EMOTE, "MAP", <event: number, player: Player, textEmote: number, emoteNum: number, guid: number>, ""]
     * @values [25, PLAYER_EVENT_ON_SAVE, "MAP", <event: number, player: Player>, ""]
     * @values [26, PLAYER_EVENT_ON_BIND_TO_INSTANCE, "MAP", <event: number, player: Player, difficulty: number, mapid: number, permanent: boolean>, ""]
     * @values [27, PLAYER_EVENT_ON_UPDATE_ZONE, "MAP", <event: number, player: Player, newZone: number, newArea: number>, ""]
     * @values [28, PLAYER_EVENT_ON_MAP_CHANGE, "MAP", <event: number, player: Player>, ""]
     * @values [29, PLAYER_EVENT_ON_EQUIP, "MAP", <event: number, player: Player, item: Item, bag: number, slot: number>, ""]
     * @values [30, PLAYER_EVENT_ON_FIRST_LOGIN, "WORLD", <event: number, player: Player>, ""]
     * @values [31, PLAYER_EVENT_ON_CAN_USE_ITEM, "MAP", <event: number, player: Player, itemEntry: number>, "Can return InventoryResult enum value"]
     * @values [32, PLAYER_EVENT_ON_LOOT_ITEM, "MAP", <event: number, player: Player, item: Item, count: number>, ""]
     * @values [33, PLAYER_EVENT_ON_ENTER_COMBAT, "MAP", <event: number, player: Player, enemy: Unit>, ""]
     * @values [34, PLAYER_EVENT_ON_LEAVE_COMBAT, "MAP", <event: number, player: Player>, ""]
     * @values [35, PLAYER_EVENT_ON_REPOP, "MAP", <event: number, player: Player>, ""]
     * @values [36, PLAYER_EVENT_ON_RESURRECT, "MAP", <event: number, player: Player>, ""]
     * @values [37, PLAYER_EVENT_ON_LOOT_MONEY, "MAP", <event: number, player: Player, amount: number>, ""]
     * @values [38, PLAYER_EVENT_ON_QUEST_ABANDON, "MAP", <event: number, player: Player, questId: number>, ""]
     * @values [39, PLAYER_EVENT_ON_LEARN_TALENTS, "MAP", <event: number, player: Player, talentId: number, talentRank: number, spellid: number>, ""]
     * @values [42, PLAYER_EVENT_ON_COMMAND, "WORLD", <event: number, player: Player, command: string, chatHandler: ChatHandler>, "Player is nil if command used from console. Can return false"]
     * @values [43, PLAYER_EVENT_ON_PET_ADDED_TO_WORLD, "MAP", <event: number, player: Player, pet: Creature>, ""]
     * @values [44, PLAYER_EVENT_ON_LEARN_SPELL, "MAP", <event: number, player: Player, spellId: number>, ""]
     * @values [45, PLAYER_EVENT_ON_ACHIEVEMENT_COMPLETE, "MAP", <event: number, player: Player, achievement: AchievementEntry>, ""]
     * @values [46, PLAYER_EVENT_ON_FFAPVP_CHANGE, "MAP", <event: number, player: Player, hasFfaPvp: boolean>, ""]
     * @values [47, PLAYER_EVENT_ON_UPDATE_AREA, "MAP", <event: number, player: Player, oldArea: number, newArea: number>, ""]
     * @values [48, PLAYER_EVENT_ON_CAN_INIT_TRADE, "MAP", <event: number, player: Player, target: Player>, "Can return false to prevent the trade"]
     * @values [49, PLAYER_EVENT_ON_CAN_SEND_MAIL, "MAP", <event: number, player: Player, receiverGuid: number, mailbox: GameObject, subject: string, body: string, money: number, cod: number, item: Item>, "Can return false to prevent sending the mail"]
     * @values [50, PLAYER_EVENT_ON_CAN_JOIN_LFG, "MAP", <event: number, player: Player, roles: number, dungeons: number, comment: string>, "Can return false to prevent queueing"]
     * @values [51, PLAYER_EVENT_ON_QUEST_REWARD_ITEM, "MAP", <event: number, player: Player, item: Item, count: number>, ""]
     * @values [52, PLAYER_EVENT_ON_CREATE_ITEM, "MAP", <event: number, player: Player, item: Item, count: number>, ""]
     * @values [53, PLAYER_EVENT_ON_STORE_NEW_ITEM, "MAP", <event: number, player: Player, item: Item, count: number>, ""]
     * @values [54, PLAYER_EVENT_ON_COMPLETE_QUEST, "MAP", <event: number, player: Player, quest: Quest>, ""]
     * @values [55, PLAYER_EVENT_ON_CAN_GROUP_INVITE, "MAP", <event: number, player: Player, memberName: string>, "Can return false to prevent inviting"]
     * @values [56, PLAYER_EVENT_ON_GROUP_ROLL_REWARD_ITEM, "MAP", <event: number, player: Player, item: Item, count: number, voteType: number, roll: Roll>, ""]
     * @values [57, PLAYER_EVENT_ON_BG_DESERTION, "MAP", <event: number, player: Player, type: number>, ""]
     * @values [58, PLAYER_EVENT_ON_PET_KILL, "MAP", <event: number, player: Player, killer: Creature>, ""]
     * @values [59, PLAYER_EVENT_ON_CAN_RESURRECT, "MAP", <event: number, player: Player>, ""]
     * @values [60, PLAYER_EVENT_ON_CAN_UPDATE_SKILL, "MAP", <event: number, player: Player, skill_id: number>, "Can return true or false"]
     * @values [61, PLAYER_EVENT_ON_BEFORE_UPDATE_SKILL, "MAP", <event: number, player: Player, skill_id: number, value: number, max: number, step: number>, "Can return new amount"]
     * @values [62, PLAYER_EVENT_ON_UPDATE_SKILL, "MAP", <event: number, player: Player, skill_id: number, value: number, max: number, step: number, new_value: number>, ""]
     * @values [63, PLAYER_EVENT_ON_QUEST_ACCEPT, "MAP", <event: number, player: Player, quest: Quest>, ""]
     * @values [64, PLAYER_EVENT_ON_AURA_APPLY, "MAP", <event: number, player: Player, aura: Aura>, ""]
     * @values [65, PLAYER_EVENT_ON_HEAL, "MAP", <event: number, player: Player, target: Unit, heal: number>, "Can return new heal amount"]
     * @values [66, PLAYER_EVENT_ON_DAMAGE, "MAP", <event: number, player: Player, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [67, PLAYER_EVENT_ON_AURA_REMOVE, "MAP", <event: number, player: Player, aura: Aura, remove_mode: number>, ""]
     * @values [68, PLAYER_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK, "MAP", <event: number, player: Player, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [69, PLAYER_EVENT_ON_MODIFY_MELEE_DAMAGE, "MAP", <event: number, player: Player, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [70, PLAYER_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN, "MAP", <event: number, player: Player, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [71, PLAYER_EVENT_ON_MODIFY_HEAL_RECEIVED, "MAP", <event: number, player: Player, target: Unit, heal: number, spellInfo: SpellInfo>, "Can return new heal amount"]
     * @values [72, PLAYER_EVENT_ON_DEAL_DAMAGE, "MAP", <event: number, player: Player, target: Unit, damage: number, damagetype: number>, "Can return new damage amount"]
     * @values [73, PLAYER_EVENT_ON_RELEASED_GHOST, "MAP", <event: number, player: Player>, ""]
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Player] event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterPlayerEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_PLAYER);
    }

    /**
     * Registers a [Guild] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GUILD_EVENT_ON_ADD_MEMBER, "WORLD", <event: number, guild: Guild, player: Player, rank: number>, ""]
     * @values [2, GUILD_EVENT_ON_REMOVE_MEMBER, "WORLD", <event: number, guild: Guild, player: Player, isDisbanding: boolean>, ""]
     * @values [3, GUILD_EVENT_ON_MOTD_CHANGE, "WORLD", <event: number, guild: Guild, newMotd: string>, ""]
     * @values [4, GUILD_EVENT_ON_INFO_CHANGE, "WORLD", <event: number, guild: Guild, newInfo: string>, ""]
     * @values [5, GUILD_EVENT_ON_CREATE, "WORLD", <event: number, guild: Guild, leader: Player, name: string>, "Not on TC"]
     * @values [6, GUILD_EVENT_ON_DISBAND, "WORLD", <event: number, guild: Guild>, ""]
     * @values [7, GUILD_EVENT_ON_MONEY_WITHDRAW, "WORLD", <event: number, guild: Guild, player: Player, amount: number, isRepair: boolean>, "Can return new money amount"]
     * @values [8, GUILD_EVENT_ON_MONEY_DEPOSIT, "WORLD", <event: number, guild: Guild, player: Player, amount: number>, "Can return new money amount"]
     * @values [9, GUILD_EVENT_ON_ITEM_MOVE, "WORLD", <event: number, guild: Guild, player: Player, item: Item, isSrcBank: boolean, srcContainer: number, srcSlotId: number, isDestBank: boolean, destContainer: number, destSlotId: number>, "TODO"]
     * @values [10, GUILD_EVENT_ON_EVENT, "WORLD", <event: number, guild: Guild, eventType: number, plrGUIDLow1: number, plrGUIDLow2: number, newRank: number>, "TODO"]
     * @values [11, GUILD_EVENT_ON_BANK_EVENT, "WORLD", <event: number, guild: Guild, eventType: number, tabId: number, playerGUIDLow: number, itemOrMoney: number, itemStackCount: number, destTabId: number>, ""]
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Guild] event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGuildEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_GUILD);
    }

    /**
     * Registers a [Group] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GROUP_EVENT_ON_MEMBER_ADD, "WORLD", <event: number, group: Group, guid: number>, ""]
     * @values [2, GROUP_EVENT_ON_MEMBER_INVITE, "WORLD", <event: number, group: Group, guid: number>, ""]
     * @values [3, GROUP_EVENT_ON_MEMBER_REMOVE, "WORLD", <event: number, group: Group, guid: number, method: number, kicker: Player, reason: string>, ""]
     * @values [4, GROUP_EVENT_ON_LEADER_CHANGE, "WORLD", <event: number, group: Group, newLeaderGuid: number, oldLeaderGuid: number>, ""]
     * @values [5, GROUP_EVENT_ON_DISBAND, "WORLD", <event: number, group: Group>, ""]
     * @values [6, GROUP_EVENT_ON_CREATE, "WORLD", <event: number, group: Group, leaderGuid: number, groupType: number>, ""]
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Group] event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGroupEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_GROUP);
    }

    /**
     * Registers a [BattleGround] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, BG_EVENT_ON_START, "MAP", <event: number, bg: BattleGround, bgId: number, instanceId: number>, "Needs to be added to TC"]
     * @values [2, BG_EVENT_ON_END, "MAP", <event: number, bg: BattleGround, bgId: number, instanceId: number, winner: number>, "Needs to be added to TC"]
     * @values [3, BG_EVENT_ON_CREATE, "MAP", <event: number, bg: BattleGround, bgId: number, instanceId: number>, "Needs to be added to TC"]
     * @values [4, BG_EVENT_ON_PRE_DESTROY, "MAP", <event: number, bg: BattleGround, bgId: number, instanceId: number>, "Needs to be added to TC"]
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [BattleGround] event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterBGEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_BG);
    }

    /**
     * Registers a [WorldPacket] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [5, PACKET_EVENT_ON_PACKET_RECEIVE, "WORLD", <event: number, packet: WorldPacket, player: Player>, "Player only if accessible. Can return false, newPacket"]
     * @values [6, PACKET_EVENT_ON_PACKET_RECEIVE_UNKNOWN, "", "", "Not Implemented"]
     * @values [7, PACKET_EVENT_ON_PACKET_SEND, "WORLD", <event: number, packet: WorldPacket, player: Player>, "Player only if accessible. Can return false, newPacket"]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : opcode
     * @param uint32 event : packet event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterPacketEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_PACKET);
    }

    /**
     * Registers a [Creature] gossip event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GOSSIP_EVENT_ON_HELLO, "MAP", <event: number, player: Player, object: Creature|GameObject|Item>, "Object is the Creature/GameObject/Item. Can return false to do default action."]
     * @values [2, GOSSIP_EVENT_ON_SELECT, "MAP", <event: number, player: Player, object: Creature|GameObject|Item|Player, sender: number, intid: number, code: string, menu_id: number>, "Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action."]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 event : [Creature] gossip event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterCreatureGossipEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_CREATURE_GOSSIP);
    }

    /**
     * Registers a [GameObject] gossip event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GOSSIP_EVENT_ON_HELLO, "MAP", <event: number, player: Player, object: Creature|GameObject|Item>, "Object is the Creature/GameObject/Item. Can return false to do default action."]
     * @values [2, GOSSIP_EVENT_ON_SELECT, "MAP", <event: number, player: Player, object: Creature|GameObject|Item|Player, sender: number, intid: number, code: string, menu_id: number>, "Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action."]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [GameObject] entry Id
     * @param uint32 event : [GameObject] gossip event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGameObjectGossipEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_GAMEOBJECT_GOSSIP);
    }

    /**
     * Registers an [Item] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, ITEM_EVENT_ON_DUMMY_EFFECT, "MAP", <event: number, caster: WorldObject, spellid: number, effindex: number, item: Item>, ""]
     * @values [2, ITEM_EVENT_ON_USE, "MAP", <event: number, player: Player, item: Item, target: nil|GameObject|Item|Corpse|Unit|WorldObject>, "Can return false to stop the spell casting"]
     * @values [3, ITEM_EVENT_ON_QUEST_ACCEPT, "MAP", <event: number, player: Player, item: Item, quest: Quest>, "Can return true"]
     * @values [4, ITEM_EVENT_ON_EXPIRE, "MAP", <event: number, player: Player, itemid: number>, "Can return true"]
     * @values [5, ITEM_EVENT_ON_REMOVE, "MAP", <event: number, player: Player, item: Item>, "Can return true"]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Item] entry Id
     * @param uint32 event : [Item] event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterItemEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_ITEM);
    }

    /**
     * Registers an [Item] gossip event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GOSSIP_EVENT_ON_HELLO, "MAP", <event: number, player: Player, object: Creature|GameObject|Item>, "Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting."]
     * @values [2, GOSSIP_EVENT_ON_SELECT, "MAP", <event: number, player: Player, object: Creature|GameObject|Item|Player, sender: number, intid: number, code: string, menu_id: number>, "Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action."]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Item] entry Id
     * @param uint32 event : [Item] gossip event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterItemGossipEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_ITEM_GOSSIP);
    }

    /**
     * Registers a [Map] event handler for all instance of a [Map].
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, INSTANCE_EVENT_ON_INITIALIZE, "MAP", <event: number, instance_data: string, map: Map>, ""]
     * @values [2, INSTANCE_EVENT_ON_LOAD, "MAP", <event: number, instance_data: string, map: Map>, ""]
     * @values [3, INSTANCE_EVENT_ON_UPDATE, "MAP", <event: number, instance_data: string, map: Map, diff: number>, ""]
     * @values [4, INSTANCE_EVENT_ON_PLAYER_ENTER, "MAP", <event: number, instance_data: string, map: Map, player: Player>, ""]
     * @values [5, INSTANCE_EVENT_ON_CREATURE_CREATE, "MAP", <event: number, instance_data: string, map: Map, creature: Creature>, ""]
     * @values [6, INSTANCE_EVENT_ON_GAMEOBJECT_CREATE, "MAP", <event: number, instance_data: string, map: Map, go: GameObject>, ""]
     * @values [7, INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS, "MAP", <event: number, instance_data: string, map: Map>, ""]
     *
     * @param uint32 map_id : ID of a [Map]
     * @param uint32 event : [Map] event ID, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterMapEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_MAP);
    }

    /**
     * Registers a [Map] event handler for one instance of a [Map].
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, INSTANCE_EVENT_ON_INITIALIZE, "MAP", <event: number, instance_data: string, map: Map>, ""]
     * @values [2, INSTANCE_EVENT_ON_LOAD, "MAP", <event: number, instance_data: string, map: Map>, ""]
     * @values [3, INSTANCE_EVENT_ON_UPDATE, "MAP", <event: number, instance_data: string, map: Map, diff: number>, ""]
     * @values [4, INSTANCE_EVENT_ON_PLAYER_ENTER, "MAP", <event: number, instance_data: string, map: Map, player: Player>, ""]
     * @values [5, INSTANCE_EVENT_ON_CREATURE_CREATE, "MAP", <event: number, instance_data: string, map: Map, creature: Creature>, ""]
     * @values [6, INSTANCE_EVENT_ON_GAMEOBJECT_CREATE, "MAP", <event: number, instance_data: string, map: Map, go: GameObject>, ""]
     * @values [7, INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS, "MAP", <event: number, instance_data: string, map: Map>, ""]
     *
     * @param uint32 instance_id : ID of an instance of a [Map]
     * @param uint32 event : [Map] event ID, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterInstanceEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_INSTANCE);
    }

    /**
     * Registers a [Player] gossip event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * Note that you can not use `GOSSIP_EVENT_ON_HELLO` with this hook. It does nothing since players dont have an "on hello".
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GOSSIP_EVENT_ON_HELLO, "MAP", <event: number, player: Player, object: Creature|GameObject|Item>, "Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting."]
     * @values [2, GOSSIP_EVENT_ON_SELECT, "MAP", <event: number, player: Player, object: Creature|GameObject|Item|Player, sender: number, intid: number, code: string, menu_id: number>, "Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action."]
     *
     * @proto cancel = (menu_id, event, function)
     * @proto cancel = (menu_id, event, function, shots)
     *
     * @param uint32 menu_id : [Player] gossip menu Id
     * @param uint32 event : [Player] gossip event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterPlayerGossipEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_PLAYER_GOSSIP);
    }

    /**
     * Registers a [Creature] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, CREATURE_EVENT_ON_ENTER_COMBAT, "MAP", <event: number, creature: Creature, target: Unit>, "Can return true to stop normal action"]
     * @values [2, CREATURE_EVENT_ON_LEAVE_COMBAT, "MAP", <event: number, creature: Creature>, "Can return true to stop normal action"]
     * @values [3, CREATURE_EVENT_ON_TARGET_DIED, "MAP", <event: number, creature: Creature, victim: Unit>, "Can return true to stop normal action"]
     * @values [4, CREATURE_EVENT_ON_DIED, "MAP", <event: number, creature: Creature, killer: Unit>, "Can return true to stop normal action"]
     * @values [5, CREATURE_EVENT_ON_SPAWN, "MAP", <event: number, creature: Creature>, "Can return true to stop normal action"]
     * @values [6, CREATURE_EVENT_ON_REACH_WP, "MAP", <event: number, creature: Creature, type: number, id: number>, "Can return true to stop normal action"]
     * @values [7, CREATURE_EVENT_ON_AIUPDATE, "MAP", <event: number, creature: Creature, diff: number>, "Can return true to stop normal action"]
     * @values [8, CREATURE_EVENT_ON_RECEIVE_EMOTE, "MAP", <event: number, creature: Creature, player: Player, emoteid: number>, "Can return true to stop normal action"]
     * @values [9, CREATURE_EVENT_ON_DAMAGE_TAKEN, "MAP", <event: number, creature: Creature, attacker: Unit, damage: number>, "Can return true to stop normal action, can return new damage as second return value."]
     * @values [10, CREATURE_EVENT_ON_PRE_COMBAT, "MAP", <event: number, creature: Creature, target: Unit>, "Can return true to stop normal action"]
     * @values [12, CREATURE_EVENT_ON_OWNER_ATTACKED, "MAP", <event: number, creature: Creature, target: Unit>, "Can return true to stop normal action. Not on mangos"]
     * @values [13, CREATURE_EVENT_ON_OWNER_ATTACKED_AT, "MAP", <event: number, creature: Creature, attacker: Unit>, "Can return true to stop normal action. Not on mangos"]
     * @values [14, CREATURE_EVENT_ON_HIT_BY_SPELL, "MAP", <event: number, creature: Creature, caster: Unit, spellid: number>, "Can return true to stop normal action"]
     * @values [15, CREATURE_EVENT_ON_SPELL_HIT_TARGET, "MAP", <event: number, creature: Creature, target: Unit, spellid: number>, "Can return true to stop normal action"]
     * @values [19, CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE, "MAP", <event: number, creature: Creature, summon: Creature>, "Can return true to stop normal action"]
     * @values [20, CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN, "MAP", <event: number, creature: Creature, summon: Creature>, "Can return true to stop normal action"]
     * @values [21, CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED, "MAP", <event: number, creature: Creature, summon: Creature, killer: Unit>, "Can return true to stop normal action. Not on mangos"]
     * @values [22, CREATURE_EVENT_ON_SUMMONED, "MAP", <event: number, creature: Creature, summoner: Unit>, "Can return true to stop normal action"]
     * @values [23, CREATURE_EVENT_ON_RESET, "MAP", <event: number, creature: Creature>, ""]
     * @values [24, CREATURE_EVENT_ON_REACH_HOME, "MAP", <event: number, creature: Creature>, "Can return true to stop normal action"]
     * @values [26, CREATURE_EVENT_ON_CORPSE_REMOVED, "MAP", <event: number, creature: Creature, respawndelay: number>, "Can return true to stop normal action, can return new respawndelay as second return value"]
     * @values [27, CREATURE_EVENT_ON_MOVE_IN_LOS, "MAP", <event: number, creature: Creature, unit: Unit>, "Can return true to stop normal action. Does not actually check LOS, just uses the sight range"]
     * @values [30, CREATURE_EVENT_ON_DUMMY_EFFECT, "MAP", <event: number, caster: WorldObject, spellid: number, effindex: number, creature: Creature>, ""]
     * @values [31, CREATURE_EVENT_ON_QUEST_ACCEPT, "MAP", <event: number, player: Player, creature: Creature, quest: Quest>, "Can return true"]
     * @values [34, CREATURE_EVENT_ON_QUEST_REWARD, "MAP", <event: number, player: Player, creature: Creature, quest: Quest, opt: number>, "Can return true"]
     * @values [35, CREATURE_EVENT_ON_DIALOG_STATUS, "MAP", <event: number, player: Player, creature: Creature>, ""]
     * @values [36, CREATURE_EVENT_ON_ADD, "MAP", <event: number, creature: Creature>, ""]
     * @values [37, CREATURE_EVENT_ON_REMOVE, "MAP", <event: number, creature: Creature>, ""]
     * @values [38, CREATURE_EVENT_ON_AURA_APPLY, "MAP", <event: number, creature: Creature, aura: Aura>, ""]
     * @values [39, CREATURE_EVENT_ON_HEAL, "MAP", <event: number, creature: Creature, target: Unit, heal: number>, "Can return new heal amount"]
     * @values [40, CREATURE_EVENT_ON_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [41, CREATURE_EVENT_ON_AURA_REMOVE, "MAP", <event: number, creature: Creature, aura: Aura, remove_mode: number>, ""]
     * @values [42, CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK, "MAP", <event: number, creature: Creature, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [43, CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [44, CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN, "MAP", <event: number, creature: Creature, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [45, CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED, "MAP", <event: number, creature: Creature, target: Unit, heal: number, spellInfo: SpellInfo>, "Can return new heal amount"]
     * @values [46, CREATURE_EVENT_ON_DEAL_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number, damagetype: number>, "Can return new damage amount"]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : the ID of one or more [Creature]s
     * @param uint32 event : refer to table above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterCreatureEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_CREATURE);
    }

    /**
     * Registers a [Creature] event handler for a *single* [Creature].
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, CREATURE_EVENT_ON_ENTER_COMBAT, "MAP", <event: number, creature: Creature, target: Unit>, "Can return true to stop normal action"]
     * @values [2, CREATURE_EVENT_ON_LEAVE_COMBAT, "MAP", <event: number, creature: Creature>, "Can return true to stop normal action"]
     * @values [3, CREATURE_EVENT_ON_TARGET_DIED, "MAP", <event: number, creature: Creature, victim: Unit>, "Can return true to stop normal action"]
     * @values [4, CREATURE_EVENT_ON_DIED, "MAP", <event: number, creature: Creature, killer: Unit>, "Can return true to stop normal action"]
     * @values [5, CREATURE_EVENT_ON_SPAWN, "MAP", <event: number, creature: Creature>, "Can return true to stop normal action"]
     * @values [6, CREATURE_EVENT_ON_REACH_WP, "MAP", <event: number, creature: Creature, type: number, id: number>, "Can return true to stop normal action"]
     * @values [7, CREATURE_EVENT_ON_AIUPDATE, "MAP", <event: number, creature: Creature, diff: number>, "Can return true to stop normal action"]
     * @values [8, CREATURE_EVENT_ON_RECEIVE_EMOTE, "MAP", <event: number, creature: Creature, player: Player, emoteid: number>, "Can return true to stop normal action"]
     * @values [9, CREATURE_EVENT_ON_DAMAGE_TAKEN, "MAP", <event: number, creature: Creature, attacker: Unit, damage: number>, "Can return true to stop normal action, can return new damage as second return value."]
     * @values [10, CREATURE_EVENT_ON_PRE_COMBAT, "MAP", <event: number, creature: Creature, target: Unit>, "Can return true to stop normal action"]
     * @values [12, CREATURE_EVENT_ON_OWNER_ATTACKED, "MAP", <event: number, creature: Creature, target: Unit>, "Can return true to stop normal action. Not on mangos"]
     * @values [13, CREATURE_EVENT_ON_OWNER_ATTACKED_AT, "MAP", <event: number, creature: Creature, attacker: Unit>, "Can return true to stop normal action. Not on mangos"]
     * @values [14, CREATURE_EVENT_ON_HIT_BY_SPELL, "MAP", <event: number, creature: Creature, caster: Unit, spellid: number>, "Can return true to stop normal action"]
     * @values [15, CREATURE_EVENT_ON_SPELL_HIT_TARGET, "MAP", <event: number, creature: Creature, target: Unit, spellid: number>, "Can return true to stop normal action"]
     * @values [19, CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE, "MAP", <event: number, creature: Creature, summon: Creature>, "Can return true to stop normal action"]
     * @values [20, CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN, "MAP", <event: number, creature: Creature, summon: Creature>, "Can return true to stop normal action"]
     * @values [21, CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED, "MAP", <event: number, creature: Creature, summon: Creature, killer: Unit>, "Can return true to stop normal action. Not on mangos"]
     * @values [22, CREATURE_EVENT_ON_SUMMONED, "MAP", <event: number, creature: Creature, summoner: Unit>, "Can return true to stop normal action"]
     * @values [23, CREATURE_EVENT_ON_RESET, "MAP", <event: number, creature: Creature>, ""]
     * @values [24, CREATURE_EVENT_ON_REACH_HOME, "MAP", <event: number, creature: Creature>, "Can return true to stop normal action"]
     * @values [26, CREATURE_EVENT_ON_CORPSE_REMOVED, "MAP", <event: number, creature: Creature, respawndelay: number>, "Can return true to stop normal action, can return new respawndelay as second return value"]
     * @values [27, CREATURE_EVENT_ON_MOVE_IN_LOS, "MAP", <event: number, creature: Creature, unit: Unit>, "Can return true to stop normal action. Does not actually check LOS, just uses the sight range"]
     * @values [30, CREATURE_EVENT_ON_DUMMY_EFFECT, "MAP", <event: number, caster: WorldObject, spellid: number, effindex: number, creature: Creature>, ""]
     * @values [31, CREATURE_EVENT_ON_QUEST_ACCEPT, "MAP", <event: number, player: Player, creature: Creature, quest: Quest>, "Can return true"]
     * @values [34, CREATURE_EVENT_ON_QUEST_REWARD, "MAP", <event: number, player: Player, creature: Creature, quest: Quest, opt: number>, "Can return true"]
     * @values [35, CREATURE_EVENT_ON_DIALOG_STATUS, "MAP", <event: number, player: Player, creature: Creature>, ""]
     * @values [36, CREATURE_EVENT_ON_ADD, "MAP", <event: number, creature: Creature>, ""]
     * @values [37, CREATURE_EVENT_ON_REMOVE, "MAP", <event: number, creature: Creature>, ""]
     * @values [38, CREATURE_EVENT_ON_AURA_APPLY, "MAP", <event: number, creature: Creature, aura: Aura>, ""]
     * @values [39, CREATURE_EVENT_ON_HEAL, "MAP", <event: number, creature: Creature, target: Unit, gain: number>, "Can return new heal amount"]
     * @values [40, CREATURE_EVENT_ON_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [41, CREATURE_EVENT_ON_AURA_REMOVE, "MAP", <event: number, creature: Creature, aura: Aura, remove_mode: number>, ""]
     * @values [42, CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK, "MAP", <event: number, creature: Creature, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [43, CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [44, CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN, "MAP", <event: number, creature: Creature, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [45, CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED, "MAP", <event: number, creature: Creature, target: Unit, heal: number, spellInfo: SpellInfo>, "Can return new heal amount"]
     * @values [46, CREATURE_EVENT_ON_DEAL_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number, damagetype: number>, "Can return new damage amount"]
     *
     * @proto cancel = (guid, instance_id, event, function)
     * @proto cancel = (guid, instance_id, event, function, shots)
     *
     * @param ObjectGuid guid : the GUID of a single [Creature]
     * @param uint32 instance_id : the instance ID of a single [Creature]
     * @param uint32 event : refer to table above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterUniqueCreatureEvent(lua_State* L)
    {
        return RegisterUniqueHelper(L, Hooks::REGTYPE_CREATURE);
    }

    /**
     * Registers a [GameObject] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, GAMEOBJECT_EVENT_ON_AIUPDATE, "MAP", <event: number, go: GameObject, diff: number>, ""]
     * @values [2, GAMEOBJECT_EVENT_ON_SPAWN, "MAP", <event: number, go: GameObject>, ""]
     * @values [3, GAMEOBJECT_EVENT_ON_DUMMY_EFFECT, "MAP", <event: number, caster: WorldObject, spellid: number, effindex: number, go: GameObject>, "Can return true to stop normal action"]
     * @values [4, GAMEOBJECT_EVENT_ON_QUEST_ACCEPT, "MAP", <event: number, player: Player, go: GameObject, quest: Quest>, "Can return true to stop normal action"]
     * @values [5, GAMEOBJECT_EVENT_ON_QUEST_REWARD, "MAP", <event: number, player: Player, go: GameObject, quest: Quest, opt: number>, "Can return true to stop normal action"]
     * @values [6, GAMEOBJECT_EVENT_ON_DIALOG_STATUS, "MAP", <event: number, player: Player, go: GameObject>, ""]
     * @values [7, GAMEOBJECT_EVENT_ON_DESTROYED, "MAP", <event: number, go: GameObject, attacker: Unit>, ""]
     * @values [8, GAMEOBJECT_EVENT_ON_DAMAGED, "MAP", <event: number, go: GameObject, attacker: Unit>, ""]
     * @values [9, GAMEOBJECT_EVENT_ON_LOOT_STATE_CHANGE, "MAP", <event: number, go: GameObject, state: number>, ""]
     * @values [10, GAMEOBJECT_EVENT_ON_GO_STATE_CHANGED, "MAP", <event: number, go: GameObject, state: number>, ""]
     * @values [12, GAMEOBJECT_EVENT_ON_ADD, "MAP", <event: number, go: GameObject>, ""]
     * @values [13, GAMEOBJECT_EVENT_ON_REMOVE, "MAP", <event: number, go: GameObject>, ""]
     * @values [14, GAMEOBJECT_EVENT_ON_USE, "MAP", <event: number, go: GameObject, player: Player>, "Can return true to stop normal action"]
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [GameObject] entry Id
     * @param uint32 event : [GameObject] event Id, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    int RegisterGameObjectEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_GAMEOBJECT);
    }

    /**
     * Registers a [Ticket] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, TICKET_EVENT_ON_CREATE, "WORLD", <event: number, player: Player, ticket: Ticket>, ""]
     * @values [2, TICKET_EVENT_ON_UPDATE, "WORLD", <event: number, player: Player, ticket: Ticket, message: string>, ""]
     * @values [3, TICKET_EVENT_ON_CLOSE, "WORLD", <event: number, player: Player, ticket: Ticket>, ""]
     * @values [4, TICKET_EVENT_STATUS_UPDATE, "WORLD", <event: number, player: Player, ticket: Ticket>, ""]
     * @values [5, TICKET_EVENT_ON_RESOLVE, "WORLD", <event: number, player: Player, ticket: Ticket>, ""]
     *
     * @param uint32 event : event ID, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterTicketEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_TICKET);
    }

    /**
     * Registers a [Spell] event handler.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, SPELL_EVENT_ON_PREPARE, "MAP", <event: number, caster: WorldObject, spell: Spell>, ""]
     * @values [2, SPELL_EVENT_ON_CAST, "MAP", <event: number, caster: WorldObject, spell: Spell, skipCheck: boolean>, ""]
     * @values [3, SPELL_EVENT_ON_CAST_CANCEL, "MAP", <event: number, caster: WorldObject, spell: Spell, bySelf: boolean>, ""]
     *
     * @param uint32 entry : [Spell] entry Id
     * @param uint32 event : event ID, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterSpellEvent(lua_State* L)
    {
        return RegisterEntryHelper(L, Hooks::REGTYPE_SPELL);
    }

    /**
     * Registers a [Creature] event handler. It used AllCreatureScript so this don't need creature entry as a key.
     *
     * In Multistate mode (default), events are either registered to the WORLD state (-1) or the MAP states (map ID). These events will only ever trigger on their respective state.
     *
     * In Compatibility mode, all events are registered to the WORLD state (-1).
     *
     * @table
     * @columns [ID, Event, State, Parameters, Comment]
     * @values [1, ALL_CREATURE_EVENT_ON_ADD, "MAP", <event: number, creature: Creature>, ""]
     * @values [2, ALL_CREATURE_EVENT_ON_REMOVE, "MAP", <event: number, creature: Creature>, ""]
     * @values [3, ALL_CREATURE_EVENT_ON_SELECT_LEVEL, "MAP", <event: number, creature_template: CreatureTemplate, creature: Creature>, ""]
     * @values [4, ALL_CREATURE_EVENT_ON_BEFORE_SELECT_LEVEL, "MAP", <event: number, creature_template: CreatureTemplate, creature: Creature, level: number>, "Can return the new level"]
     * @values [5, ALL_CREATURE_EVENT_ON_AURA_APPLY, "MAP", <event: number, creature: Creature, aura: Aura>, ""]
     * @values [6, ALL_CREATURE_EVENT_ON_HEAL, "MAP", <event: number, creature: Creature, target: Unit, gain: number>, "Can return new heal amount"]
     * @values [7, ALL_CREATURE_EVENT_ON_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [8, ALL_CREATURE_EVENT_ON_AURA_REMOVE, "MAP", <event: number, creature: Creature, aura: Aura, remove_mode: number>, ""]
     * @values [9, ALL_CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK, "MAP", <event: number, creature: Creature, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [10, ALL_CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number>, "Can return new damage amount"]
     * @values [11, ALL_CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN, "MAP", <event: number, creature: Creature, target: Unit, damage: number, spellInfo: SpellInfo>, "Can return new damage amount"]
     * @values [12, ALL_CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED, "MAP", <event: number, creature: Creature, target: Unit, heal: number, spellInfo: SpellInfo>, "Can return new heal amount"]
     * @values [13, ALL_CREATURE_EVENT_ON_DEAL_DAMAGE, "MAP", <event: number, creature: Creature, target: Unit, damage: number, damagetype: number>, "Can return new damage amount"]
     *
     * @param uint32 event : event ID, refer to table above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    int RegisterAllCreatureEvent(lua_State* L)
    {
        return RegisterEventHelper(L, Hooks::REGTYPE_ALL_CREATURE);
    }

    /**
     * Reloads the Lua engine.
     */
    int ReloadALE(lua_State* /*L*/)
    {
        ALE::ReloadALE();
        return 0;
    }

    /**
     * Runs a command.
     *
     * @param string command : the command to run
     */
    int RunCommand(lua_State* L)
    {
        const char* command = ALE::CHECKVAL<const char*>(L, 1);

        eWorld->QueueCliCommand(new CliCommandHolder(nullptr, command, [](void*, std::string_view view)
        {
            std::string str = { view.begin(), view.end() };
            str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end()); // Remove trailing spaces and line breaks
            ALE_LOG_INFO("{}", str);
        }, nullptr));

        return 0;
    }

    /**
     * Sends a message to all [Player]s online.
     *
     * @param string message : message to send
     */
    int SendWorldMessage(lua_State* L)
    {
        const char* message = ALE::CHECKVAL<const char*>(L, 1);
        eWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, message);
        return 0;
    }

    template <typename T>
    static int DBQueryAsync(lua_State* L, DatabaseWorkerPool<T>& db)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (funcRef == LUA_REFNIL || funcRef == LUA_NOREF)
        {
            luaL_argerror(L, 2, "unable to make a ref to function");
            return 0;
        }

        ALE* sALE = ALE::GetALE(L);

        sALE->queryProcessor.AddCallback(db.AsyncQuery(query).WithCallback([L, funcRef, sALE](QueryResult result)
            {
                ALEQuery* eq = result ? new ALEQuery(result) : nullptr;

                LOCK_ALE;

                // Get function
                lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);

                // Push parameters
                ALE::Push(L, eq);

                // Call function
                sALE->ExecuteCall(1, 0);

                luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
            }));

        return 0;
    }

    /**
     * Executes a SQL query on the world database and returns an [ALEQuery].
     *
     * The query is always executed synchronously
     *   (i.e. execution halts until the query has finished and then results are returned).
     * If you need to execute the query asynchronously, use [Global:WorldDBQueryAsync] instead.
     *
     *     local Q = WorldDBQuery("SELECT entry, name FROM creature_template LIMIT 10")
     *     if Q then
     *         repeat
     *             local entry, name = Q:GetUInt32(0), Q:GetString(1)
     *             print(entry, name)
     *         until not Q:NextRow()
     *     end
     *
     * @param string sql : query to execute
     * @return [ALEQuery] results or nil if no rows found or nil if no rows found
     */
    int WorldDBQuery(lua_State* L)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);

        int numArgs = lua_gettop(L);
        if (numArgs > 1)
            query = ALE::FormatQuery(L, query).c_str();

        ALEQuery result = WorldDatabase.Query(query);
        if (result)
            ALE::Push(L, new ALEQuery(result));
        else
            ALE::Push(L);
        return 1;
    }

    /**
     * Executes an asynchronous SQL query on the world database and passes an [ALEQuery] to a callback function.
     *
     * The query is executed asynchronously
     *   (i.e. the server keeps running while the query is executed in parallel, and results are passed to a callback function).
     * If you need to execute the query synchronously, use [Global:WorldDBQuery] instead.
     *
     *     WorldDBQueryAsync("SELECT entry, name FROM creature_template LIMIT 10", function(Q)
     *         if Q then
     *             repeat
     *                 local entry, name = Q:GetUInt32(0), Q:GetString(1)
     *                 print(entry, name)
     *             until not Q:NextRow()
     *         end
     *     end)
     *
     * @param string sql : query to execute
     * @param function callback : function that will be called when the results are available
     */
    int WorldDBQueryAsync(lua_State* L)
    {
        return DBQueryAsync(L, WorldDatabase);
    }

    /**
     * Executes a SQL query on the world database.
     *
     * The query may be executed *asynchronously* (at a later, unpredictable time).
     * If you need to execute the query synchronously, use [Global:WorldDBQuery] instead.
     *
     * Any results produced are ignored.
     * If you need results from the query, use [Global:WorldDBQuery] or [Global:WorldDBQueryAsync] instead.
     *
     *     WorldDBExecute("DELETE FROM my_table")
     *
     * @param string sql : query to execute
     */
    int WorldDBExecute(lua_State* L)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);

        int numArgs = lua_gettop(L);
        if (numArgs > 1)
            query = ALE::FormatQuery(L, query).c_str();

        WorldDatabase.Execute(query);
        return 0;
    }

    /**
     * Executes a SQL query on the character database and returns an [ALEQuery].
     *
     * The query is always executed synchronously
     *   (i.e. execution halts until the query has finished and then results are returned).
     * If you need to execute the query asynchronously, use [Global:CharDBQueryAsync] instead.
     *
     * For an example see [Global:WorldDBQuery].
     *
     * @param string sql : query to execute
     * @return [ALEQuery] results or nil if no rows found
     */
    int CharDBQuery(lua_State* L)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);

        int numArgs = lua_gettop(L);
        if (numArgs > 1)
            query = ALE::FormatQuery(L, query).c_str();

        QueryResult result = CharacterDatabase.Query(query);
        if (result)
            ALE::Push(L, new QueryResult(result));
        else
            ALE::Push(L);
        return 1;
    }

    /**
     * Executes an asynchronous SQL query on the character database and passes an [ALEQuery] to a callback function.
     *
     * The query is executed asynchronously
     *   (i.e. the server keeps running while the query is executed in parallel, and results are passed to a callback function).
     * If you need to execute the query synchronously, use [Global:CharDBQuery] instead.
     *
     * For an example see [Global:WorldDBQueryAsync].
     *
     * @param string sql : query to execute
     * @param function callback : function that will be called when the results are available
     */
    int CharDBQueryAsync(lua_State* L)
    {
        return DBQueryAsync(L, CharacterDatabase);
    }

    /**
     * Executes a SQL query on the character database.
     *
     * The query may be executed *asynchronously* (at a later, unpredictable time).
     * If you need to execute the query synchronously, use [Global:CharDBQuery] instead.
     *
     * Any results produced are ignored.
     * If you need results from the query, use [Global:CharDBQuery] or [Global:CharDBQueryAsync] instead.
     *
     *     CharDBExecute("DELETE FROM my_table")
     *
     * @param string sql : query to execute
     */
    int CharDBExecute(lua_State* L)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);

        int numArgs = lua_gettop(L);
        if (numArgs > 1)
            query = ALE::FormatQuery(L, query).c_str();

        CharacterDatabase.Execute(query);
        return 0;
    }

    /**
     * Executes a SQL query on the login database and returns an [ALEQuery].
     *
     * The query is always executed synchronously
     *   (i.e. execution halts until the query has finished and then results are returned).
     * If you need to execute the query asynchronously, use [Global:AuthDBQueryAsync] instead.
     *
     * For an example see [Global:WorldDBQuery].
     *
     * @param string sql : query to execute
     * @return [ALEQuery] results or nil if no rows found
     */
    int AuthDBQuery(lua_State* L)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);

        int numArgs = lua_gettop(L);
        if (numArgs > 1)
            query = ALE::FormatQuery(L, query).c_str();

        QueryResult result = LoginDatabase.Query(query);
        if (result)
            ALE::Push(L, new QueryResult(result));
        else
            ALE::Push(L);
        return 1;
    }

    /**
     * Executes an asynchronous SQL query on the character database and passes an [ALEQuery] to a callback function.
     *
     * The query is executed asynchronously
     *   (i.e. the server keeps running while the query is executed in parallel, and results are passed to a callback function).
     * If you need to execute the query synchronously, use [Global:AuthDBQuery] instead.
     *
     * For an example see [Global:WorldDBQueryAsync].
     *
     * @param string sql : query to execute
     * @param function callback : function that will be called when the results are available
     */
    int AuthDBQueryAsync(lua_State* L)
    {
        return DBQueryAsync(L, LoginDatabase);
    }

    /**
     * Executes a SQL query on the login database.
     *
     * The query may be executed *asynchronously* (at a later, unpredictable time).
     * If you need to execute the query synchronously, use [Global:AuthDBQuery] instead.
     *
     * Any results produced are ignored.
     * If you need results from the query, use [Global:AuthDBQuery] or [Global:AuthDBQueryAsync] instead.
     *
     *     AuthDBExecute("DELETE FROM my_table")
     *
     * @param string sql : query to execute
     */
    int AuthDBExecute(lua_State* L)
    {
        const char* query = ALE::CHECKVAL<const char*>(L, 1);

        int numArgs = lua_gettop(L);
        if (numArgs > 1)
            query = ALE::FormatQuery(L, query).c_str();
            
        LoginDatabase.Execute(query);
        return 0;
    }

    /**
     * Registers a global timed event.
     *
     * When the passed function is called, the parameters `(eventId, delay, repeats)` are passed to it.
     *
     * Repeats will decrease on each call if the event does not repeat indefinitely
     *
     * @proto eventId = (function, delay)
     * @proto eventId = (function, delaytable)
     * @proto eventId = (function, delay, repeats)
     * @proto eventId = (function, delaytable, repeats)
     *
     * @param function function : function to trigger when the time has passed
     * @param uint32 delay : set time in milliseconds for the event to trigger
     * @param table delaytable : a table `{min, max}` containing the minimum and maximum delay time
     * @param uint32 repeats = 1 : how many times for the event to repeat, 0 is infinite
     * @return int eventId : unique ID for the timed event used to cancel it or nil
     */
    int CreateLuaEvent(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        uint32 min, max;
        if (lua_istable(L, 2))
        {
            ALE::Push(L, 1);
            lua_gettable(L, 2);
            min = ALE::CHECKVAL<uint32>(L, -1);
            ALE::Push(L, 2);
            lua_gettable(L, 2);
            max = ALE::CHECKVAL<uint32>(L, -1);
            lua_pop(L, 2);
        }
        else
            min = max = ALE::CHECKVAL<uint32>(L, 2);
        uint32 repeats = ALE::CHECKVAL<uint32>(L, 3, 1);

        if (min > max)
            return luaL_argerror(L, 2, "min is bigger than max delay");

        lua_pushvalue(L, 1);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (functionRef != LUA_REFNIL && functionRef != LUA_NOREF)
        {
            ALE* callingE = ALE::GetALE(L);
            callingE->eventMgr->globalProcessor->AddEvent(functionRef, min, max, repeats, callingE->GetSelfPtr());
            ALE::Push(L, functionRef);
        }
        return 1;
    }

    /**
     * Removes a global timed event specified by ID.
     *
     * @param int eventId : event Id to remove
     * @param bool all_Events = false : remove from all events, not just global
     */
    int RemoveEventById(lua_State* L)
    {
        int eventId = ALE::CHECKVAL<int>(L, 1);
        bool all_Events = ALE::CHECKVAL<bool>(L, 1, false);

        // not thread safe
        if (all_Events)
            ALE::GetALE(L)->eventMgr->SetState(eventId, LUAEVENT_STATE_ABORT);
        else
            ALE::GetALE(L)->eventMgr->globalProcessor->SetState(eventId, LUAEVENT_STATE_ABORT);
        return 0;
    }

    /**
     * Removes all global timed events.
     *
     * @param bool all_Events = false : remove all events, not just global
     */
    int RemoveEvents(lua_State* L)
    {
        bool all_Events = ALE::CHECKVAL<bool>(L, 1, false);

        // not thread safe
        if (all_Events)
            ALE::GetALE(L)->eventMgr->SetStates(LUAEVENT_STATE_ABORT);
        else
            ALE::GetALE(L)->eventMgr->globalProcessor->SetStates(LUAEVENT_STATE_ABORT);
        return 0;
    }

    /**
     * Performs an in-game spawn and returns the [Creature] or [GameObject] spawned.
     *
     * @param int32 spawnType : type of object to spawn, 1 = [Creature], 2 = [GameObject]
     * @param uint32 entry : entry ID of the [Creature] or [GameObject]
     * @param uint32 mapId : map ID to spawn the [Creature] or [GameObject] in
     * @param uint32 instanceId : instance ID to put the [Creature] or [GameObject] in. Non instance is 0
     * @param float x : x coordinate of the [Creature] or [GameObject]
     * @param float y : y coordinate of the [Creature] or [GameObject]
     * @param float z : z coordinate of the [Creature] or [GameObject]
     * @param float o : o facing/orientation of the [Creature] or [GameObject]
     * @param bool save = false : optional to save the [Creature] or [GameObject] to the database
     * @param uint32 durorresptime = 0 : despawn time of the [Creature] if it's not saved or respawn time of [GameObject]
     * @param uint32 phase = 1 : phase to put the [Creature] or [GameObject] in
     * @return [WorldObject] worldObject : returns [Creature] or [GameObject]
     */
    int PerformIngameSpawn(lua_State* L)
    {
        int spawntype = ALE::CHECKVAL<int>(L, 1);
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        uint32 mapID = ALE::CHECKVAL<uint32>(L, 3);
        uint32 instanceID = ALE::CHECKVAL<uint32>(L, 4);

        float x = ALE::CHECKVAL<float>(L, 5);
        float y = ALE::CHECKVAL<float>(L, 6);
        float z = ALE::CHECKVAL<float>(L, 7);
        float o = ALE::CHECKVAL<float>(L, 8);
        bool save = ALE::CHECKVAL<bool>(L, 9, false);
        uint32 durorresptime = ALE::CHECKVAL<uint32>(L, 10, 0);
        uint32 phase = ALE::CHECKVAL<uint32>(L, 11, PHASEMASK_NORMAL);
        
        if (!phase)
        {
            ALE::Push(L);
            return 1;
        }

        Map* map = eMapMgr->FindMap(mapID, instanceID);
        if (!map)
        {
            ALE::Push(L);
            return 1;
        }

        Position pos = { x, y, z, o };

        if (spawntype == 1) // spawn creature
        {
            if (save)
            {
                Creature* creature = new Creature();
                if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, phase, entry, 0, x, y, z, o))
                {
                    delete creature;
                    ALE::Push(L);
                    return 1;
                }

                creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), phase);

                uint32 db_guid = creature->GetSpawnId();

                // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells()
                // current "creature" variable is deleted and created fresh new, otherwise old values might trigger asserts or cause undefined behavior
                creature->CleanupsBeforeDelete();
                delete creature;
                creature = new Creature();

                if (!creature->LoadCreatureFromDB(db_guid, map, true, true))
                {
                    delete creature;
                    ALE::Push(L);
                    return 1;
                }

                eObjectMgr->AddCreatureToGrid(db_guid, eObjectMgr->GetCreatureData(db_guid));
                ALE::Push(L, creature);
            }
            else
            {
                TempSummon* creature = map->SummonCreature(entry, pos, NULL, durorresptime);
                if (!creature)
                {
                    ALE::Push(L);
                    return 1;
                }

                if (durorresptime)
                    creature->SetTempSummonType(TEMPSUMMON_TIMED_OR_DEAD_DESPAWN);
                else
                    creature->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);

                ALE::Push(L, creature);
            }

            return 1;
        }

        if (spawntype == 2) // Spawn object
        {
            const GameObjectTemplate* objectInfo = eObjectMgr->GetGameObjectTemplate(entry);
            if (!objectInfo)
            {
                ALE::Push(L);
                return 1;
            }

            if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
            {
                ALE::Push(L);
                return 1;
            }

            GameObject* object = new GameObject;
            uint32 guidLow = map->GenerateLowGuid<HighGuid::GameObject>();

            if (!object->Create(guidLow, entry, map, phase, x, y, z, o, G3D::Quat(0.0f, 0.0f, 0.0f, 0.0f), 100, GO_STATE_READY))
            {
                delete object;
                ALE::Push(L);
                return 1;
            }

            if (durorresptime)
                object->SetRespawnTime(durorresptime);

            if (save)
            {
                // fill the gameobject data and save to the db
                object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), phase);
                guidLow = object->GetSpawnId();

                // delete the old object and do a clean load from DB with a fresh new GameObject instance.
                // this is required to avoid weird behavior and memory leaks
                delete object;

                object = new GameObject();
                // this will generate a new lowguid if the object is in an instance
                if (!object->LoadGameObjectFromDB(guidLow, map, true))
                {
                    delete object;
                    ALE::Push(L);
                    return 1;
                }

                eObjectMgr->AddGameobjectToGrid(guidLow, eObjectMgr->GetGameObjectData(guidLow));
            }
            else
                map->AddToMap(object);
            ALE::Push(L, object);
            return 1;
        }
        ALE::Push(L);
        return 1;
    }

    /**
     * Creates a [WorldPacket].
     *
     * @param [Opcodes] opcode : the opcode of the packet
     * @param uint32 size : the size of the packet
     * @return [WorldPacket] packet
     */
    int CreatePacket(lua_State* L)
    {
        uint32 opcode = ALE::CHECKVAL<uint32>(L, 1);
        size_t size = ALE::CHECKVAL<size_t>(L, 2);
        if (opcode >= NUM_MSG_TYPES)
            return luaL_argerror(L, 1, "valid opcode expected");

        ALE::Push(L, new WorldPacket((OpcodesList)opcode, size));
        return 1;
    }

    /**
     * Adds an [Item] to a vendor and updates the world database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     * @param int32 maxcount : max [Item] stack count
     * @param uint32 incrtime : combined with maxcount, incrtime tells how often (in seconds) the vendor list is refreshed and the limited [Item] copies are restocked
     * @param uint32 extendedcost : unique cost of an [Item], such as conquest points for example
     */
    int AddVendorItem(lua_State* L)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
        uint32 item = ALE::CHECKVAL<uint32>(L, 2);
        int maxcount = ALE::CHECKVAL<int>(L, 3);
        uint32 incrtime = ALE::CHECKVAL<uint32>(L, 4);
        uint32 extendedcost = ALE::CHECKVAL<uint32>(L, 5);

        if (!eObjectMgr->IsVendorItemValid(entry, item, maxcount, incrtime, extendedcost))
            return 0;
        eObjectMgr->AddVendorItem(entry, item, maxcount, incrtime, extendedcost);

        return 0;
    }

    /**
     * Removes an [Item] from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     */
    int VendorRemoveItem(lua_State* L)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
        uint32 item = ALE::CHECKVAL<uint32>(L, 2);
        if (!eObjectMgr->GetCreatureTemplate(entry))
            return luaL_argerror(L, 1, "valid CreatureEntry expected");

        eObjectMgr->RemoveVendorItem(entry, item);
        return 0;
    }

    /**
     * Removes all [Item]s from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     */
    int VendorRemoveAllItems(lua_State* L)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

        VendorItemData const* items = eObjectMgr->GetNpcVendorItemList(entry);
        if (!items || items->Empty())
            return 0;

        auto const& itemlist = items->m_items;
        for (auto itr = itemlist.rbegin(); itr != itemlist.rend(); ++itr)
            eObjectMgr->RemoveVendorItem(entry, (*itr)->item);
        return 0;
    }

    /**
     * Kicks a [Player] from the server.
     *
     * @param [Player] player : [Player] to kick
     */
    int Kick(lua_State* L)
    {
        Player* player = ALE::CHECKOBJ<Player>(L, 1);
        player->GetSession()->KickPlayer();
        return 0;
    }

    /**
     * Ban's a [Player]'s account, character or IP
     *
     *     enum BanMode
     *     {
     *         BAN_ACCOUNT = 0,
     *         BAN_CHARACTER = 1,
     *         BAN_IP = 2
     *     };
     *
     * @param [BanMode] banMode : method of ban, refer to BanMode above
     * @param string nameOrIP : If BanMode is 0 then accountname, if 1 then charactername if 2 then ip
     * @param uint32 duration : duration (in seconds) of the ban
     * @param string reason = "" : ban reason, this is optional
     * @param string whoBanned = "" : the [Player]'s name that banned the account, character or IP, this is optional
     * @return int result : status of the ban. 0 if success, 1 if syntax error, 2 if target not found, 3 if a longer ban already exists, nil if unknown result
     */
    int Ban(lua_State* L)
    {
        int banMode = ALE::CHECKVAL<int>(L, 1);
        std::string nameOrIP = ALE::CHECKVAL<std::string>(L, 2);
        uint32 duration = ALE::CHECKVAL<uint32>(L, 3);
        const char* reason = ALE::CHECKVAL<const char*>(L, 4, "");
        const char* whoBanned = ALE::CHECKVAL<const char*>(L, 5, "");

        const int BAN_ACCOUNT = 0;
        const int BAN_CHARACTER = 1;
        const int BAN_IP = 2;

        switch (banMode)
        {
            case BAN_ACCOUNT:
                if (!Utf8ToUpperOnlyLatin(nameOrIP))
                    return luaL_argerror(L, 2, "invalid account name");
                break;
            case BAN_CHARACTER:
                if (!normalizePlayerName(nameOrIP))
                    return luaL_argerror(L, 2, "invalid character name");
                break;
            case BAN_IP:
                if (!IsIPAddress(nameOrIP.c_str()))
                    return luaL_argerror(L, 2, "invalid ip");
                break;
            default:
                return luaL_argerror(L, 1, "unknown banmode");
        }

        BanReturn result;
        switch (banMode)
        {
            case BAN_ACCOUNT:
                result = sBan->BanAccount(nameOrIP, std::to_string(duration) + "s", reason, whoBanned);
            break;
            case BAN_CHARACTER:
                result = sBan->BanCharacter(nameOrIP, std::to_string(duration) + "s", reason, whoBanned);
            break;
            case BAN_IP:
                result = sBan->BanIP(nameOrIP, std::to_string(duration) + "s", reason, whoBanned);
            break;
        }

        switch (result)
        {
        case BanReturn::BAN_SUCCESS:
            ALE::Push(L, 0);
            break;
        case BanReturn::BAN_SYNTAX_ERROR:
            ALE::Push(L, 1);
            break;
        case BanReturn::BAN_NOTFOUND:
            ALE::Push(L, 2);
            break;
        case BanReturn::BAN_LONGER_EXISTS:
            ALE::Push(L, 3);
            break;
        }
        return 1;
    }

    /**
     * Saves all [Player]s.
     */
    int SaveAllPlayers(lua_State* /*L*/)
    {
        eObjectAccessor()SaveAllPlayers();
        return 0;
    }

    /**
     * Sends mail to a [Player].
     *
     * There can be several item entry-amount pairs at the end of the function.
     * There can be maximum of 12 different items.
     *
     *     enum MailStationery
     *     {
     *         MAIL_STATIONERY_TEST = 1,
     *         MAIL_STATIONERY_DEFAULT = 41,
     *         MAIL_STATIONERY_GM = 61,
     *         MAIL_STATIONERY_AUCTION = 62,
     *         MAIL_STATIONERY_VAL = 64, // Valentine
     *         MAIL_STATIONERY_CHR = 65, // Christmas
     *         MAIL_STATIONERY_ORP = 67 // Orphan
     *     };
     *
     * @param string subject : title (subject) of the mail
     * @param string text : contents of the mail
     * @param uint32 receiverGUIDLow : low GUID of the receiver
     * @param uint32 senderGUIDLow = 0 : low GUID of the sender
     * @param [MailStationery] stationary = MAIL_STATIONERY_DEFAULT : type of mail that is being sent as, refer to MailStationery above
     * @param uint32 delay = 0 : mail send delay in milliseconds
     * @param uint32 money = 0 : money to send
     * @param uint32 cod = 0 : cod money amount
     * @param uint32 entry = 0 : entry of an [Item] to send with mail
     * @param uint32 amount = 0 : amount of the [Item] to send with mail
     * @return uint32 itemGUIDlow : low GUID of the item. Up to 12 values returned, returns nil if no further items are sent
     */
    int SendMail(lua_State* L)
    {
        int i = 0;
        std::string subject = ALE::CHECKVAL<std::string>(L, ++i);
        std::string text = ALE::CHECKVAL<std::string>(L, ++i);
        uint32 receiverGUIDLow = ALE::CHECKVAL<uint32>(L, ++i);
        uint32 senderGUIDLow = ALE::CHECKVAL<uint32>(L, ++i, 0);
        uint32 stationary = ALE::CHECKVAL<uint32>(L, ++i, MAIL_STATIONERY_DEFAULT);
        uint32 delay = ALE::CHECKVAL<uint32>(L, ++i, 0);
        uint32 money = ALE::CHECKVAL<uint32>(L, ++i, 0);
        uint32 cod = ALE::CHECKVAL<uint32>(L, ++i, 0);
        int argAmount = lua_gettop(L);

        MailSender sender(MAIL_NORMAL, senderGUIDLow, (MailStationery)stationary);
        MailDraft draft(subject, text);

        if (cod)
            draft.AddCOD(cod);
        if (money)
            draft.AddMoney(money);

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        uint8 addedItems = 0;
        while (addedItems <= MAX_MAIL_ITEMS && i + 2 <= argAmount)
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, ++i);
            uint32 amount = ALE::CHECKVAL<uint32>(L, ++i);

            ItemTemplate const* item_proto = eObjectMgr->GetItemTemplate(entry);
            if (!item_proto)
            {
                luaL_error(L, "Item entry %d does not exist", entry);
                continue;
            }
            if (amount < 1 || (item_proto->MaxCount > 0 && amount > uint32(item_proto->MaxCount)))
            {
                luaL_error(L, "Item entry %d has invalid amount %d", entry, amount);
                continue;
            }
            if (Item* item = Item::CreateItem(entry, amount))
            {
                item->SaveToDB(trans);
                draft.AddItem(item);
                ALE::Push(L, item->GetGUID().GetCounter());
                ++addedItems;
            }
        }

        Player* receiverPlayer = eObjectAccessor()FindPlayer(MAKE_NEW_GUID(receiverGUIDLow, 0, HIGHGUID_PLAYER));
        draft.SendMailTo(trans, MailReceiver(receiverPlayer, receiverGUIDLow), sender, MAIL_CHECK_MASK_NONE, delay);
        CharacterDatabase.CommitTransaction(trans);
        return addedItems;
    }

    /**
     * Performs a bitwise AND (a & b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_and(lua_State* L)
    {
        uint32 a = ALE::CHECKVAL<uint32>(L, 1);
        uint32 b = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, a & b);
        return 1;
    }

    /**
     * Performs a bitwise OR (a | b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_or(lua_State* L)
    {
        uint32 a = ALE::CHECKVAL<uint32>(L, 1);
        uint32 b = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, a | b);
        return 1;
    }

    /**
     * Performs a bitwise left-shift (a << b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_lshift(lua_State* L)
    {
        uint32 a = ALE::CHECKVAL<uint32>(L, 1);
        uint32 b = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, a << b);
        return 1;
    }

    /**
     * Performs a bitwise right-shift (a >> b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_rshift(lua_State* L)
    {
        uint32 a = ALE::CHECKVAL<uint32>(L, 1);
        uint32 b = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, a >> b);
        return 1;
    }

    /**
     * Performs a bitwise XOR (a ^ b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    int bit_xor(lua_State* L)
    {
        uint32 a = ALE::CHECKVAL<uint32>(L, 1);
        uint32 b = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, a ^ b);
        return 1;
    }

    /**
     * Performs a bitwise NOT (~a).
     *
     * @param uint32 a
     * @return uint32 result
     */
    int bit_not(lua_State* L)
    {
        uint32 a = ALE::CHECKVAL<uint32>(L, 1);
        ALE::Push(L, ~a);
        return 1;
    }

    /**
     * Adds a taxi path to a specified map, returns the used pathId.
     *
     * Note that the first taxi point needs to be near the player when he starts the taxi path.
     * The function should also be used only **once** per path added so use it on server startup for example.
     *
     * Related function: [Player:StartTaxi]
     *
     *     -- Execute on startup
     *     local pathTable = {{mapid, x, y, z}, {mapid, x, y, z}}
     *     local path = AddTaxiPath(pathTable, 28135, 28135)
     *
     *     -- Execute when the player should fly
     *     player:StartTaxi(path)
     *
     * @param table waypoints : table containing waypoints: {map, x, y, z[, actionFlag, delay]}
     * @param uint32 mountA : alliance [Creature] entry
     * @param uint32 mountH : horde [Creature] entry
     * @param uint32 price = 0 : price of the taxi path
     * @param uint32 pathId = 0 : path Id of the taxi path
     * @return uint32 actualPathId
     */
    int AddTaxiPath(lua_State* L)
    {
        luaL_checktype(L, 1, LUA_TTABLE);
        uint32 mountA = ALE::CHECKVAL<uint32>(L, 2);
        uint32 mountH = ALE::CHECKVAL<uint32>(L, 3);
        uint32 price = ALE::CHECKVAL<uint32>(L, 4, 0);
        uint32 pathId = ALE::CHECKVAL<uint32>(L, 5, 0);
        lua_pushvalue(L, 1);
        // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}

        std::list<TaxiPathNodeEntry> nodes;

        int start = lua_gettop(L);
        int end = start;

        ALE::Push(L);
        // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, nil
        while (lua_next(L, -2) != 0)
        {
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value
            luaL_checktype(L, -1, LUA_TTABLE);
            ALE::Push(L);
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value, nil
            while (lua_next(L, -2) != 0)
            {
                // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value, key2, value2
                lua_insert(L, end++);
                // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, value2, key, value, key2
            }
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, value2, key, value
            if (start == end)
                continue;
            if (end - start < 4) // no mandatory args, dont add
                return luaL_argerror(L, 1, "all waypoints do not have mandatory arguments");

            while (end - start < 8) // fill optional args with 0
            {
                ALE::Push(L, 0);
                lua_insert(L, end++);
                // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, node, key, value
            }
            TaxiPathNodeEntry entry;
            // mandatory
            entry.mapid = ALE::CHECKVAL<uint32>(L, start);
            entry.x = ALE::CHECKVAL<float>(L, start + 1);
            entry.y = ALE::CHECKVAL<float>(L, start + 2);
            entry.z = ALE::CHECKVAL<float>(L, start + 3);
            // optional
            entry.actionFlag = ALE::CHECKVAL<uint32>(L, start + 4, 0);
            entry.delay = ALE::CHECKVAL<uint32>(L, start + 5, 0);

            nodes.push_back(entry);

            while (end != start) // remove args
                if (!lua_isnone(L, --end))
                    lua_remove(L, end);
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key, value

            lua_pop(L, 1);
            // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}, key
        }
        // Stack: {nodes}, mountA, mountH, price, pathid, {nodes}
        lua_pop(L, 1);
        // Stack: {nodes}, mountA, mountH, price, pathid

        if (nodes.size() < 2)
            return 1;
        if (!pathId)
            pathId = sTaxiPathNodesByPath.size();
        if (sTaxiPathNodesByPath.size() <= pathId)
            sTaxiPathNodesByPath.resize(pathId + 1);

        sTaxiPathNodesByPath[pathId].clear();
        sTaxiPathNodesByPath[pathId].resize(nodes.size());
        static uint32 nodeId = 500;
        uint32 startNode = nodeId;
        uint32 index = 0;

        for (std::list<TaxiPathNodeEntry>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            TaxiPathNodeEntry& entry = *it;
            TaxiNodesEntry* nodeEntry = new TaxiNodesEntry();
            entry.path = pathId;
            entry.index = nodeId;
            nodeEntry->ID = index;
            nodeEntry->map_id = entry.mapid;
            nodeEntry->x = entry.x;
            nodeEntry->y = entry.y;
            nodeEntry->z = entry.z;
            nodeEntry->MountCreatureID[0] = mountH;
            nodeEntry->MountCreatureID[1] = mountA;
            sTaxiNodesStore.SetEntry(nodeId++, nodeEntry);
            sTaxiPathNodesByPath[pathId][index++] = new TaxiPathNodeEntry(entry);
        }
        if (startNode >= nodeId)
            return 1;

        TaxiPathEntry* pathEntry = new TaxiPathEntry();
        pathEntry->from = startNode;
        pathEntry->to = nodeId - 1;
        pathEntry->price = price;
        pathEntry->ID = pathId;
        sTaxiPathStore.SetEntry(pathId, pathEntry);
        sTaxiPathSetBySource[startNode][nodeId - 1] = pathEntry;

        ALE::Push(L, pathId);
        return 1;
    }

    /**
     * Returns `true` if ALE is in compatibility mode, `false` if in multistate.
     *
     * @return bool isCompatibilityMode
     */
    int IsCompatibilityMode(lua_State* L)
    {
        ALE::Push(L, ALEConfig::GetInstance().IsCompatibilityModeEnabled());
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid inventory position, otherwise `false`.
     *
     * Some commonly used combinations:
     *
     * *Bag 255 (common character inventory)*
     *
     * - Slots 0-18: equipment
     * - Slots 19-22: bag slots
     * - Slots 23-38: backpack
     * - Slots 39-66: bank main slots
     * - Slots 67-74: bank bag slots
     * - Slots 86-117: keyring
     *
     * *Bags 19-22 (equipped bags)*
     *
     * - Slots 0-35
     *
     * *Bags 67-74 (bank bags)*
     *
     * - Slots 0-35
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isInventoryPos
     */
    int IsInventoryPos(lua_State* L)
    {
        uint8 bag = ALE::CHECKVAL<uint8>(L, 1);
        uint8 slot = ALE::CHECKVAL<uint8>(L, 2);

        ALE::Push(L, Player::IsInventoryPos(bag, slot));
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid equipment position, otherwise `false`.
     *
     * See [Global:IsInventoryPos] for bag/slot combination examples.
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isEquipmentPosition
     */
    int IsEquipmentPos(lua_State* L)
    {
        uint8 bag = ALE::CHECKVAL<uint8>(L, 1);
        uint8 slot = ALE::CHECKVAL<uint8>(L, 2);

        ALE::Push(L, Player::IsEquipmentPos(bag, slot));
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid bank position, otherwise `false`.
     *
     * See [Global:IsInventoryPos] for bag/slot combination examples.
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isBankPosition
     */
    int IsBankPos(lua_State* L)
    {
        uint8 bag = ALE::CHECKVAL<uint8>(L, 1);
        uint8 slot = ALE::CHECKVAL<uint8>(L, 2);

        ALE::Push(L, Player::IsBankPos(bag, slot));
        return 1;
    }

    /**
     * Returns `true` if the bag and slot is a valid bag position, otherwise `false`.
     *
     * See [Global:IsInventoryPos] for bag/slot combination examples.
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return bool isBagPosition
     */
    int IsBagPos(lua_State* L)
    {
        uint8 bag = ALE::CHECKVAL<uint8>(L, 1);
        uint8 slot = ALE::CHECKVAL<uint8>(L, 2);

        ALE::Push(L, Player::IsBagPos((bag << 8) + slot));
        return 1;
    }

    /**
     * Returns `true` if the event is currently active, otherwise `false`.
     *
     * @param uint16 eventId : the event id to check.
     * @return bool isActive
     */
    int IsGameEventActive(lua_State* L)
    {
        uint16 eventId = ALE::CHECKVAL<uint16>(L, 1);

        ALE::Push(L, eGameEventMgr->IsActiveEvent(eventId));
        return 1;
    }

    /**
     * Returns the server's current time.
     *
     * @return uint32 currTime : the current time, in milliseconds
     */
    int GetCurrTime(lua_State* L)
    {
        ALE::Push(L, ALEUtil::GetCurrTime());
        return 1;
    }

    /**
     * Returns the difference between an old timestamp and the current time.
     *
     * @param uint32 oldTime : an old timestamp, in milliseconds
     * @return uint32 timeDiff : the difference, in milliseconds
     */
    int GetTimeDiff(lua_State* L)
    {
        uint32 oldtimems = ALE::CHECKVAL<uint32>(L, 1);

        ALE::Push(L, ALEUtil::GetTimeDiff(oldtimems));
        return 1;
    }

    static std::string GetStackAsString(lua_State* L)
    {
        std::ostringstream oss;
        int top = lua_gettop(L);
        for (int i = 1; i <= top; ++i)
        {
            oss << luaL_tolstring(L, i, NULL);
            lua_pop(L, 1);
        }
        return oss.str();
    }

    /**
     * Prints given parameters to the info log.
     *
     * @param ...
     */
    int PrintInfo(lua_State* L)
    {
        ALE_LOG_INFO("{}", GetStackAsString(L));
        return 0;
    }

    /**
     * Prints given parameters to the error log.
     *
     * @param ...
     */
    int PrintError(lua_State* L)
    {
        ALE_LOG_ERROR("{}", GetStackAsString(L));
        return 0;
    }

    /**
     * Prints given parameters to the debug log.
     *
     * @param ...
     */
    int PrintDebug(lua_State* L)
    {
        ALE_LOG_DEBUG("{}", GetStackAsString(L));
        return 0;
    }

    /**
    * Starts the event by eventId, if force is set, the event will force start regardless of previous event state.
    *
    * @param uint16 eventId : the event id to start.
    * @param bool force = false : set `true` to force start the event.
    */
    int StartGameEvent(lua_State* L)
    {
        uint16 eventId = ALE::CHECKVAL<uint16>(L, 1);
        bool force = ALE::CHECKVAL<bool>(L, 2, false);

        eGameEventMgr->StartEvent(eventId, force);
        return 0;
    }

    /**
    * Stops the event by eventId, if force is set, the event will force stop regardless of previous event state.
    *
    * @param uint16 eventId : the event id to stop.
    * @param bool force = false : set `true` to force stop the event.
    */
    int StopGameEvent(lua_State* L)
    {
        uint16 eventId = ALE::CHECKVAL<uint16>(L, 1);
        bool force = ALE::CHECKVAL<bool>(L, 2, false);

        eGameEventMgr->StopEvent(eventId, force);
        return 0;
    }

    /**
     * Performs a non-blocking HTTP request.
     *
     * When the passed callback function is called, the parameters `(status, body, headers)` are passed to it.
     *
     *     -- GET example (prints a random word)
     *     HttpRequest("GET", "https://random-word-api.herokuapp.com/word", function(status, body, headers)
     *         print("Random word: " .. string.sub(body, 3, body:len() - 2))
     *     end)
     *
     *     -- POST example with JSON request body
     *     HttpRequest("POST", "https://jsonplaceholder.typicode.com/posts", '{"userId": 1,"title": "Foo","body": "Bar!"}', "application/json", function(status, body, headers)
     *         print(body)
     *     end)
     *
     *     -- Example with request headers
     *     HttpRequest("GET", "https://postman-echo.com/headers", { Accept = "application/json", ["User-Agent"] = "ALE Lua Engine" }, function(status, body, headers)
     *         print(body)
     *     end)
     *
     * @proto (httpMethod, url, function)
     * @proto (httpMethod, url, headers, function)
     * @proto (httpMethod, url, body, contentType, function)
     * @proto (httpMethod, url, body, contentType, headers, function)
     *
     * @param string httpMethod : the HTTP method to use (possible values are: `"GET"`, `"HEAD"`, `"POST"`, `"PUT"`, `"PATCH"`, `"DELETE"`, `"OPTIONS"`)
     * @param string url : the URL to query
     * @param table headers : a table with string key-value pairs containing the request headers
     * @param string body : the request's body (only used for POST, PUT and PATCH requests)
     * @param string contentType : the body's content-type
     * @param function function : function that will be called when the request is executed
     */
    int HttpRequest(lua_State* L)
    {
        std::string httpVerb = ALE::CHECKVAL<std::string>(L, 1);
        std::string url = ALE::CHECKVAL<std::string>(L, 2);
        std::string body;
        std::string bodyContentType;
        httplib::Headers headers;

        int headersIdx = 3;
        int callbackIdx = 3;

        if (!lua_istable(L, headersIdx) && lua_isstring(L, headersIdx) && lua_isstring(L, headersIdx + 1))
        {
            body = ALE::CHECKVAL<std::string>(L, 3);
            bodyContentType = ALE::CHECKVAL<std::string>(L, 4);
            headersIdx = 5;
            callbackIdx = 5;
        }

        if (lua_istable(L, headersIdx))
        {
            ++callbackIdx;

            lua_pushnil(L); // First key
            while (lua_next(L, headersIdx) != 0)
            {
                // Uses 'key' (at index -2) and 'value' (at index -1)
                if (lua_isstring(L, -2))
                {
                    std::string key(lua_tostring(L, -2));
                    std::string value(lua_tostring(L, -1));
                    headers.insert(std::pair<std::string, std::string>(key, value));
                }
                // Removes 'value'; keeps 'key' for next iteration
                lua_pop(L, 1);
            }
        }

        lua_pushvalue(L, callbackIdx);
        int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
        if (funcRef >= 0)
        {
            ALE* sALE = ALE::GetALE(L);
            sALE->httpManager.PushRequest(new HttpWorkItem(funcRef, sALE, httpVerb, url, body, bodyContentType, headers));
        }
        else
        {
            luaL_argerror(L, callbackIdx, "unable to make a ref to function");
        }

        return 0;
    }

    /**
     * Returns an object representing a `long long` (64-bit) value.
     *
     * The value by default is 0, but can be initialized to a value by passing a number or long long as a string.
     *
     * @proto value = ()
     * @proto value = (n)
     * @proto value = (n_ll)
     * @proto value = (n_str)
     * @param int32 n
     * @param int64 n_ll
     * @param string n_str
     * @return int64 value
     */
    int CreateLongLong(lua_State* L)
    {
        long long init = 0;
        if (lua_isstring(L, 1))
        {
            std::string str = ALE::CHECKVAL<std::string>(L, 1);
            std::istringstream iss(str);
            iss >> init;
            if (iss.bad())
                return luaL_argerror(L, 1, "long long (as string) could not be converted");
        }
        else if (!lua_isnoneornil(L, 1))
            init = ALE::CHECKVAL<long long>(L, 1);

        ALE::Push(L, init);
        return 1;
    }

    /**
     * Returns an object representing an `unsigned long long` (64-bit) value.
     *
     * The value by default is 0, but can be initialized to a value by passing a number or unsigned long long as a string.
     *
     * @proto value = ()
     * @proto value = (n)
     * @proto value = (n_ull)
     * @proto value = (n_str)
     * @param uint32 n
     * @param uint64 n_ull
     * @param string n_str
     * @return uint64 value
     */
    int CreateULongLong(lua_State* L)
    {
        unsigned long long init = 0;
        if (lua_isstring(L, 1))
        {
            std::string str = ALE::CHECKVAL<std::string>(L, 1);
            std::istringstream iss(str);
            iss >> init;
            if (iss.bad())
                return luaL_argerror(L, 1, "unsigned long long (as string) could not be converted");
        }
        else if (!lua_isnoneornil(L, 1))
            init = ALE::CHECKVAL<unsigned long long>(L, 1);

        ALE::Push(L, init);
        return 1;
    }

    /**
     * Unbinds event handlers for either all [BattleGround] events, or one type of event.
     *
     * If `event_type` is `nil`, all [BattleGround] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterBGEvent]
     */
    int ClearBattleGroundEvents(lua_State* L)
    {
        typedef EventKey<Hooks::BGEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->BGEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->BGEventBindings->Clear(Key((Hooks::BGEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Creature]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Creature]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Creature], not just one.
     * To bind and unbind events to a single [Creature], see [Global:RegisterUniqueCreatureEvent] and [Global:ClearUniqueCreatureEvents].
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of one or more [Creature]s whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterCreatureEvent]
     */
    int ClearCreatureEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::CreatureEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
                E->CreatureEventBindings->Clear(Key((Hooks::CreatureEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->CreatureEventBindings->Clear(Key((Hooks::CreatureEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Creature]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Creature]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect only a single [Creature].
     * To bind and unbind events to all instances of a [Creature], see [Global:RegisterCreatureEvent] and [Global:ClearCreatureEvent].
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param ObjectGuid guid : the GUID of a single [Creature] whose handlers will be cleared
     * @param uint32 instance_id : the instance ID of a single [Creature] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterCreatureEvent]
     */
    int ClearUniqueCreatureEvents(lua_State* L)
    {
        typedef UniqueObjectKey<Hooks::CreatureEvents> Key;

        if (lua_isnoneornil(L, 3))
        {
            ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
            uint32 instanceId = ALE::CHECKVAL<uint32>(L, 2);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
                E->CreatureUniqueBindings->Clear(Key((Hooks::CreatureEvents)i, guid, instanceId));
        }
        else
        {
            ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 1);
            uint32 instanceId = ALE::CHECKVAL<uint32>(L, 2);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 3);
            ALE::GetALE(L)->CreatureUniqueBindings->Clear(Key((Hooks::CreatureEvents)event_type, guid, instanceId));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Creature]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Creature]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Creature], not just one.
     * To bind and unbind gossip events to a single [Creature], tell the ALE developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [Creature] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterCreatureGossipEvent]
     */
    int ClearCreatureGossipEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->CreatureGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->CreatureGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [GameObject]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [GameObject]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [GameObject], not just one.
     * To bind and unbind events to a single [GameObject], tell the ALE developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [GameObject] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGameObjectEvent]
     */
    int ClearGameObjectEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::GameObjectEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::GAMEOBJECT_EVENT_COUNT; ++i)
                E->GameObjectEventBindings->Clear(Key((Hooks::GameObjectEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->GameObjectEventBindings->Clear(Key((Hooks::GameObjectEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [GameObject]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [GameObject]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [GameObject], not just one.
     * To bind and unbind gossip events to a single [GameObject], tell the ALE developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [GameObject] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGameObjectGossipEvent]
     */
    int ClearGameObjectGossipEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->GameObjectGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->GameObjectGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Group] events, or one type of [Group] event.
     *
     * If `event_type` is `nil`, all [Group] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGroupEvent]
     */
    int ClearGroupEvents(lua_State* L)
    {
        typedef EventKey<Hooks::GroupEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->GroupEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->GroupEventBindings->Clear(Key((Hooks::GroupEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Guild] events, or one type of [Guild] event.
     *
     * If `event_type` is `nil`, all [Guild] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterGuildEvent]
     */
    int ClearGuildEvents(lua_State* L)
    {
        typedef EventKey<Hooks::GuildEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->GuildEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->GuildEventBindings->Clear(Key((Hooks::GuildEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of an [Item]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Item]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Item], not just one.
     * To bind and unbind events to a single [Item], tell the ALE developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of an [Item] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterItemEvent]
     */
    int ClearItemEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::ItemEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::ITEM_EVENT_COUNT; ++i)
                E->ItemEventBindings->Clear(Key((Hooks::ItemEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->ItemEventBindings->Clear(Key((Hooks::ItemEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of an [Item]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Item]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * **NOTE:** this will affect all instances of the [Item], not just one.
     * To bind and unbind gossip events to a single [Item], tell the ALE developers to implement that.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of an [Item] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterItemGossipEvent]
     */
    int ClearItemGossipEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->ItemGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->ItemGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [WorldPacket] opcode's events, or one type of event.
     *
     * If `event_type` is `nil`, all the [WorldPacket] opcode's event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (opcode)
     * @proto (opcode, event_type)
     * @param uint32 opcode : the type of [WorldPacket] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPacketEvent]
     */
    int ClearPacketEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::PacketEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::PACKET_EVENT_COUNT; ++i)
                E->PacketEventBindings->Clear(Key((Hooks::PacketEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->PacketEventBindings->Clear(Key((Hooks::PacketEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Player] events, or one type of [Player] event.
     *
     * If `event_type` is `nil`, all [Player] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPlayerEvent]
     */
    int ClearPlayerEvents(lua_State* L)
    {
        typedef EventKey<Hooks::PlayerEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->PlayerEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->PlayerEventBindings->Clear(Key((Hooks::PlayerEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Player]'s gossip events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Player]'s gossip event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the low GUID of a [Player] whose handlers will be cleared
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPlayerGossipEvent]
     */
    int ClearPlayerGossipEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                E->PlayerGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->PlayerGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all server events, or one type of event.
     *
     * If `event_type` is `nil`, all server event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterServerEvent]
     */
    int ClearServerEvents(lua_State* L)
    {
        typedef EventKey<Hooks::ServerEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->ServerEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->ServerEventBindings->Clear(Key((Hooks::ServerEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a non-instanced [Map]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the non-instanced [Map]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (map_id)
     * @proto (map_id, event_type)
     * @param uint32 map_id : the ID of a [Map]
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterPlayerGossipEvent]
     */
    int ClearMapEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::InstanceEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
                E->MapEventBindings->Clear(Key((Hooks::InstanceEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->MapEventBindings->Clear(Key((Hooks::InstanceEvents)event_type, entry));
        }

        return 0;
    }

    /**
     * Unbinds event handlers for either all of an instanced [Map]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the instanced [Map]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto (instance_id)
     * @proto (instance_id, event_type)
     * @param uint32 entry : the ID of an instance of a [Map]
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterInstanceEvent]
     */
    int ClearInstanceEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::InstanceEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
                E->InstanceEventBindings->Clear(Key((Hooks::InstanceEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->InstanceEventBindings->Clear(Key((Hooks::InstanceEvents)event_type, entry));
        }

        return 0;
    }

    /**
     * Unbinds event handlers for either all [Ticket] events, or one type of [Ticket] event.
     *
     * If `event_type` is `nil`, all [Ticket] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterTicketEvent]
     */
    int ClearTicketEvents(lua_State* L)
    {
        typedef EventKey<Hooks::TicketEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->TicketEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->TicketEventBindings->Clear(Key((Hooks::TicketEvents)event_type));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all of a [Spell]'s events, or one type of event.
     *
     * If `event_type` is `nil`, all the [Spell]'s event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     *
     * @proto (entry)
     * @proto (entry, event_type)
     * @param uint32 entry : the ID of a [Spell]s
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterSpellEvent]
     */
    int ClearSpellEvents(lua_State* L)
    {
        typedef EntryKey<Hooks::SpellEvents> Key;

        if (lua_isnoneornil(L, 2))
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);

            ALE* E = ALE::GetALE(L);
            for (uint32 i = 1; i < Hooks::SPELL_EVENT_COUNT; ++i)
                E->SpellEventBindings->Clear(Key((Hooks::SpellEvents)i, entry));
        }
        else
        {
            uint32 entry = ALE::CHECKVAL<uint32>(L, 1);
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 2);
            ALE::GetALE(L)->SpellEventBindings->Clear(Key((Hooks::SpellEvents)event_type, entry));
        }
        return 0;
    }

    /**
     * Unbinds event handlers for either all [Creature] events, or one type of [Creature] event.
     *
     * If `event_type` is `nil`, all [Creature] event handlers are cleared.
     *
     * Otherwise, only event handlers for `event_type` are cleared.
     *
     * @proto ()
     * @proto (event_type)
     * @param uint32 event_type : the event whose handlers will be cleared, see [Global:RegisterAllCreatureEvent]
     */
    int ClearAllCreatureEvents(lua_State* L)
    {
        typedef EventKey<Hooks::AllCreatureEvents> Key;

        if (lua_isnoneornil(L, 1))
        {
            ALE::GetALE(L)->AllCreatureEventBindings->Clear();
        }
        else
        {
            uint32 event_type = ALE::CHECKVAL<uint32>(L, 1);
            ALE::GetALE(L)->AllCreatureEventBindings->Clear(Key((Hooks::AllCreatureEvents)event_type));
        }
        return 0;
    }

    /**
     * Gets the faction which is the current owner of Halaa in Nagrand
     * 0 = Alliance
     * 1 = Horde
     *
     * 600 = slider max Alliance
     * -600 = slider max Horde
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @return int16 the ID of the team to own Halaa
     * @return float the slider position.
     */
    int GetOwnerHalaa(lua_State* L)
    {
        OutdoorPvPNA* nagrandPvp = (OutdoorPvPNA*)sOutdoorPvPMgr->GetOutdoorPvPToZoneId(3518);
        OPvPCapturePointNA* halaa = nagrandPvp->GetCapturePoint();
        ALE::Push(L, halaa->GetControllingFaction());
        ALE::Push(L, halaa->GetSlider());

        return 2;
    }

    /**
     * Sets the owner of Halaa in Nagrand to the respective faction
     * 0 = Alliance
     * 1 = Horde
     *
     * In multistate, this method is only available in the WORLD state
     *
     * @param uint16 teamId : the ID of the team to own Halaa
     */
    int SetOwnerHalaa(lua_State* L)
    {
        uint16 teamId = ALE::CHECKVAL<uint16>(L, 1);

        OutdoorPvPNA* nagrandPvp = (OutdoorPvPNA*)sOutdoorPvPMgr->GetOutdoorPvPToZoneId(3518);
        OPvPCapturePointNA* halaa = nagrandPvp->GetCapturePoint();

        if (teamId == 0)
        {
            halaa->SetSlider(599);
        }
        else if (teamId == 1)
        {
            halaa->SetSlider(-599);
        }
        else
        {
            return luaL_argerror(L, 1, "0 for Alliance or 1 for Horde expected");
        }

        return 0;
    }

    /**
     * Gets the localized OptionText and BoxText for a specific gossip menu option.
     * If the text for the specified locale is not found, it returns the default text.
     *
     * @param uint32 menuId : The ID of the gossip menu.
     * @param uint32 optionId : The ID of the gossip menu option.
     * @param uint8 locale : The locale to retrieve the text for. 0 represents the default locale.
     *
     * @return string, string : The localized OptionText and BoxText for the gossip menu option, or the default text if no localization is found.
     */
    int GetGossipMenuOptionLocale(lua_State* L)
    {
        uint32 menuId = ALE::CHECKVAL<uint32>(L, 1);
        uint32 optionId = ALE::CHECKVAL<uint32>(L, 2);
        uint8 locale = ALE::CHECKVAL<uint8>(L, 3);

        std::string strOptionText;
        std::string strBoxText;

        if (locale != DEFAULT_LOCALE)
        {
            if (GossipMenuItemsLocale const* gossipMenuLocale = sObjectMgr->GetGossipMenuItemsLocale(MAKE_PAIR32(menuId, optionId)))
            {
                ObjectMgr::GetLocaleString(gossipMenuLocale->OptionText, LocaleConstant(locale), strOptionText);
                ObjectMgr::GetLocaleString(gossipMenuLocale->BoxText, LocaleConstant(locale), strBoxText);
            }
        }

        if (strOptionText.empty() || strBoxText.empty())
        {
            GossipMenuItemsMapBounds bounds = sObjectMgr->GetGossipMenuItemsMapBounds(menuId);
            for (auto itr = bounds.first; itr != bounds.second; ++itr)
            {
                if (itr->second.OptionID == optionId)
                {
                    if (strOptionText.empty())
                        strOptionText = itr->second.OptionText;
                    if (strBoxText.empty())
                        strBoxText = itr->second.BoxText;
                    break;
                }
            }
        }

        ALE::Push(L, strOptionText);
        ALE::Push(L, strBoxText);
        return 2;
    }

    /**
     * Return the entrance position (x, y, z, o) of the specified dungeon map id.
     *
     * @param uint32 mapId
     *
     * @return uint32 pos_x
     * @return uint32 pos_y
     * @return uint32 pos_z
     * @return uint32 pos_o
     */
    int GetMapEntrance(lua_State* L)
    {
        uint32 mapId = ALE::CHECKVAL<uint32>(L, 1);
        AreaTriggerTeleport const* at = sObjectMgr->GetMapEntranceTrigger(mapId);

        if (!at)
        {
            lua_pushnil(L);
            return 1;
        }

        ALE::Push(L, at->target_X);
        ALE::Push(L, at->target_Y);
        ALE::Push(L, at->target_Z);
        ALE::Push(L, at->target_Orientation);

        return 5;
    }
      
    /**  
     * Get the [SpellInfo] for the specified [Spell] id
     *
     * @param uint32 spellId : the ID of the spell
     * @return [SpellInfo] spellInfo
     */
    int GetSpellInfo(lua_State* L)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 1);
        ALE::Push(L, sSpellMgr->GetSpellInfo(spellId));
        return 1;

    }
  
    /**
     * Returns an entry from the specified DBC (DatabaseClient) store.
     *
     * This function looks up an entry in a DBC file by name and ID, and pushes it onto the Lua stack.
     *
     * @param string dbcName : The name of the DBC store (e.g., "ItemDisplayInfo")
     * @param uint32 id : The ID used to look up within the specified DBC store
     *
     * @return [DBCStore] store : The requested DBC store instance
     */
    int LookupEntry(lua_State* L)
    {
        const char* dbcName = ALE::CHECKVAL<const char*>(L, 1);
        uint32 id = ALE::CHECKVAL<uint32>(L, 2);

        for (const auto& dbc : dbcRegistry)
        {
            if (dbc.name == dbcName)
            {
                const void* entry = dbc.lookupFunction(id);
                if (!entry)
                    return 0;

                dbc.pushFunction(L, entry);
                return 1;
            }
        }

        return luaL_error(L, "Invalid DBC name: %s", dbcName);
    }
}
#endif
