/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"
#include "LuaEngine.h"

#include "ALEDBCRegistry.h"
#include "ALEEventMgr.h"

#include "BanMgr.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameEventMgr.h"
#include "GameObject.h"
#include "GameTime.h"
#include "GitRevision.h"
#include "GuildMgr.h"
#include "Mail.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "OutdoorPvPMgr.h"
#include "QueryCallback.h"
#include "SharedDefines.h"
#include "SpellMgr.h"
#include "StringConvert.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "Util.h"
#include "WorldPacket.h"
#include "WorldSessionMgr.h"
#include "../../../../src/server/scripts/OutdoorPvP/OutdoorPvPNA.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <list>
#include <shared_mutex>
#include <sstream>
#include <string>

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
    char const* GetLuaEngine()
    {
        return "ALEEngine";
    }

    /**
     * Returns emulator's name.
     *
     * The result will be either `MaNGOS`, `cMaNGOS`, or `TrinityCore`.
     *
     * @return string coreName
     */
    char const* GetCoreName()
    {
        return "AzerothCore";
    }

    /**
     * Returns config value as a string.
     *
     * @param string name : name of the value
     * @return string value
     */
    sol::object GetConfigValue(std::string key, sol::this_state s)
    {
        sol::state_view lua(s);

        std::string val = sConfigMgr->GetOption<std::string>(key, "", false);

        if (val.empty())
            return sol::make_object(lua, val);

        std::string lower = val;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "true")
            return sol::make_object(lua, true);
        else if (lower == "false")
            return sol::make_object(lua, false);

        auto intVal = Acore::StringTo<uint32>(val);
        if (intVal)
            return sol::make_object(lua, *intVal);

        return sol::make_object(lua, val);
    }

    /**
     * Returns emulator .conf RealmID
     *
     * - for MaNGOS returns the realmID as it is stored in the core.
     * - for TrinityCore returns the realmID as it is in the conf file.
     * @return uint32 realm ID
     */
    uint32 GetRealmID()
    {
        return sConfigMgr->GetOption<uint32>("RealmID", 1);
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
    char const* GetCoreVersion()
    {
        return GitRevision::GetFullVersion();
    }

    /**
     * Returns emulator's supported expansion.
     *
     * Expansion is 0 for pre-TBC, 1 for TBC, 2 for WotLK, and 3 for Cataclysm.
     *
     * @return int32 expansion
     */
    int32 GetCoreExpansion()
    {
        return 2;
    }

    /**
     * Returns the [Map] pointer of the Lua state. Returns null for the "World" state. 
     *
     * @return [Map] map
     */
    Map* GetStateMap()
    {
        // Until AC supports multistate, this will always return nil
        return nullptr;
    }

    /**
     * Returns the map ID of the Lua state. Returns -1 for the "World" state.
     *
     * @return int32 mapId
     */
    int32 GetStateMapId()
    {
        // Until AC supports multistate, this will always return -1
        return -1;
    }

    /**
     * Returns the instance ID of the Lua state. Returns 0 for continent maps and the world state.
     *
     * @return uint32 instanceId
     */
    uint32 GetStateInstanceId()
    {
        // Until AC supports multistate, this will always return 0
        return 0;
    }

    /**
     * Returns [Quest] template
     *
     * @param uint32 questId : [Quest] entry ID
     * @return [Quest] quest
     */
    Quest* GetQuest(uint32 questId)
    {
        return const_cast<Quest*>(sObjectMgr->GetQuestTemplate(questId));
    }

    /**
     * Finds and Returns [Player] by guid if found
     *
     * @param ObjectGuid guid : guid of the [Player], you can get it with [Object:GetGUID]
     * @return [Player] player
     */
    Player* GetPlayerByGUID(ObjectGuid guid)
    {
        return ObjectAccessor::FindPlayer(guid);
    }

    /**
     * Finds and Returns [Player] by name if found
     *
     * @param string name : name of the [Player]
     * @return [Player] player
     */
    Player* GetPlayerByName(std::string name)
    {
        return ObjectAccessor::FindPlayerByName(name);
    }

    /**
     * Returns game time in seconds
     *
     * @return uint32 time
     */
    int64 GetGameTime()
    {
        return GameTime::GetGameTime().count();
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
     * @param [TeamId] team = TEAM_NEUTRAL : optional check team of the [Player], Alliance, Horde or Neutral (All)
     * @param bool onlyGM = false : optional check if GM only
     * @return table worldPlayers
     */
    sol::table GetPlayersInWorld(sol::optional<uint32> teamArg, sol::optional<bool> onlyGMArg, sol::this_state s)
    {
        uint32 team = teamArg.value_or(TEAM_NEUTRAL);
        bool onlyGM = onlyGMArg.value_or(false);

        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        {
            std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
            const HashMapHolder<Player>::MapType& m = ObjectAccessor::GetPlayers();
            for (HashMapHolder<Player>::MapType::const_iterator it = m.begin(); it != m.end(); ++it)
            {
                if (Player* player = it->second)
                {
                    if (!player->IsInWorld())
                        continue;

                    if ((team == TEAM_NEUTRAL || player->GetTeamId() == team) && (!onlyGM || player->IsGameMaster()))
                        tbl[++i] = PlayerRef(player);
                }
            }
        }

        return tbl;
    }

    /**
     * Returns a [Guild] by name.
     *
     * @param string name
     * @return [Guild] guild : the Guild, or `nil` if it doesn't exist
     */
    Guild* GetGuildByName(std::string name)
    {
        return sGuildMgr->GetGuildByName(name);
    }

    /**
     * Returns a [Map] by ID.
     *
     * @param uint32 mapId : see [Map.dbc](https://github.com/cmangos/issues/wiki/Map.dbc)
     * @param uint32 instanceId = 0 : required if the map is an instance, otherwise don't pass anything
     * @return [Map] map : the Map, or `nil` if it doesn't exist
     */
    Map* GetMapById(uint32 mapid, sol::optional<uint32> instanceArg)
    {
        uint32 instance = instanceArg.value_or(0);

        return sMapMgr->FindMap(mapid, instance);
    }

    /**
     * Returns [Guild] by the leader's GUID
     *
     * @param ObjectGuid guid : the guid of a [Guild] leader
     * @return [Guild] guild, or `nil` if it doesn't exist
     */
    Guild* GetGuildByLeaderGUID(ObjectGuid guid)
    {
        return sGuildMgr->GetGuildByLeader(guid);
    }

    /**
     * Returns the amount of [Player]s in the world.
     *
     * @return uint32 count
     */
    uint32 GetPlayerCount()
    {
        return sWorldSessionMgr->GetActiveSessionCount();
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
    ObjectGuid GetPlayerGUID(uint32 lowguid)
    {
        return ObjectGuid(HighGuid::Player, 0, lowguid);
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
    ObjectGuid GetItemGUID(uint32 lowguid)
    {
        return ObjectGuid(HighGuid::Item, 0, lowguid);
    }

    /**
    * Returns the [ItemTemplate] for the specified item ID.  The ItemTemplate contains all static data about an item, such as name, quality, stats, required level, and more.
    *
    * @param uint32 itemID : the item entry ID from `item_template` to look up
    * @return [ItemTemplate] itemTemplate
    */
    ItemTemplate* GetItemTemplate(uint32 entry)
    {
        return const_cast<ItemTemplate*>(sObjectMgr->GetItemTemplate(entry));
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
    ObjectGuid GetObjectGUID(uint32 lowguid, uint32 entry)
    {
        return ObjectGuid(HighGuid::GameObject, entry, lowguid);
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
    ObjectGuid GetUnitGUID(uint32 lowguid, uint32 entry)
    {
        return ObjectGuid(HighGuid::Unit, entry, lowguid);
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
    uint32 GetGUIDLow(ObjectGuid guid)
    {
        return guid.GetCounter();
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
    std::string GetItemLink(uint32 entry, sol::optional<uint8> localeArg)
    {
        uint8 locale = localeArg.value_or(DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            throw std::invalid_argument("valid LocaleConstant expected");

        const ItemTemplate* temp = sObjectMgr->GetItemTemplate(entry);
        if (!temp)
            throw std::invalid_argument("valid ItemEntry expected");

        std::string name = temp->Name1;
        if (ItemLocale const* il = sObjectMgr->GetItemLocale(entry))
            ObjectMgr::GetLocaleString(il->Name, static_cast<LocaleConstant>(locale), name);

        std::ostringstream oss;
        oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
            "|Hitem:" << entry << ":0:" <<
            "0:0:0:0:" <<
            "0:0:0:0|h[" << name << "]|h|r";

        return oss.str();
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
    int32 GetGUIDType(ObjectGuid guid)
    {
        return static_cast<int>(guid.GetHigh());
    }

    /**
     * Returns the entry ID from a GUID.
     *
     * GUID consist of entry ID, low GUID, and type ID.
     *
     * @param ObjectGuid guid : GUID of an [Creature] or [GameObject]
     * @return uint32 entry : entry ID, or `0` if `guid` is not a [Creature] or [GameObject]
     */
    uint32 GetGUIDEntry(ObjectGuid guid)
    {
        return guid.GetEntry();
    }

    /**
     * Returns the byte size in bytes (2-9) of the ObjectGuid when packed.
     *
     * @param ObjectGuid guid : the ObjectGuid to get packed size for
     * @return number size
     */
    int32 GetPackedGUIDSize(ObjectGuid guid)
    {
        PackedGuid packedGuid(guid);
        return static_cast<int>(packedGuid.size());
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
    char const* GetAreaName(uint32 areaOrZoneId, sol::optional<uint8> localeArg)
    {
        uint8 locale = localeArg.value_or(DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            throw std::invalid_argument("valid LocaleConstant expected");

        AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(areaOrZoneId);

        if (!areaEntry)
            throw std::invalid_argument("valid Area or Zone ID expected");

        return areaEntry->area_name[locale];
    }

    /**
     * Returns the currently active game events.
     *
     * @return table activeEvents
     */
    sol::table GetActiveGameEvents(sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 counter = 1;
        GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr->GetActiveEventList();

        for (GameEventMgr::ActiveEvents::const_iterator i = activeEvents.begin(); i != activeEvents.end(); ++i)
        {
            tbl[counter] = *i;

            counter++;
        }

        return tbl;
    }

    static sol::object RegisterEntryHelper(uint8 regtype, uint32 id, uint32 ev, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return sALE->Register(regtype, id, ObjectGuid::Empty, 0, ev, std::move(callback), shots.value_or(0));
    }

    static sol::object RegisterEventHelper(uint8 regtype, uint32 ev, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return sALE->Register(regtype, 0, ObjectGuid::Empty, 0, ev, std::move(callback), shots.value_or(0));
    }

    static sol::object RegisterUniqueHelper(uint8 regtype, ObjectGuid guid, uint32 instanceId, uint32 ev, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return sALE->Register(regtype, 0, guid, instanceId, ev, std::move(callback), shots.value_or(0));
    }

    /**
     * Registers a server event handler.
     *
     *     enum ServerEvents
     *     {
     *         // Server
     *         SERVER_EVENT_ON_NETWORK_START           =     1,       // Not Implemented
     *         SERVER_EVENT_ON_NETWORK_STOP            =     2,       // Not Implemented
     *         SERVER_EVENT_ON_SOCKET_OPEN             =     3,       // Not Implemented
     *         SERVER_EVENT_ON_SOCKET_CLOSE            =     4,       // Not Implemented
     *         SERVER_EVENT_ON_PACKET_RECEIVE          =     5,       // (event, packet, player) - Player only if accessible. Can return false, newPacket
     *         SERVER_EVENT_ON_PACKET_RECEIVE_UNKNOWN  =     6,       // Not Implemented
     *         SERVER_EVENT_ON_PACKET_SEND             =     7,       // (event, packet, player) - Player only if accessible. Can return false, newPacket
     *
     *         // World
     *         WORLD_EVENT_ON_OPEN_STATE_CHANGE        =     8,        // (event, open) - Needs core support on Mangos
     *         WORLD_EVENT_ON_CONFIG_LOAD              =     9,        // (event, reload)
     *         // UNUSED                               =     10,
     *         WORLD_EVENT_ON_SHUTDOWN_INIT            =     11,       // (event, code, mask)
     *         WORLD_EVENT_ON_SHUTDOWN_CANCEL          =     12,       // (event)
     *         WORLD_EVENT_ON_UPDATE                   =     13,       // (event, diff)
     *         WORLD_EVENT_ON_STARTUP                  =     14,       // (event)
     *         WORLD_EVENT_ON_SHUTDOWN                 =     15,       // (event)
     *
     *         // ALE
     *         ALE_EVENT_ON_LUA_STATE_CLOSE          =     16,       // (event) - triggers just before shutting down ALE (on shutdown and restart)
     *
     *         // Map
     *         MAP_EVENT_ON_CREATE                     =     17,       // (event, map)
     *         MAP_EVENT_ON_DESTROY                    =     18,       // (event, map)
     *         MAP_EVENT_ON_GRID_LOAD                  =     19,       // Not Implemented
     *         MAP_EVENT_ON_GRID_UNLOAD                =     20,       // Not Implemented
     *         MAP_EVENT_ON_PLAYER_ENTER               =     21,       // (event, map, player)
     *         MAP_EVENT_ON_PLAYER_LEAVE               =     22,       // (event, map, player)
     *         MAP_EVENT_ON_UPDATE                     =     23,       // (event, map, diff)
     *
     *         // Area trigger
     *         TRIGGER_EVENT_ON_TRIGGER                =     24,       // (event, player, triggerId) - Can return true
     *
     *         // Weather
     *         WEATHER_EVENT_ON_CHANGE                 =     25,       // (event, zoneId, state, grade)
     *
     *         // Auction house
     *         AUCTION_EVENT_ON_ADD                    =     26,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *         AUCTION_EVENT_ON_REMOVE                 =     27,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *         AUCTION_EVENT_ON_SUCCESSFUL             =     28,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *         AUCTION_EVENT_ON_EXPIRE                 =     29,       // (event, auctionId, owner, item, expireTime, buyout, startBid, currentBid, bidderGUIDLow)
     *
     *         // AddOns
     *         ADDON_EVENT_ON_MESSAGE                  =     30,       // (event, sender, type, prefix, msg, target) - target can be nil/whisper_target/guild/group/channel. Can return false
     *
     *         WORLD_EVENT_ON_DELETE_CREATURE          =     31,       // (event, creature)
     *         WORLD_EVENT_ON_DELETE_GAMEOBJECT        =     32,       // (event, gameobject)
     *
     *         // ALE
     *         ALE_EVENT_ON_LUA_STATE_OPEN           =     33,       // (event) - triggers after all scripts are loaded
     *
     *         GAME_EVENT_START                        =     34,       // (event, gameeventid)
     *         GAME_EVENT_STOP                         =     35,       // (event, gameeventid)
     *     };
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : server event ID, refer to ServerEvents above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterServerEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_SERVER, event, std::move(callback), shots);
    }

    /**
     * Registers a [Player] event handler.
     *
     * <pre>
     * enum PlayerEvents
     * {
     *     PLAYER_EVENT_ON_CHARACTER_CREATE                     =     1,        // (event, player)
     *     PLAYER_EVENT_ON_CHARACTER_DELETE                     =     2,        // (event, guid)
     *     PLAYER_EVENT_ON_LOGIN                                =     3,        // (event, player)
     *     PLAYER_EVENT_ON_LOGOUT                               =     4,        // (event, player)
     *     PLAYER_EVENT_ON_SPELL_CAST                           =     5,        // (event, player, spell, skipCheck)
     *     PLAYER_EVENT_ON_KILL_PLAYER                          =     6,        // (event, killer, killed)
     *     PLAYER_EVENT_ON_KILL_CREATURE                        =     7,        // (event, killer, killed)
     *     PLAYER_EVENT_ON_KILLED_BY_CREATURE                   =     8,        // (event, killer, killed)
     *     PLAYER_EVENT_ON_DUEL_REQUEST                         =     9,        // (event, target, challenger)
     *     PLAYER_EVENT_ON_DUEL_START                           =     10,       // (event, player1, player2)
     *     PLAYER_EVENT_ON_DUEL_END                             =     11,       // (event, winner, loser, type)
     *     PLAYER_EVENT_ON_GIVE_XP                              =     12,       // (event, player, amount, victim, source) - Can return new XP amount
     *     PLAYER_EVENT_ON_LEVEL_CHANGE                         =     13,       // (event, player, oldLevel)
     *     PLAYER_EVENT_ON_MONEY_CHANGE                         =     14,       // (event, player, amount) - Can return new money amount
     *     PLAYER_EVENT_ON_REPUTATION_CHANGE                    =     15,       // (event, player, factionId, standing, incremental) - Can return new standing -> if standing == -1, it will prevent default action (rep gain)
     *     PLAYER_EVENT_ON_TALENTS_CHANGE                       =     16,       // (event, player, points)
     *     PLAYER_EVENT_ON_TALENTS_RESET                        =     17,       // (event, player, noCost)
     *     PLAYER_EVENT_ON_CHAT                                 =     18,       // (event, player, msg, Type, lang) - Can return false, newMessage
     *     PLAYER_EVENT_ON_WHISPER                              =     19,       // (event, player, msg, Type, lang, receiver) - Can return false, newMessage
     *     PLAYER_EVENT_ON_GROUP_CHAT                           =     20,       // (event, player, msg, Type, lang, group) - Can return false, newMessage
     *     PLAYER_EVENT_ON_GUILD_CHAT                           =     21,       // (event, player, msg, Type, lang, guild) - Can return false, newMessage
     *     PLAYER_EVENT_ON_CHANNEL_CHAT                         =     22,       // (event, player, msg, Type, lang, channel) - channel is negative for custom channels. Can return false, newMessage
     *     PLAYER_EVENT_ON_EMOTE                                =     23,       // (event, player, emote) - Not triggered on any known emote
     *     PLAYER_EVENT_ON_TEXT_EMOTE                           =     24,       // (event, player, textEmote, emoteNum, guid)
     *     PLAYER_EVENT_ON_SAVE                                 =     25,       // (event, player)
     *     PLAYER_EVENT_ON_BIND_TO_INSTANCE                     =     26,       // (event, player, difficulty, mapid, permanent)
     *     PLAYER_EVENT_ON_UPDATE_ZONE                          =     27,       // (event, player, newZone, newArea)
     *     PLAYER_EVENT_ON_MAP_CHANGE                           =     28,       // (event, player)
     *
     *     // Custom
     *     PLAYER_EVENT_ON_EQUIP                                =     29,       // (event, player, item, bag, slot)
     *     PLAYER_EVENT_ON_FIRST_LOGIN                          =     30,       // (event, player)
     *     PLAYER_EVENT_ON_CAN_USE_ITEM                         =     31,       // (event, player, itemEntry) - Can return InventoryResult enum value
     *     PLAYER_EVENT_ON_LOOT_ITEM                            =     32,       // (event, player, item, count)
     *     PLAYER_EVENT_ON_ENTER_COMBAT                         =     33,       // (event, player, enemy)
     *     PLAYER_EVENT_ON_LEAVE_COMBAT                         =     34,       // (event, player)
     *     PLAYER_EVENT_ON_REPOP                                =     35,       // (event, player)
     *     PLAYER_EVENT_ON_RESURRECT                            =     36,       // (event, player)
     *     PLAYER_EVENT_ON_LOOT_MONEY                           =     37,       // (event, player, amount)
     *     PLAYER_EVENT_ON_QUEST_ABANDON                        =     38,       // (event, player, questId)
     *     PLAYER_EVENT_ON_LEARN_TALENTS                        =     39,       // (event, player, talentId, talentRank, spellid)
     *     // UNUSED                                            =     40,       // (event, player)
     *     // UNUSED                                            =     41,       // (event, player)
     *     PLAYER_EVENT_ON_COMMAND                              =     42,       // (event, player, command, chatHandler) - player is nil if command used from console. Can return false
     *     PLAYER_EVENT_ON_PET_ADDED_TO_WORLD                   =     43,       // (event, player, pet)
     *     PLAYER_EVENT_ON_LEARN_SPELL                          =     44,       // (event, player, spellId)
     *     PLAYER_EVENT_ON_ACHIEVEMENT_COMPLETE                 =     45,       // (event, player, achievement)
     *     PLAYER_EVENT_ON_FFAPVP_CHANGE                        =     46,       // (event, player, hasFfaPvp)
     *     PLAYER_EVENT_ON_UPDATE_AREA                          =     47,       // (event, player, oldArea, newArea)
     *     PLAYER_EVENT_ON_CAN_INIT_TRADE                       =     48,       // (event, player, target) - Can return false to prevent the trade
     *     PLAYER_EVENT_ON_CAN_SEND_MAIL                        =     49,       // (event, player, receiverGuid, mailbox, subject, body, money, cod, item) - Can return false to prevent sending the mail
     *     PLAYER_EVENT_ON_CAN_JOIN_LFG                         =     50,       // (event, player, roles, dungeons, comment) - Can return false to prevent queueing
     *     PLAYER_EVENT_ON_QUEST_REWARD_ITEM                    =     51,       //  (event, player, item, count)
     *     PLAYER_EVENT_ON_CREATE_ITEM                          =     52,       //  (event, player, item, count)
     *     PLAYER_EVENT_ON_STORE_NEW_ITEM                       =     53,       //  (event, player, item, count)
     *     PLAYER_EVENT_ON_COMPLETE_QUEST                       =     54,       // (event, player, quest)
     *     PLAYER_EVENT_ON_CAN_GROUP_INVITE                     =     55,       // (event, player, memberName) - Can return false to prevent inviting
     *     PLAYER_EVENT_ON_GROUP_ROLL_REWARD_ITEM               =     56,       // (event, player, item, count, voteType, roll)
     *     PLAYER_EVENT_ON_BG_DESERTION                         =     57,       // (event, player, type)
     *     PLAYER_EVENT_ON_PET_KILL                             =     58,       // (event, player, killer)
     *     PLAYER_EVENT_ON_CAN_RESURRECT                        =     59,       // (event, player)
     *     PLAYER_EVENT_ON_CAN_UPDATE_SKILL                     =     60,       // (event, player, skill_id) -- Can return true or false
     *     PLAYER_EVENT_ON_BEFORE_UPDATE_SKILL                  =     61,       // (event, player, skill_id, value, max, step) -- Can return new amount
     *     PLAYER_EVENT_ON_UPDATE_SKILL                         =     62,       // (event, player, skill_id, value, max, step, new_value)
     *     PLAYER_EVENT_ON_QUEST_ACCEPT                         =     63,       // (event, player, quest)
     *     PLAYER_EVENT_ON_AURA_APPLY                           =     64,       // (event, player, aura)
     *     PLAYER_EVENT_ON_HEAL                                 =     65,       // (event, player, target, heal) - Can return new heal amount
     *     PLAYER_EVENT_ON_DAMAGE                               =     66,       // (event, player, target, damage) - Can return new damage amount
     *     PLAYER_EVENT_ON_AURA_REMOVE                          =     67,       // (event, player, aura, remove_mode)
     *     PLAYER_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK    =     68,    // (event, player, target, damage, spellInfo) - Can return new damage amount
     *     PLAYER_EVENT_ON_MODIFY_MELEE_DAMAGE                  =     69,       // (event, player, target, damage) - Can return new damage amount
     *     PLAYER_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN            =     70,       // (event, player, target, damage, spellInfo) - Can return new damage amount
     *     PLAYER_EVENT_ON_MODIFY_HEAL_RECEIVED                 =     71,       // (event, player, target, heal, spellInfo) - Can return new heal amount
     *     PLAYER_EVENT_ON_DEAL_DAMAGE                          =     72,       // (event, player, target, damage, damagetype) - Can return new damage amount
     *     PLAYER_EVENT_ON_RELEASED_GHOST                       =     73,       // (event, player)
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Player] event Id, refer to PlayerEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterPlayerEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_PLAYER, event, std::move(callback), shots);
    }

    /**
     * Registers a [Guild] event handler.
     *
     * <pre>
     * enum GuildEvents
     * {
     *     // Guild
     *     GUILD_EVENT_ON_ADD_MEMBER               =     1,       // (event, guild, player, rank)
     *     GUILD_EVENT_ON_REMOVE_MEMBER            =     2,       // (event, guild, player, isDisbanding)
     *     GUILD_EVENT_ON_MOTD_CHANGE              =     3,       // (event, guild, newMotd)
     *     GUILD_EVENT_ON_INFO_CHANGE              =     4,       // (event, guild, newInfo)
     *     GUILD_EVENT_ON_CREATE                   =     5,       // (event, guild, leader, name)  // Not on TC
     *     GUILD_EVENT_ON_DISBAND                  =     6,       // (event, guild)
     *     GUILD_EVENT_ON_MONEY_WITHDRAW           =     7,       // (event, guild, player, amount, isRepair) - Can return new money amount
     *     GUILD_EVENT_ON_MONEY_DEPOSIT            =     8,       // (event, guild, player, amount) - Can return new money amount
     *     GUILD_EVENT_ON_ITEM_MOVE                =     9,       // (event, guild, player, item, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId)   // TODO
     *     GUILD_EVENT_ON_EVENT                    =     10,      // (event, guild, eventType, plrGUIDLow1, plrGUIDLow2, newRank)  // TODO
     *     GUILD_EVENT_ON_BANK_EVENT               =     11,      // (event, guild, eventType, tabId, playerGUIDLow, itemOrMoney, itemStackCount, destTabId)
     *
     *     GUILD_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Guild] event Id, refer to GuildEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterGuildEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_GUILD, event, std::move(callback), shots);
    }

    /**
     * Registers a [Group] event handler.
     *
     * <pre>
     * enum GroupEvents
     * {
     *     // Group
     *     GROUP_EVENT_ON_MEMBER_ADD               =     1,       // (event, group, guid)
     *     GROUP_EVENT_ON_MEMBER_INVITE            =     2,       // (event, group, guid)
     *     GROUP_EVENT_ON_MEMBER_REMOVE            =     3,       // (event, group, guid, method, kicker, reason)
     *     GROUP_EVENT_ON_LEADER_CHANGE            =     4,       // (event, group, newLeaderGuid, oldLeaderGuid)
     *     GROUP_EVENT_ON_DISBAND                  =     5,       // (event, group)
     *     GROUP_EVENT_ON_CREATE                   =     6,       // (event, group, leaderGuid, groupType)
     *
     *     GROUP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [Group] event Id, refer to GroupEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterGroupEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_GROUP, event, std::move(callback), shots);
    }

    /**
     * Registers a [BattleGround] event handler.
     *
     * <pre>
     * enum BGEvents
     * {
     *     BG_EVENT_ON_START                               = 1,    // (event, bg, bgId, instanceId) - Needs to be added to TC
     *     BG_EVENT_ON_END                                 = 2,    // (event, bg, bgId, instanceId, winner) - Needs to be added to TC
     *     BG_EVENT_ON_CREATE                              = 3,    // (event, bg, bgId, instanceId) - Needs to be added to TC
     *     BG_EVENT_ON_PRE_DESTROY                         = 4,    // (event, bg, bgId, instanceId) - Needs to be added to TC
     *     BG_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (event, function)
     * @proto cancel = (event, function, shots)
     *
     * @param uint32 event : [BattleGround] event Id, refer to BGEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterBGEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_BG, event, std::move(callback), shots);
    }

    /**
     * Registers a [WorldPacket] event handler.
     *
     * <pre>
     * enum PacketEvents
     * {
     *     PACKET_EVENT_ON_PACKET_RECEIVE          =     5,       // (event, packet, player) - Player only if accessible. Can return false, newPacket
     *     PACKET_EVENT_ON_PACKET_RECEIVE_UNKNOWN  =     6,       // Not Implemented
     *     PACKET_EVENT_ON_PACKET_SEND             =     7,       // (event, packet, player) - Player only if accessible. Can return false, newPacket
     *
     *     PACKET_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : opcode
     * @param uint32 event : packet event Id, refer to PacketEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterPacketEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_PACKET, entry, event, std::move(callback), shots);
    }

    /**
     * Registers a [Creature] gossip event handler.
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 event : [Creature] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterCreatureGossipEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_CREATURE_GOSSIP, entry, event, std::move(callback), shots);
    }

    /**
     * Registers a [GameObject] gossip event handler.
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [GameObject] entry Id
     * @param uint32 event : [GameObject] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterGameObjectGossipEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_GAMEOBJECT_GOSSIP, entry, event, std::move(callback), shots);
    }

    /**
     * Registers an [Item] event handler.
     *
     * <pre>
     * enum ItemEvents
     * {
     *     ITEM_EVENT_ON_DUMMY_EFFECT                      = 1,    // (event, caster, spellid, effindex, item)
     *     ITEM_EVENT_ON_USE                               = 2,    // (event, player, item, target) - Can return false to stop the spell casting
     *     ITEM_EVENT_ON_QUEST_ACCEPT                      = 3,    // (event, player, item, quest) - Can return true
     *     ITEM_EVENT_ON_EXPIRE                            = 4,    // (event, player, itemid) - Can return true
     *     ITEM_EVENT_ON_REMOVE                            = 5,    // (event, player, item) - Can return true
     *     ITEM_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Item] entry Id
     * @param uint32 event : [Item] event Id, refer to ItemEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterItemEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_ITEM, entry, event, std::move(callback), shots);
    }

    /**
     * Registers an [Item] gossip event handler.
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [Item] entry Id
     * @param uint32 event : [Item] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterItemGossipEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_ITEM_GOSSIP, entry, event, std::move(callback), shots);
    }

    /**
     * Registers a [Map] event handler for all instance of a [Map].
     *
     * <pre>
     * enum InstanceEvents
     * {
     *     INSTANCE_EVENT_ON_INITIALIZE                    = 1,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_LOAD                          = 2,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_UPDATE                        = 3,    // (event, instance_data, map, diff)
     *     INSTANCE_EVENT_ON_PLAYER_ENTER                  = 4,    // (event, instance_data, map, player)
     *     INSTANCE_EVENT_ON_CREATURE_CREATE               = 5,    // (event, instance_data, map, creature)
     *     INSTANCE_EVENT_ON_GAMEOBJECT_CREATE             = 6,    // (event, instance_data, map, go)
     *     INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS   = 7,    // (event, instance_data, map)
     *     INSTANCE_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 map_id : ID of a [Map]
     * @param uint32 event : [Map] event ID, refer to MapEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    sol::object RegisterMapEvent(uint32 map_id, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_MAP, map_id, event, std::move(callback), shots);
    }

    /**
     * Registers a [Map] event handler for one instance of a [Map].
     *
     * <pre>
     * enum InstanceEvents
     * {
     *     INSTANCE_EVENT_ON_INITIALIZE                    = 1,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_LOAD                          = 2,    // (event, instance_data, map)
     *     INSTANCE_EVENT_ON_UPDATE                        = 3,    // (event, instance_data, map, diff)
     *     INSTANCE_EVENT_ON_PLAYER_ENTER                  = 4,    // (event, instance_data, map, player)
     *     INSTANCE_EVENT_ON_CREATURE_CREATE               = 5,    // (event, instance_data, map, creature)
     *     INSTANCE_EVENT_ON_GAMEOBJECT_CREATE             = 6,    // (event, instance_data, map, go)
     *     INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS   = 7,    // (event, instance_data, map)
     *     INSTANCE_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 instance_id : ID of an instance of a [Map]
     * @param uint32 event : [Map] event ID, refer to MapEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    sol::object RegisterInstanceEvent(uint32 instance_id, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_INSTANCE, instance_id, event, std::move(callback), shots);
    }

    /**
     * Registers a [Player] gossip event handler.
     *
     * Note that you can not use `GOSSIP_EVENT_ON_HELLO` with this hook. It does nothing since players dont have an "on hello".
     *
     * <pre>
     * enum GossipEvents
     * {
     *     GOSSIP_EVENT_ON_HELLO                           = 1,    // (event, player, object) - Object is the Creature/GameObject/Item. Can return false to do default action. For item gossip can return false to stop spell casting.
     *     GOSSIP_EVENT_ON_SELECT                          = 2,    // (event, player, object, sender, intid, code, menu_id) - Object is the Creature/GameObject/Item/Player, menu_id is only for player gossip. Can return false to do default action.
     *     GOSSIP_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (menu_id, event, function)
     * @proto cancel = (menu_id, event, function, shots)
     *
     * @param uint32 menu_id : [Player] gossip menu Id
     * @param uint32 event : [Player] gossip event Id, refer to GossipEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterPlayerGossipEvent(uint32 menu_id, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_PLAYER_GOSSIP, menu_id, event, std::move(callback), shots);
    }

    /**
     * Registers a [Creature] event handler.
     *
     * <pre>
     * enum CreatureEvents
     * {
     *     CREATURE_EVENT_ON_ENTER_COMBAT                       = 1,  // (event, creature, target) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_LEAVE_COMBAT                       = 2,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_TARGET_DIED                        = 3,  // (event, creature, victim) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DIED                               = 4,  // (event, creature, killer) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPAWN                              = 5,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_REACH_WP                           = 6,  // (event, creature, type, id) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_AIUPDATE                           = 7,  // (event, creature, diff) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RECEIVE_EMOTE                      = 8,  // (event, creature, player, emoteid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DAMAGE_TAKEN                       = 9,  // (event, creature, attacker, damage) - Can return true to stop normal action, can return new damage as second return value.
     *     CREATURE_EVENT_ON_PRE_COMBAT                         = 10, // (event, creature, target) - Can return true to stop normal action
     *     // UNUSED
     *     CREATURE_EVENT_ON_OWNER_ATTACKED                     = 12, // (event, creature, target) - Can return true to stop normal action            // Not on mangos
     *     CREATURE_EVENT_ON_OWNER_ATTACKED_AT                  = 13, // (event, creature, attacker) - Can return true to stop normal action          // Not on mangos
     *     CREATURE_EVENT_ON_HIT_BY_SPELL                       = 14, // (event, creature, caster, spellid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPELL_HIT_TARGET                   = 15, // (event, creature, target, spellid) - Can return true to stop normal action
     *     // UNUSED                                            = 16, // (event, creature)
     *     // UNUSED                                            = 17, // (event, creature)
     *     // UNUSED                                            = 18, // (event, creature)
     *     CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE             = 19, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN          = 20, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED             = 21, // (event, creature, summon, killer) - Can return true to stop normal action    // Not on mangos
     *     CREATURE_EVENT_ON_SUMMONED                           = 22, // (event, creature, summoner) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RESET                              = 23, // (event, creature)
     *     CREATURE_EVENT_ON_REACH_HOME                         = 24, // (event, creature) - Can return true to stop normal action
     *     // UNUSED                                            = 25, // (event, creature)
     *     CREATURE_EVENT_ON_CORPSE_REMOVED                     = 26, // (event, creature, respawndelay) - Can return true to stop normal action, can return new respawndelay as second return value
     *     CREATURE_EVENT_ON_MOVE_IN_LOS                        = 27, // (event, creature, unit) - Can return true to stop normal action. Does not actually check LOS, just uses the sight range
     *     // UNUSED                                            = 28, // (event, creature)
     *     // UNUSED                                            = 29, // (event, creature)
     *     CREATURE_EVENT_ON_DUMMY_EFFECT                       = 30, // (event, caster, spellid, effindex, creature)
     *     CREATURE_EVENT_ON_QUEST_ACCEPT                       = 31, // (event, player, creature, quest) - Can return true
     *     // UNUSED                                            = 32, // (event, creature)
     *     // UNUSED                                            = 33, // (event, creature)
     *     CREATURE_EVENT_ON_QUEST_REWARD                       = 34, // (event, player, creature, quest, opt) - Can return true
     *     CREATURE_EVENT_ON_DIALOG_STATUS                      = 35, // (event, player, creature)
     *     CREATURE_EVENT_ON_ADD                                = 36, // (event, creature)
     *     CREATURE_EVENT_ON_REMOVE                             = 37, // (event, creature)
     *     CREATURE_EVENT_ON_AURA_APPLY                         = 38, // (event, creature, aura)
     *     CREATURE_EVENT_ON_HEAL                               = 39, // (event, creature, target, heal) - Can return new heal amount
     *     CREATURE_EVENT_ON_DAMAGE                             = 40, // (event, creature, target, damage) - Can return new damage amount
     *     CREATURE_EVENT_ON_AURA_REMOVE                        = 41, // (event, creature, aura, remove_mode)
     *     CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK  = 42, // (event, creature, target, damage, spellInfo) - Can return new damage amount
     *     CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE                = 43, // (event, creature, target, damage) - Can return new damage amount
     *     CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN          = 44, // (event, creature, target, damage, spellInfo) - Can return new damage amount
     *     CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED               = 45, // (event, creature, target, heal, spellInfo) - Can return new heal amount
     *     CREATURE_EVENT_ON_DEAL_DAMAGE                        = 46, // (event, creature, target, damage, damagetype) - Can return new damage amount
     *     CREATURE_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : the ID of one or more [Creature]s
     * @param uint32 event : refer to CreatureEvents above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterCreatureEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_CREATURE, entry, event, std::move(callback), shots);
    }

    /**
     * Registers a [Creature] event handler for a *single* [Creature].
     *
     * <pre>
     * enum CreatureEvents
     * {
     *     CREATURE_EVENT_ON_ENTER_COMBAT                    = 1,  // (event, creature, target) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_LEAVE_COMBAT                    = 2,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_TARGET_DIED                     = 3,  // (event, creature, victim) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DIED                            = 4,  // (event, creature, killer) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPAWN                           = 5,  // (event, creature) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_REACH_WP                        = 6,  // (event, creature, type, id) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_AIUPDATE                        = 7,  // (event, creature, diff) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RECEIVE_EMOTE                   = 8,  // (event, creature, player, emoteid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_DAMAGE_TAKEN                    = 9,  // (event, creature, attacker, damage) - Can return true to stop normal action, can return new damage as second return value.
     *     CREATURE_EVENT_ON_PRE_COMBAT                      = 10, // (event, creature, target) - Can return true to stop normal action
     *     // UNUSED
     *     CREATURE_EVENT_ON_OWNER_ATTACKED                  = 12, // (event, creature, target) - Can return true to stop normal action            // Not on mangos
     *     CREATURE_EVENT_ON_OWNER_ATTACKED_AT               = 13, // (event, creature, attacker) - Can return true to stop normal action          // Not on mangos
     *     CREATURE_EVENT_ON_HIT_BY_SPELL                    = 14, // (event, creature, caster, spellid) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SPELL_HIT_TARGET                = 15, // (event, creature, target, spellid) - Can return true to stop normal action
     *     // UNUSED                                         = 16, // (event, creature)
     *     // UNUSED                                         = 17, // (event, creature)
     *     // UNUSED                                         = 18, // (event, creature)
     *     CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE          = 19, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN       = 20, // (event, creature, summon) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED          = 21, // (event, creature, summon, killer) - Can return true to stop normal action    // Not on mangos
     *     CREATURE_EVENT_ON_SUMMONED                        = 22, // (event, creature, summoner) - Can return true to stop normal action
     *     CREATURE_EVENT_ON_RESET                           = 23, // (event, creature)
     *     CREATURE_EVENT_ON_REACH_HOME                      = 24, // (event, creature) - Can return true to stop normal action
     *     // UNUSED                                         = 25, // (event, creature)
     *     CREATURE_EVENT_ON_CORPSE_REMOVED                  = 26, // (event, creature, respawndelay) - Can return true to stop normal action, can return new respawndelay as second return value
     *     CREATURE_EVENT_ON_MOVE_IN_LOS                     = 27, // (event, creature, unit) - Can return true to stop normal action. Does not actually check LOS, just uses the sight range
     *     // UNUSED                                         = 28, // (event, creature)
     *     // UNUSED                                         = 29, // (event, creature)
     *     CREATURE_EVENT_ON_DUMMY_EFFECT                    = 30, // (event, caster, spellid, effindex, creature)
     *     CREATURE_EVENT_ON_QUEST_ACCEPT                    = 31, // (event, player, creature, quest) - Can return true
     *     // UNUSED                                         = 32, // (event, creature)
     *     // UNUSED                                         = 33, // (event, creature)
     *     CREATURE_EVENT_ON_QUEST_REWARD                    = 34, // (event, player, creature, quest, opt) - Can return true
     *     CREATURE_EVENT_ON_DIALOG_STATUS                   = 35, // (event, player, creature)
     *     CREATURE_EVENT_ON_ADD                             = 36, // (event, creature)
     *     CREATURE_EVENT_ON_REMOVE                          = 37, // (event, creature)
     *     CREATURE_EVENT_ON_AURA_APPLY                      = 38, // (event, creature, aura)
     *     CREATURE_EVENT_ON_HEAL                            = 39, // (event, creature, target, gain) - Can return new heal amount
     *     CREATURE_EVENT_ON_DAMAGE                          = 40, // (event, creature, target, damage) - Can return new damage amount
     *     CREATURE_EVENT_ON_AURA_REMOVE                     = 41, // (event, creature, aura, remove_mode)
     *     CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK = 42, // (event, creature, target, damage, spellInfo) - Can return new damage amount
     *     CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE            = 43, // (event, creature, target, damage) - Can return new damage amount
     *     CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN      = 44, // (event, creature, target, damage, spellInfo) - Can return new damage amount
     *     CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED           = 45, // (event, creature, target, heal, spellInfo) - Can return new heal amount
     *     CREATURE_EVENT_ON_DEAL_DAMAGE                    = 46, // (event, creature, target, damage, damagetype) - Can return new damage amount
     *     CREATURE_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (guid, instance_id, event, function)
     * @proto cancel = (guid, instance_id, event, function, shots)
     *
     * @param ObjectGuid guid : the GUID of a single [Creature]
     * @param uint32 instance_id : the instance ID of a single [Creature]
     * @param uint32 event : refer to CreatureEvents above
     * @param function function : function that will be called when the event occurs
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterUniqueCreatureEvent(ObjectGuid guid, uint32 instance_id, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterUniqueHelper(Hooks::REGTYPE_CREATURE, guid, instance_id, event, std::move(callback), shots);
    }

    /**
     * Registers a [GameObject] event handler.
     *
     * <pre>
     * enum GameObjectEvents
     * {
     *     GAMEOBJECT_EVENT_ON_AIUPDATE                    = 1,    // (event, go, diff)
     *     GAMEOBJECT_EVENT_ON_SPAWN                       = 2,    // (event, go)
     *     GAMEOBJECT_EVENT_ON_DUMMY_EFFECT                = 3,    // (event, caster, spellid, effindex, go) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_ON_QUEST_ACCEPT                = 4,    // (event, player, go, quest) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_ON_QUEST_REWARD                = 5,    // (event, player, go, quest, opt) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_ON_DIALOG_STATUS               = 6,    // (event, player, go)
     *     GAMEOBJECT_EVENT_ON_DESTROYED                   = 7,    // (event, go, attacker)
     *     GAMEOBJECT_EVENT_ON_DAMAGED                     = 8,    // (event, go, attacker)
     *     GAMEOBJECT_EVENT_ON_LOOT_STATE_CHANGE           = 9,    // (event, go, state)
     *     GAMEOBJECT_EVENT_ON_GO_STATE_CHANGED            = 10,   // (event, go, state)
     *     // UNUSED                                       = 11,   // (event, gameobject)
     *     GAMEOBJECT_EVENT_ON_ADD                         = 12,   // (event, gameobject)
     *     GAMEOBJECT_EVENT_ON_REMOVE                      = 13,   // (event, gameobject)
     *     GAMEOBJECT_EVENT_ON_USE                         = 14,   // (event, go, player) - Can return true to stop normal action
     *     GAMEOBJECT_EVENT_COUNT
     * };
     * </pre>
     *
     * @proto cancel = (entry, event, function)
     * @proto cancel = (entry, event, function, shots)
     *
     * @param uint32 entry : [GameObject] entry Id
     * @param uint32 event : [GameObject] event Id, refer to GameObjectEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     *
     * @return function cancel : a function that cancels the binding when called
     */
    sol::object RegisterGameObjectEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_GAMEOBJECT, entry, event, std::move(callback), shots);
    }

    /**
     * Registers a [Ticket] event handler.
     *
     * <pre>
     * enum TicketEvents
     * {
     *     TICKET_EVENT_ON_CREATE                          = 1,    // (event, player, ticket)
     *     TICKET_EVENT_ON_UPDATE                          = 2,    // (event, player, ticket, message)
     *     TICKET_EVENT_ON_CLOSE                           = 3,    // (event, player, ticket)
     *     TICKET_EVENT_STATUS_UPDATE                      = 4,    // (event, player, ticket)
     *     TICKET_EVENT_ON_RESOLVE                         = 5,    // (event, player, ticket)
     *     TICKET_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 event : event ID, refer to UnitEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    sol::object RegisterTicketEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_TICKET, event, std::move(callback), shots);
    }

    /**
     * Registers a [Spell] event handler.
     *
     * <pre>
     * enum SpellEvents
     * {
     *     SPELL_EVENT_ON_PREPARE                          = 1, // (event, caster, spell)
     *     SPELL_EVENT_ON_CAST                             = 2, // (event, caster, spell, skipCheck)
     *     SPELL_EVENT_ON_CAST_CANCEL                      = 3, // (event, caster, spell, bySelf)
     *     SPELL_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 entry : [Spell] entry Id
     * @param uint32 event : event ID, refer to SpellEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    sol::object RegisterSpellEvent(uint32 entry, uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEntryHelper(Hooks::REGTYPE_SPELL, entry, event, std::move(callback), shots);
    }

    /**
     * Registers a [Creature] event handler. It used AllCreatureScript so this don't need creature entry as a key.
     *
     * <pre>
     * enum AllCreatureEvents
     * {
     *     ALL_CREATURE_EVENT_ON_ADD                               = 1, // (event, creature)
     *     ALL_CREATURE_EVENT_ON_REMOVE                            = 2, // (event, creature)
     *     ALL_CREATURE_EVENT_ON_SELECT_LEVEL                      = 3, // (event, creature_template, creature)
     *     ALL_CREATURE_EVENT_ON_BEFORE_SELECT_LEVEL               = 4, // (event, creature_template, creature, level) - Can return the new level
     *     ALL_CREATURE_EVENT_ON_AURA_APPLY                        = 5, // (event, creature, aura)
     *     ALL_CREATURE_EVENT_ON_HEAL                              = 6, // (event, creature, target, gain) - Can return new heal amount
     *     ALL_CREATURE_EVENT_ON_DAMAGE                            = 7, // (event, creature, target, damage) - Can return new damage amount
     *     ALL_CREATURE_EVENT_ON_AURA_REMOVE                       = 8, // (event, creature, aura, remove_mode)
     *     ALL_CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK = 9, // (event, creature, target, damage, spellInfo) - Can return new damage amount
     *     ALL_CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE               = 10, // (event, creature, target, damage) - Can return new damage amount
     *     ALL_CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN         = 11, // (event, creature, target, damage, spellInfo) - Can return new damage amount
     *     ALL_CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED              = 12, // (event, creature, target, heal, spellInfo) - Can return new heal amount
     *     ALL_CREATURE_EVENT_ON_DEAL_DAMAGE                       = 13, // (event, creature, target, damage, damagetype) - Can return new damage amount
     *     ALL_CREATURE_EVENT_COUNT
     * };
     * </pre>
     *
     * @param uint32 event : event ID, refer to AllCreatureEvents above
     * @param function function : function to register
     * @param uint32 shots = 0 : the number of times the function will be called, 0 means "always call this function"
     */
    sol::object RegisterAllCreatureEvent(uint32 event, sol::protected_function callback, sol::optional<uint32> shots)
    {
        return RegisterEventHelper(Hooks::REGTYPE_ALL_CREATURE, event, std::move(callback), shots);
    }

    /**
     * Reloads the Lua engine.
     */
    void ReloadALE()
    {
        ALE::ReloadALE();
    }

    static void PrintRunCommandOutput(void* /*callbackArg*/, std::string_view view)
    {
        std::string str = { view.begin(), view.end() };
        // Remove trailing spaces and line breaks
        while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back())))
            str.pop_back();
        ALE_LOG_INFO("{}", str);
    }

    /**
     * Runs a command.
     *
     * @param string command : the command to run
     */
    void RunCommand(std::string command)
    {
        sWorld->QueueCliCommand(new CliCommandHolder(nullptr, command.c_str(), &PrintRunCommandOutput, nullptr));
    }

    /**
     * Sends a message to all [Player]s online.
     *
     * @param string message : message to send
     */
    void SendWorldMessage(std::string message)
    {
        sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, message);
    }

    static void ForwardAsyncQueryResult(sol::protected_function callback, QueryResult result)
    {
        LOCK_ALE;

        sALE->CallFunction(callback, result);
    }

    template <typename T>
    static void DBQueryAsync(DatabaseWorkerPool<T>& db, std::string const& query, sol::protected_function callback)
    {
        sALE->queryProcessor.AddCallback(db.AsyncQuery(query.c_str())
            .WithCallback(std::bind(&ForwardAsyncQueryResult, std::move(callback), std::placeholders::_1)));
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
    QueryResult WorldDBQuery(std::string query)
    {
        return WorldDatabase.Query(query.c_str());
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
    void WorldDBQueryAsync(std::string query, sol::protected_function callback)
    {
        DBQueryAsync(WorldDatabase, query, std::move(callback));
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
    void WorldDBExecute(std::string query)
    {
        WorldDatabase.Execute(query.c_str());
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
    QueryResult CharDBQuery(std::string query)
    {
        return CharacterDatabase.Query(query.c_str());
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
    void CharDBQueryAsync(std::string query, sol::protected_function callback)
    {
        DBQueryAsync(CharacterDatabase, query, std::move(callback));
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
    void CharDBExecute(std::string query)
    {
        CharacterDatabase.Execute(query.c_str());
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
    QueryResult AuthDBQuery(std::string query)
    {
        return LoginDatabase.Query(query.c_str());
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
    void AuthDBQueryAsync(std::string query, sol::protected_function callback)
    {
        DBQueryAsync(LoginDatabase, query, std::move(callback));
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
    void AuthDBExecute(std::string query)
    {
        LoginDatabase.Execute(query.c_str());
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
    uint64 CreateLuaEvent(sol::protected_function callback, sol::object delay, sol::optional<uint32> repeatsArg)
    {
        uint32 min, max;
        if (delay.is<sol::table>())
        {
            sol::table delayTable = delay.as<sol::table>();
            min = delayTable.get<uint32>(1);
            max = delayTable.get<uint32>(2);
        }
        else
            min = max = delay.as<uint32>();
        uint32 repeats = repeatsArg.value_or(1);

        if (min > max)
            throw std::invalid_argument("min is bigger than max delay");

        return sALE->eventMgr->globalProcessor->AddEvent(std::move(callback), min, max, repeats);
    }

    /**
     * Removes a global timed event specified by ID.
     *
     * @param int eventId : event Id to remove
     * @param bool all_Events = false : remove from all events, not just global
     */
    void RemoveEventById(uint64 eventId, sol::optional<bool> allEventsArg)
    {
        bool all_Events = allEventsArg.value_or(false);

        // not thread safe
        if (all_Events)
            sALE->eventMgr->SetState(eventId, LUAEVENT_STATE_ABORT);
        else
            sALE->eventMgr->globalProcessor->SetState(eventId, LUAEVENT_STATE_ABORT);
    }

    /**
     * Removes all global timed events.
     *
     * @param bool all_Events = false : remove all events, not just global
     */
    void RemoveEvents(sol::optional<bool> allEventsArg)
    {
        bool all_Events = allEventsArg.value_or(false);

        // not thread safe
        if (all_Events)
            sALE->eventMgr->SetStates(LUAEVENT_STATE_ABORT);
        else
            sALE->eventMgr->globalProcessor->SetStates(LUAEVENT_STATE_ABORT);
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
    WorldObject* PerformIngameSpawn(int32 spawntype, uint32 entry, uint32 mapID, uint32 instanceID,
        float x, float y, float z, float o,
        sol::optional<bool> saveArg, sol::optional<uint32> durorresptimeArg, sol::optional<uint32> phaseArg)
    {
        bool save = saveArg.value_or(false);
        uint32 durorresptime = durorresptimeArg.value_or(0);
        uint32 phase = phaseArg.value_or(PHASEMASK_NORMAL);

        if (!phase)
            return nullptr;

        Map* map = sMapMgr->FindMap(mapID, instanceID);
        if (!map)
            return nullptr;

        Position pos = { x, y, z, o };

        if (spawntype == 1) // spawn creature
        {
            if (save)
            {
                Creature* creature = new Creature();
                if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, phase, entry, 0, x, y, z, o))
                {
                    delete creature;
                    return nullptr;
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
                    return nullptr;
                }

                sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
                return creature;
            }
            else
            {
                TempSummon* creature = map->SummonCreature(entry, pos, nullptr, durorresptime);
                if (!creature)
                    return nullptr;

                if (durorresptime)
                    creature->SetTempSummonType(TEMPSUMMON_TIMED_OR_DEAD_DESPAWN);
                else
                    creature->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);

                return creature;
            }
        }

        if (spawntype == 2) // Spawn object
        {
            const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(entry);
            if (!objectInfo)
                return nullptr;

            if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
                return nullptr;

            GameObject* object = new GameObject;
            uint32 guidLow = map->GenerateLowGuid<HighGuid::GameObject>();

            if (!object->Create(guidLow, entry, map, phase, x, y, z, o, G3D::Quat(0.0f, 0.0f, 0.0f, 0.0f), 100, GO_STATE_READY))
            {
                delete object;
                return nullptr;
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
                    return nullptr;
                }

                sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));
            }
            else
                map->AddToMap(object);
            return object;
        }
        return nullptr;
    }

    /**
     * Creates a [WorldPacket].
     *
     * @param [Opcodes] opcode : the opcode of the packet
     * @param uint32 size : the size of the packet
     * @return [WorldPacket] packet
     */
    WorldPacket CreatePacket(uint32 opcode, size_t size)
    {
        if (opcode >= NUM_MSG_TYPES)
            throw std::invalid_argument("valid opcode expected");

        return WorldPacket(static_cast<Opcodes>(opcode), size);
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
    void AddVendorItem(uint32 entry, uint32 item, int32 maxcount, uint32 incrtime, uint32 extendedcost)
    {
        if (!sObjectMgr->IsVendorItemValid(entry, item, maxcount, incrtime, extendedcost))
            return;
        sObjectMgr->AddVendorItem(entry, item, maxcount, incrtime, extendedcost);
    }

    /**
     * Removes an [Item] from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     * @param uint32 item : [Item] entry Id
     */
    void VendorRemoveItem(uint32 entry, uint32 item)
    {
        if (!sObjectMgr->GetCreatureTemplate(entry))
            throw std::invalid_argument("valid CreatureEntry expected");

        sObjectMgr->RemoveVendorItem(entry, item);
    }

    /**
     * Removes all [Item]s from a vendor and updates the database.
     *
     * @param uint32 entry : [Creature] entry Id
     */
    void VendorRemoveAllItems(uint32 entry)
    {
        VendorItemData const* items = sObjectMgr->GetNpcVendorItemList(entry);
        if (!items || items->Empty())
            return;

        auto const& itemlist = items->m_items;
        for (auto itr = itemlist.rbegin(); itr != itemlist.rend(); ++itr)
            sObjectMgr->RemoveVendorItem(entry, (*itr)->item);
    }

    /**
     * Kicks a [Player] from the server.
     *
     * @param [Player] player : [Player] to kick
     */
    void Kick(Player* player)
    {
        player->GetSession()->KickPlayer();
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
    sol::optional<int32> Ban(int32 banMode, std::string nameOrIP, uint32 duration, sol::optional<std::string> reasonArg, sol::optional<std::string> whoBannedArg)
    {
        std::string reason = reasonArg.value_or("");
        std::string whoBanned = whoBannedArg.value_or("");

        const int BAN_ACCOUNT = 0;
        const int BAN_CHARACTER = 1;
        const int BAN_IP = 2;

        switch (banMode)
        {
            case BAN_ACCOUNT:
                if (!Utf8ToUpperOnlyLatin(nameOrIP))
                    throw std::invalid_argument("invalid account name");
                break;
            case BAN_CHARACTER:
                if (!normalizePlayerName(nameOrIP))
                    throw std::invalid_argument("invalid character name");
                break;
            case BAN_IP:
                if (!IsIPAddress(nameOrIP.c_str()))
                    throw std::invalid_argument("invalid ip");
                break;
            default:
                throw std::invalid_argument("unknown banmode");
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
            return 0;
        case BanReturn::BAN_SYNTAX_ERROR:
            return 1;
        case BanReturn::BAN_NOTFOUND:
            return 2;
        case BanReturn::BAN_LONGER_EXISTS:
            return 3;
        }
        return sol::nullopt;
    }

    /**
     * Saves all [Player]s.
     */
    void SaveAllPlayers()
    {
        ObjectAccessor::SaveAllPlayers();
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
    sol::variadic_results SendMail(std::string subject, std::string text, uint32 receiverGUIDLow,
        sol::optional<uint32> senderGUIDLowArg, sol::optional<uint32> stationaryArg, sol::optional<uint32> delayArg,
        sol::optional<uint32> moneyArg, sol::optional<uint32> codArg, sol::this_state s, sol::variadic_args itemArgs)
    {
        uint32 senderGUIDLow = senderGUIDLowArg.value_or(0);
        uint32 stationary = stationaryArg.value_or(MAIL_STATIONERY_DEFAULT);
        uint32 delay = delayArg.value_or(0);
        uint32 money = moneyArg.value_or(0);
        uint32 cod = codArg.value_or(0);

        MailSender sender(MAIL_NORMAL, senderGUIDLow, (MailStationery)stationary);
        MailDraft draft(subject, text);

        if (cod)
            draft.AddCOD(cod);
        if (money)
            draft.AddMoney(money);

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        sol::variadic_results results;
        uint8 addedItems = 0;
        std::size_t argIndex = 0;
        while (addedItems <= MAX_MAIL_ITEMS && argIndex + 2 <= itemArgs.size())
        {
            uint32 entry = itemArgs.get<uint32>(argIndex++);
            uint32 amount = itemArgs.get<uint32>(argIndex++);

            ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(entry);
            if (!item_proto)
                throw std::runtime_error(Acore::StringFormat("Item entry {} does not exist", entry));
            if (amount < 1 || (item_proto->MaxCount > 0 && amount > uint32(item_proto->MaxCount)))
                throw std::runtime_error(Acore::StringFormat("Item entry {} has invalid amount {}", entry, amount));
            if (Item* item = Item::CreateItem(entry, amount))
            {
                item->SaveToDB(trans);
                draft.AddItem(item);
                results.push_back(sol::make_object(s, item->GetGUID().GetCounter()));
                ++addedItems;
            }
        }

        Player* receiverPlayer = ObjectAccessor::FindPlayer(ObjectGuid(HighGuid::Player, 0, receiverGUIDLow));
        draft.SendMailTo(trans, MailReceiver(receiverPlayer, receiverGUIDLow), sender, MAIL_CHECK_MASK_NONE, delay);
        CharacterDatabase.CommitTransaction(trans);
        return results;
    }

    /**
     * Performs a bitwise AND (a & b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    uint32 bit_and(uint32 a, uint32 b)
    {
        return a & b;
    }

    /**
     * Performs a bitwise OR (a | b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    uint32 bit_or(uint32 a, uint32 b)
    {
        return a | b;
    }

    /**
     * Performs a bitwise left-shift (a << b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    uint32 bit_lshift(uint32 a, uint32 b)
    {
        return a << b;
    }

    /**
     * Performs a bitwise right-shift (a >> b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    uint32 bit_rshift(uint32 a, uint32 b)
    {
        return a >> b;
    }

    /**
     * Performs a bitwise XOR (a ^ b).
     *
     * @param uint32 a
     * @param uint32 b
     * @return uint32 result
     */
    uint32 bit_xor(uint32 a, uint32 b)
    {
        return a ^ b;
    }

    /**
     * Performs a bitwise NOT (~a).
     *
     * @param uint32 a
     * @return uint32 result
     */
    uint32 bit_not(uint32 a)
    {
        return ~a;
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
    uint32 AddTaxiPath(sol::table waypoints, uint32 mountA, uint32 mountH, sol::optional<uint32> priceArg, sol::optional<uint32> pathIdArg)
    {
        uint32 price = priceArg.value_or(0);
        uint32 pathId = pathIdArg.value_or(0);

        std::list<TaxiPathNodeEntry> nodes;

        std::size_t nodeCount = waypoints.size();
        for (std::size_t i = 1; i <= nodeCount; ++i)
        {
            sol::optional<sol::table> node = waypoints.get<sol::optional<sol::table>>(i);
            if (!node)
                throw std::invalid_argument("table expected as waypoint");

            if (node->size() < 4) // no mandatory args, dont add
                throw std::invalid_argument("all waypoints do not have mandatory arguments");

            TaxiPathNodeEntry entry;
            // mandatory
            entry.mapid = node->get<uint32>(1);
            entry.x = node->get<float>(2);
            entry.y = node->get<float>(3);
            entry.z = node->get<float>(4);
            // optional
            entry.actionFlag = node->get_or<uint32>(5, 0);
            entry.delay = node->get_or<uint32>(6, 0);

            nodes.push_back(entry);
        }

        if (nodes.size() < 2)
            return pathId;
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
            return pathId;

        TaxiPathEntry* pathEntry = new TaxiPathEntry();
        pathEntry->from = startNode;
        pathEntry->to = nodeId - 1;
        pathEntry->price = price;
        pathEntry->ID = pathId;
        sTaxiPathStore.SetEntry(pathId, pathEntry);
        sTaxiPathSetBySource[startNode][nodeId - 1] = pathEntry;

        return pathId;
    }

    /**
     * Returns `true` if ALE is in compatibility mode, `false` if in multistate.
     *
     * @return bool isCompatibilityMode
     */
    bool IsCompatibilityMode()
    {
        // Until AC supports multistate, this will always return true
        return true;
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
    bool IsInventoryPos(uint8 bag, uint8 slot)
    {
        return Player::IsInventoryPos(bag, slot);
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
    bool IsEquipmentPos(uint8 bag, uint8 slot)
    {
        return Player::IsEquipmentPos(bag, slot);
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
    bool IsBankPos(uint8 bag, uint8 slot)
    {
        return Player::IsBankPos(bag, slot);
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
    bool IsBagPos(uint8 bag, uint8 slot)
    {
        return Player::IsBagPos((bag << 8) + slot);
    }

    /**
     * Returns `true` if the event is currently active, otherwise `false`.
     *
     * @param uint16 eventId : the event id to check.
     * @return bool isActive
     */
    bool IsGameEventActive(uint16 eventId)
    {
        return sGameEventMgr->IsActiveEvent(eventId);
    }

    /**
     * Returns the server's current time.
     *
     * @return uint32 currTime : the current time, in milliseconds
     */
    uint32 GetCurrTime()
    {
        return ALEUtil::GetCurrTime();
    }

    /**
     * Returns the difference between an old timestamp and the current time.
     *
     * @param uint32 oldTime : an old timestamp, in milliseconds
     * @return uint32 timeDiff : the difference, in milliseconds
     */
    uint32 GetTimeDiff(uint32 oldtimems)
    {
        return ALEUtil::GetTimeDiff(oldtimems);
    }

    static std::string BuildLogString(sol::variadic_args const& args)
    {
        sol::state_view lua(args.lua_state());
        sol::protected_function tostring = lua["tostring"];

        std::ostringstream oss;
        for (sol::stack_proxy arg : args)
            oss << tostring(arg).get<std::string>();

        return oss.str();
    }

    /**
     * Prints given parameters to the info log.
     *
     * @param ...
     */
    void PrintInfo(sol::variadic_args args)
    {
        ALE_LOG_INFO("{}", BuildLogString(args));
    }

    /**
     * Prints given parameters to the error log.
     *
     * @param ...
     */
    void PrintError(sol::variadic_args args)
    {
        ALE_LOG_ERROR("{}", BuildLogString(args));
    }

    /**
     * Prints given parameters to the debug log.
     *
     * @param ...
     */
    void PrintDebug(sol::variadic_args args)
    {
        ALE_LOG_DEBUG("{}", BuildLogString(args));
    }

    /**
    * Starts the event by eventId, if force is set, the event will force start regardless of previous event state.
    *
    * @param uint16 eventId : the event id to start.
    * @param bool force = false : set `true` to force start the event.
    */
    void StartGameEvent(uint16 eventId, sol::optional<bool> forceArg)
    {
        bool force = forceArg.value_or(false);

        sGameEventMgr->StartEvent(eventId, force);
    }

    /**
    * Stops the event by eventId, if force is set, the event will force stop regardless of previous event state.
    *
    * @param uint16 eventId : the event id to stop.
    * @param bool force = false : set `true` to force stop the event.
    */
    void StopGameEvent(uint16 eventId, sol::optional<bool> forceArg)
    {
        bool force = forceArg.value_or(false);

        sGameEventMgr->StopEvent(eventId, force);
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
    void HttpRequest(std::string httpVerb, std::string url, sol::object arg3,
        sol::optional<sol::object> arg4, sol::optional<sol::object> arg5, sol::optional<sol::object> arg6)
    {
        std::string body;
        std::string bodyContentType;
        httplib::Headers headers;

        sol::object headersOrCallback = arg3;
        sol::object callbackAfterHeaders = arg4.value_or(sol::object());

        if (arg3.get_type() == sol::type::string && arg4 && arg4->get_type() == sol::type::string)
        {
            body = arg3.as<std::string>();
            bodyContentType = arg4->as<std::string>();
            headersOrCallback = arg5.value_or(sol::object());
            callbackAfterHeaders = arg6.value_or(sol::object());
        }

        sol::object callbackObject = headersOrCallback;
        if (headersOrCallback.get_type() == sol::type::table)
        {
            sol::table headerTable = headersOrCallback.as<sol::table>();
            for (auto const& pair : headerTable)
            {
                if (pair.first.get_type() == sol::type::string
                    && (pair.second.get_type() == sol::type::string || pair.second.get_type() == sol::type::number))
                    headers.insert(std::pair<std::string, std::string>(pair.first.as<std::string>(), pair.second.as<std::string>()));
            }

            callbackObject = callbackAfterHeaders;
        }

        if (callbackObject.get_type() != sol::type::function)
            throw std::invalid_argument("function expected for the HTTP request callback");

        sALE->httpManager.PushRequest(new HttpWorkItem(callbackObject.as<sol::protected_function>(), httpVerb, url, body, bodyContentType, headers));
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
    int64 CreateLongLong(sol::optional<sol::object> value)
    {
        long long init = 0;
        if (value && value->get_type() == sol::type::string)
        {
            std::string str = value->as<std::string>();
            std::istringstream iss(str);
            iss >> init;
            if (iss.bad())
                throw std::invalid_argument("long long (as string) could not be converted");
        }
        else if (value && value->get_type() != sol::type::lua_nil)
            init = value->as<long long>();

        return init;
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
    uint64 CreateULongLong(sol::optional<sol::object> value)
    {
        unsigned long long init = 0;
        if (value && value->get_type() == sol::type::string)
        {
            std::string str = value->as<std::string>();
            std::istringstream iss(str);
            iss >> init;
            if (iss.bad())
                throw std::invalid_argument("unsigned long long (as string) could not be converted");
        }
        else if (value && value->get_type() != sol::type::lua_nil)
            init = value->as<unsigned long long>();

        return init;
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
    void ClearBattleGroundEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::BGEvents> Key;

        if (!eventTypeArg)
        {
            sALE->BGEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->BGEventBindings->Clear(Key((Hooks::BGEvents)event_type));
        }
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
    void ClearCreatureEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::CreatureEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
                sALE->CreatureEventBindings->Clear(Key((Hooks::CreatureEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->CreatureEventBindings->Clear(Key((Hooks::CreatureEvents)event_type, entry));
        }
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
    void ClearUniqueCreatureEvents(ObjectGuid guid, uint32 instanceId, sol::optional<uint32> eventTypeArg)
    {
        typedef UniqueObjectKey<Hooks::CreatureEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
                sALE->CreatureUniqueBindings->Clear(Key((Hooks::CreatureEvents)i, guid, instanceId));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->CreatureUniqueBindings->Clear(Key((Hooks::CreatureEvents)event_type, guid, instanceId));
        }
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
    void ClearCreatureGossipEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                sALE->CreatureGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->CreatureGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
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
    void ClearGameObjectEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::GameObjectEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::GAMEOBJECT_EVENT_COUNT; ++i)
                sALE->GameObjectEventBindings->Clear(Key((Hooks::GameObjectEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->GameObjectEventBindings->Clear(Key((Hooks::GameObjectEvents)event_type, entry));
        }
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
    void ClearGameObjectGossipEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                sALE->GameObjectGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->GameObjectGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
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
    void ClearGroupEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::GroupEvents> Key;

        if (!eventTypeArg)
        {
            sALE->GroupEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->GroupEventBindings->Clear(Key((Hooks::GroupEvents)event_type));
        }
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
    void ClearGuildEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::GuildEvents> Key;

        if (!eventTypeArg)
        {
            sALE->GuildEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->GuildEventBindings->Clear(Key((Hooks::GuildEvents)event_type));
        }
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
    void ClearItemEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::ItemEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::ITEM_EVENT_COUNT; ++i)
                sALE->ItemEventBindings->Clear(Key((Hooks::ItemEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->ItemEventBindings->Clear(Key((Hooks::ItemEvents)event_type, entry));
        }
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
    void ClearItemGossipEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                sALE->ItemGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->ItemGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
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
    void ClearPacketEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::PacketEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::PACKET_EVENT_COUNT; ++i)
                sALE->PacketEventBindings->Clear(Key((Hooks::PacketEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->PacketEventBindings->Clear(Key((Hooks::PacketEvents)event_type, entry));
        }
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
    void ClearPlayerEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::PlayerEvents> Key;

        if (!eventTypeArg)
        {
            sALE->PlayerEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->PlayerEventBindings->Clear(Key((Hooks::PlayerEvents)event_type));
        }
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
    void ClearPlayerGossipEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::GossipEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::GOSSIP_EVENT_COUNT; ++i)
                sALE->PlayerGossipBindings->Clear(Key((Hooks::GossipEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->PlayerGossipBindings->Clear(Key((Hooks::GossipEvents)event_type, entry));
        }
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
    void ClearServerEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::ServerEvents> Key;

        if (!eventTypeArg)
        {
            sALE->ServerEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->ServerEventBindings->Clear(Key((Hooks::ServerEvents)event_type));
        }
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
    void ClearMapEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::InstanceEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
                sALE->MapEventBindings->Clear(Key((Hooks::InstanceEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->MapEventBindings->Clear(Key((Hooks::InstanceEvents)event_type, entry));
        }
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
    void ClearInstanceEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::InstanceEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
                sALE->InstanceEventBindings->Clear(Key((Hooks::InstanceEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->InstanceEventBindings->Clear(Key((Hooks::InstanceEvents)event_type, entry));
        }
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
    void ClearTicketEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::TicketEvents> Key;

        if (!eventTypeArg)
        {
            sALE->TicketEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->TicketEventBindings->Clear(Key((Hooks::TicketEvents)event_type));
        }
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
    void ClearSpellEvents(uint32 entry, sol::optional<uint32> eventTypeArg)
    {
        typedef EntryKey<Hooks::SpellEvents> Key;

        if (!eventTypeArg)
        {
            for (uint32 i = 1; i < Hooks::SPELL_EVENT_COUNT; ++i)
                sALE->SpellEventBindings->Clear(Key((Hooks::SpellEvents)i, entry));
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->SpellEventBindings->Clear(Key((Hooks::SpellEvents)event_type, entry));
        }
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
    void ClearAllCreatureEvents(sol::optional<uint32> eventTypeArg)
    {
        typedef EventKey<Hooks::AllCreatureEvents> Key;

        if (!eventTypeArg)
        {
            sALE->AllCreatureEventBindings->Clear();
        }
        else
        {
            uint32 event_type = *eventTypeArg;
            sALE->AllCreatureEventBindings->Clear(Key((Hooks::AllCreatureEvents)event_type));
        }
    }

    /**
     * Gets the faction which is the current owner of Halaa in Nagrand
     * 0 = Alliance
     * 1 = Horde
     *
     * 600 = slider max Alliance
     * -600 = slider max Horde
     *
     * @return int16 the ID of the team to own Halaa
     * @return float the slider position.
     */
    std::tuple<TeamId, float> GetOwnerHalaa()
    {
        OutdoorPvPNA* nagrandPvp = (OutdoorPvPNA*)sOutdoorPvPMgr->GetOutdoorPvPToZoneId(3518);
        OPvPCapturePointNA* halaa = nagrandPvp->GetCapturePoint();

        return std::tuple<TeamId, float>(halaa->GetControllingFaction(), halaa->GetSlider());
    }

    /**
     * Sets the owner of Halaa in Nagrand to the respective faction
     * 0 = Alliance
     * 1 = Horde
     *
     * @param uint16 teamId : the ID of the team to own Halaa
     */
    void SetOwnerHalaa(uint16 teamId)
    {
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
            throw std::invalid_argument("0 for Alliance or 1 for Horde expected");
        }
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
    std::tuple<std::string, std::string> GetGossipMenuOptionLocale(uint32 menuId, uint32 optionId, uint8 locale)
    {
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

        return std::tuple<std::string, std::string>(strOptionText, strBoxText);
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
    sol::optional<std::tuple<float, float, float, float>> GetMapEntrance(uint32 mapId)
    {
        AreaTriggerTeleport const* at = sObjectMgr->GetMapEntranceTrigger(mapId);

        if (!at)
            return sol::nullopt;

        return std::tuple<float, float, float, float>(at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
    }

    /**
     * Get the [SpellInfo] for the specified [Spell] id
     *
     * @param uint32 spellId : the ID of the spell
     * @return [SpellInfo] spellInfo
     */
    SpellInfo* GetSpellInfo(uint32 spellId)
    {
        return const_cast<SpellInfo*>(sSpellMgr->GetSpellInfo(spellId));
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
    sol::object LookupEntry(std::string dbcName, uint32 id, sol::this_state s)
    {
        sol::state_view lua(s);

        for (const auto& dbc : dbcRegistry)
        {
            if (dbc.name == dbcName)
            {
                const void* entry = dbc.lookupFunction(id);
                if (!entry)
                    return sol::make_object(lua, sol::lua_nil);

                return dbc.makeObject(lua, entry);
            }
        }

        throw std::runtime_error(Acore::StringFormat("Invalid DBC name: {}", dbcName));
    }
}

void RegisterGlobalMethods(sol::state& lua)
{
    lua["GetLuaEngine"]                 = ALEBind::Function(&LuaGlobalFunctions::GetLuaEngine);
    lua["GetCoreName"]                  = ALEBind::Function(&LuaGlobalFunctions::GetCoreName);
    lua["GetConfigValue"]               = ALEBind::Function(&LuaGlobalFunctions::GetConfigValue);
    lua["GetRealmID"]                   = ALEBind::Function(&LuaGlobalFunctions::GetRealmID);
    lua["GetCoreVersion"]               = ALEBind::Function(&LuaGlobalFunctions::GetCoreVersion);
    lua["GetCoreExpansion"]             = ALEBind::Function(&LuaGlobalFunctions::GetCoreExpansion);
    lua["GetStateMap"]                  = ALEBind::Function(&LuaGlobalFunctions::GetStateMap);
    lua["GetStateMapId"]                = ALEBind::Function(&LuaGlobalFunctions::GetStateMapId);
    lua["GetStateInstanceId"]           = ALEBind::Function(&LuaGlobalFunctions::GetStateInstanceId);
    lua["GetQuest"]                     = ALEBind::Function(&LuaGlobalFunctions::GetQuest);
    lua["GetPlayerByGUID"]              = ALEBind::Function(&LuaGlobalFunctions::GetPlayerByGUID);
    lua["GetPlayerByName"]              = ALEBind::Function(&LuaGlobalFunctions::GetPlayerByName);
    lua["GetGameTime"]                  = ALEBind::Function(&LuaGlobalFunctions::GetGameTime);
    lua["GetPlayersInWorld"]            = ALEBind::Function(&LuaGlobalFunctions::GetPlayersInWorld);
    lua["GetGuildByName"]               = ALEBind::Function(&LuaGlobalFunctions::GetGuildByName);
    lua["GetMapById"]                   = ALEBind::Function(&LuaGlobalFunctions::GetMapById);
    lua["GetGuildByLeaderGUID"]         = ALEBind::Function(&LuaGlobalFunctions::GetGuildByLeaderGUID);
    lua["GetPlayerCount"]               = ALEBind::Function(&LuaGlobalFunctions::GetPlayerCount);
    lua["GetPlayerGUID"]                = ALEBind::Function(&LuaGlobalFunctions::GetPlayerGUID);
    lua["GetItemGUID"]                  = ALEBind::Function(&LuaGlobalFunctions::GetItemGUID);
    lua["GetItemTemplate"]              = ALEBind::Function(&LuaGlobalFunctions::GetItemTemplate);
    lua["GetObjectGUID"]                = ALEBind::Function(&LuaGlobalFunctions::GetObjectGUID);
    lua["GetUnitGUID"]                  = ALEBind::Function(&LuaGlobalFunctions::GetUnitGUID);
    lua["GetGUIDLow"]                   = ALEBind::Function(&LuaGlobalFunctions::GetGUIDLow);
    lua["GetItemLink"]                  = ALEBind::Function(&LuaGlobalFunctions::GetItemLink);
    lua["GetGUIDType"]                  = ALEBind::Function(&LuaGlobalFunctions::GetGUIDType);
    lua["GetGUIDEntry"]                 = ALEBind::Function(&LuaGlobalFunctions::GetGUIDEntry);
    lua["GetPackedGUIDSize"]            = ALEBind::Function(&LuaGlobalFunctions::GetPackedGUIDSize);
    lua["GetAreaName"]                  = ALEBind::Function(&LuaGlobalFunctions::GetAreaName);
    lua["GetActiveGameEvents"]          = ALEBind::Function(&LuaGlobalFunctions::GetActiveGameEvents);
    lua["RegisterServerEvent"]          = ALEBind::Function(&LuaGlobalFunctions::RegisterServerEvent);
    lua["RegisterPlayerEvent"]          = ALEBind::Function(&LuaGlobalFunctions::RegisterPlayerEvent);
    lua["RegisterGuildEvent"]           = ALEBind::Function(&LuaGlobalFunctions::RegisterGuildEvent);
    lua["RegisterGroupEvent"]           = ALEBind::Function(&LuaGlobalFunctions::RegisterGroupEvent);
    lua["RegisterBGEvent"]              = ALEBind::Function(&LuaGlobalFunctions::RegisterBGEvent);
    lua["RegisterPacketEvent"]          = ALEBind::Function(&LuaGlobalFunctions::RegisterPacketEvent);
    lua["RegisterCreatureGossipEvent"]  = ALEBind::Function(&LuaGlobalFunctions::RegisterCreatureGossipEvent);
    lua["RegisterGameObjectGossipEvent"] = ALEBind::Function(&LuaGlobalFunctions::RegisterGameObjectGossipEvent);
    lua["RegisterItemEvent"]            = ALEBind::Function(&LuaGlobalFunctions::RegisterItemEvent);
    lua["RegisterItemGossipEvent"]      = ALEBind::Function(&LuaGlobalFunctions::RegisterItemGossipEvent);
    lua["RegisterMapEvent"]             = ALEBind::Function(&LuaGlobalFunctions::RegisterMapEvent);
    lua["RegisterInstanceEvent"]        = ALEBind::Function(&LuaGlobalFunctions::RegisterInstanceEvent);
    lua["RegisterPlayerGossipEvent"]    = ALEBind::Function(&LuaGlobalFunctions::RegisterPlayerGossipEvent);
    lua["RegisterCreatureEvent"]        = ALEBind::Function(&LuaGlobalFunctions::RegisterCreatureEvent);
    lua["RegisterUniqueCreatureEvent"]  = ALEBind::Function(&LuaGlobalFunctions::RegisterUniqueCreatureEvent);
    lua["RegisterGameObjectEvent"]      = ALEBind::Function(&LuaGlobalFunctions::RegisterGameObjectEvent);
    lua["RegisterTicketEvent"]          = ALEBind::Function(&LuaGlobalFunctions::RegisterTicketEvent);
    lua["RegisterSpellEvent"]           = ALEBind::Function(&LuaGlobalFunctions::RegisterSpellEvent);
    lua["RegisterAllCreatureEvent"]     = ALEBind::Function(&LuaGlobalFunctions::RegisterAllCreatureEvent);
    lua["ReloadALE"]                    = ALEBind::Function(&LuaGlobalFunctions::ReloadALE);
    lua["RunCommand"]                   = ALEBind::Function(&LuaGlobalFunctions::RunCommand);
    lua["SendWorldMessage"]             = ALEBind::Function(&LuaGlobalFunctions::SendWorldMessage);
    lua["WorldDBQuery"]                 = ALEBind::Function(&LuaGlobalFunctions::WorldDBQuery);
    lua["WorldDBQueryAsync"]            = ALEBind::Function(&LuaGlobalFunctions::WorldDBQueryAsync);
    lua["WorldDBExecute"]               = ALEBind::Function(&LuaGlobalFunctions::WorldDBExecute);
    lua["CharDBQuery"]                  = ALEBind::Function(&LuaGlobalFunctions::CharDBQuery);
    lua["CharDBQueryAsync"]             = ALEBind::Function(&LuaGlobalFunctions::CharDBQueryAsync);
    lua["CharDBExecute"]                = ALEBind::Function(&LuaGlobalFunctions::CharDBExecute);
    lua["AuthDBQuery"]                  = ALEBind::Function(&LuaGlobalFunctions::AuthDBQuery);
    lua["AuthDBQueryAsync"]             = ALEBind::Function(&LuaGlobalFunctions::AuthDBQueryAsync);
    lua["AuthDBExecute"]                = ALEBind::Function(&LuaGlobalFunctions::AuthDBExecute);
    lua["CreateLuaEvent"]               = ALEBind::Function(&LuaGlobalFunctions::CreateLuaEvent);
    lua["RemoveEventById"]              = ALEBind::Function(&LuaGlobalFunctions::RemoveEventById);
    lua["RemoveEvents"]                 = ALEBind::Function(&LuaGlobalFunctions::RemoveEvents);
    lua["PerformIngameSpawn"]           = ALEBind::Function(&LuaGlobalFunctions::PerformIngameSpawn);
    lua["CreatePacket"]                 = ALEBind::Function(&LuaGlobalFunctions::CreatePacket);
    lua["AddVendorItem"]                = ALEBind::Function(&LuaGlobalFunctions::AddVendorItem);
    lua["VendorRemoveItem"]             = ALEBind::Function(&LuaGlobalFunctions::VendorRemoveItem);
    lua["VendorRemoveAllItems"]         = ALEBind::Function(&LuaGlobalFunctions::VendorRemoveAllItems);
    lua["Kick"]                         = ALEBind::Function(&LuaGlobalFunctions::Kick);
    lua["Ban"]                          = ALEBind::Function(&LuaGlobalFunctions::Ban);
    lua["SaveAllPlayers"]               = ALEBind::Function(&LuaGlobalFunctions::SaveAllPlayers);
    lua["SendMail"]                     = ALEBind::Function(&LuaGlobalFunctions::SendMail);
    lua["bit_and"]                      = ALEBind::Function(&LuaGlobalFunctions::bit_and);
    lua["bit_or"]                       = ALEBind::Function(&LuaGlobalFunctions::bit_or);
    lua["bit_lshift"]                   = ALEBind::Function(&LuaGlobalFunctions::bit_lshift);
    lua["bit_rshift"]                   = ALEBind::Function(&LuaGlobalFunctions::bit_rshift);
    lua["bit_xor"]                      = ALEBind::Function(&LuaGlobalFunctions::bit_xor);
    lua["bit_not"]                      = ALEBind::Function(&LuaGlobalFunctions::bit_not);
    lua["AddTaxiPath"]                  = ALEBind::Function(&LuaGlobalFunctions::AddTaxiPath);
    lua["IsCompatibilityMode"]          = ALEBind::Function(&LuaGlobalFunctions::IsCompatibilityMode);
    lua["IsInventoryPos"]               = ALEBind::Function(&LuaGlobalFunctions::IsInventoryPos);
    lua["IsEquipmentPos"]               = ALEBind::Function(&LuaGlobalFunctions::IsEquipmentPos);
    lua["IsBankPos"]                    = ALEBind::Function(&LuaGlobalFunctions::IsBankPos);
    lua["IsBagPos"]                     = ALEBind::Function(&LuaGlobalFunctions::IsBagPos);
    lua["IsGameEventActive"]            = ALEBind::Function(&LuaGlobalFunctions::IsGameEventActive);
    lua["GetCurrTime"]                  = ALEBind::Function(&LuaGlobalFunctions::GetCurrTime);
    lua["GetTimeDiff"]                  = ALEBind::Function(&LuaGlobalFunctions::GetTimeDiff);
    lua["PrintInfo"]                    = ALEBind::Function(&LuaGlobalFunctions::PrintInfo);
    lua["PrintError"]                   = ALEBind::Function(&LuaGlobalFunctions::PrintError);
    lua["PrintDebug"]                   = ALEBind::Function(&LuaGlobalFunctions::PrintDebug);
    lua["StartGameEvent"]               = ALEBind::Function(&LuaGlobalFunctions::StartGameEvent);
    lua["StopGameEvent"]                = ALEBind::Function(&LuaGlobalFunctions::StopGameEvent);
    lua["HttpRequest"]                  = ALEBind::Function(&LuaGlobalFunctions::HttpRequest);
    lua["CreateLongLong"]               = ALEBind::Function(&LuaGlobalFunctions::CreateLongLong);
    lua["CreateULongLong"]              = ALEBind::Function(&LuaGlobalFunctions::CreateULongLong);
    lua["ClearBattleGroundEvents"]      = ALEBind::Function(&LuaGlobalFunctions::ClearBattleGroundEvents);
    lua["ClearCreatureEvents"]          = ALEBind::Function(&LuaGlobalFunctions::ClearCreatureEvents);
    lua["ClearUniqueCreatureEvents"]    = ALEBind::Function(&LuaGlobalFunctions::ClearUniqueCreatureEvents);
    lua["ClearCreatureGossipEvents"]    = ALEBind::Function(&LuaGlobalFunctions::ClearCreatureGossipEvents);
    lua["ClearGameObjectEvents"]        = ALEBind::Function(&LuaGlobalFunctions::ClearGameObjectEvents);
    lua["ClearGameObjectGossipEvents"]  = ALEBind::Function(&LuaGlobalFunctions::ClearGameObjectGossipEvents);
    lua["ClearGroupEvents"]             = ALEBind::Function(&LuaGlobalFunctions::ClearGroupEvents);
    lua["ClearGuildEvents"]             = ALEBind::Function(&LuaGlobalFunctions::ClearGuildEvents);
    lua["ClearItemEvents"]              = ALEBind::Function(&LuaGlobalFunctions::ClearItemEvents);
    lua["ClearItemGossipEvents"]        = ALEBind::Function(&LuaGlobalFunctions::ClearItemGossipEvents);
    lua["ClearPacketEvents"]            = ALEBind::Function(&LuaGlobalFunctions::ClearPacketEvents);
    lua["ClearPlayerEvents"]            = ALEBind::Function(&LuaGlobalFunctions::ClearPlayerEvents);
    lua["ClearPlayerGossipEvents"]      = ALEBind::Function(&LuaGlobalFunctions::ClearPlayerGossipEvents);
    lua["ClearServerEvents"]            = ALEBind::Function(&LuaGlobalFunctions::ClearServerEvents);
    lua["ClearMapEvents"]               = ALEBind::Function(&LuaGlobalFunctions::ClearMapEvents);
    lua["ClearInstanceEvents"]          = ALEBind::Function(&LuaGlobalFunctions::ClearInstanceEvents);
    lua["ClearTicketEvents"]            = ALEBind::Function(&LuaGlobalFunctions::ClearTicketEvents);
    lua["ClearSpellEvents"]             = ALEBind::Function(&LuaGlobalFunctions::ClearSpellEvents);
    lua["ClearAllCreatureEvents"]       = ALEBind::Function(&LuaGlobalFunctions::ClearAllCreatureEvents);
    lua["GetOwnerHalaa"]                = ALEBind::Function(&LuaGlobalFunctions::GetOwnerHalaa);
    lua["SetOwnerHalaa"]                = ALEBind::Function(&LuaGlobalFunctions::SetOwnerHalaa);
    lua["GetGossipMenuOptionLocale"]    = ALEBind::Function(&LuaGlobalFunctions::GetGossipMenuOptionLocale);
    lua["GetMapEntrance"]               = ALEBind::Function(&LuaGlobalFunctions::GetMapEntrance);
    lua["GetSpellInfo"]                 = ALEBind::Function(&LuaGlobalFunctions::GetSpellInfo);
    lua["LookupEntry"]                  = ALEBind::Function(&LuaGlobalFunctions::LookupEntry);
}
