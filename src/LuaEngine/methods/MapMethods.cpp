/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"
#include "ALEInstanceAI.h"

#include "Corpse.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "SharedDefines.h"
#include "Weather.h"

/***
 * A game map, e.g. Azeroth, Eastern Kingdoms, the Molten Core, etc.
 *
 * Inherits all methods from: none
 */
namespace LuaMap
{

    /**
     * Returns `true` if the [Map] is an arena [BattleGround], `false` otherwise.
     *
     * @return bool isArena
     */
    bool IsArena(Map* map)
    {
        return map->IsBattleArena();
    }

    /**
     * Returns `true` if the [Map] is a non-arena [BattleGround], `false` otherwise.
     *
     * @return bool isBattleGround
     */
    bool IsBattleground(Map* map)
    {
        return map->IsBattleground();
    }

    /**
     * Returns `true` if the [Map] is a dungeon, `false` otherwise.
     *
     * @return bool isDungeon
     */
    bool IsDungeon(Map* map)
    {
        return map->IsDungeon();
    }

    /**
     * Returns `true` if the [Map] has no [Player]s, `false` otherwise.
     *
     * @return bool IsEmpty
     */
    bool IsEmpty(Map* map)
    {
        return map->IsEmpty();
    }

    /**
     * Returns `true` if the [Map] is a heroic, `false` otherwise.
     *
     * @return bool isHeroic
     */
    bool IsHeroic(Map* map)
    {
        return map->IsHeroic();
    }

    /**
     * Returns `true` if the [Map] is a raid, `false` otherwise.
     *
     * @return bool isRaid
     */
    bool IsRaid(Map* map)
    {
        return map->IsRaid();
    }

    /**
     * Returns the name of the [Map].
     *
     * @return string mapName
     */
    char const* GetName(Map* map)
    {
        return map->GetMapName();
    }

    /**
     * Returns the height of the [Map] at the given X and Y coordinates.
     *
     * In case of no height found nil is returned
     *
     * @param float x
     * @param float y
     * @return float z
     */
    sol::optional<float> GetHeight(Map* map, float x, float y, sol::optional<uint32> phasemask)
    {
        float z = map->GetHeight(phasemask.value_or(1), x, y, MAX_HEIGHT);
        if (z != INVALID_HEIGHT)
            return z;

        return sol::nullopt;
    }

    /**
     * Returns the difficulty of the [Map].
     *
     * Always returns 0 if the expansion is pre-TBC.
     *
     * @return int32 difficulty
     */
    Difficulty GetDifficulty(Map* map)
    {
        return map->GetDifficulty();
    }

    /**
     * Returns the instance ID of the [Map].
     *
     * @return uint32 instanceId
     */
    uint32 GetInstanceId(Map* map)
    {
        return map->GetInstanceId();
    }

    /**
     * Returns the player count currently on the [Map] (excluding GMs).
     *
     * @return uint32 playerCount
     */
    uint32 GetPlayerCount(Map* map)
    {
        return map->GetPlayersCountExceptGMs();
    }

    /**
     * Returns the ID of the [Map].
     *
     * @return uint32 mapId
     */
    uint32 GetMapId(Map* map)
    {
        return map->GetId();
    }

    /**
     * Returns the area ID of the [Map] at the specified X, Y, and Z coordinates.
     *
     * @param float x
     * @param float y
     * @param float z
     * @param uint32 phasemask = PHASEMASK_NORMAL
     * @return uint32 areaId
     */
    uint32 GetAreaId(Map* map, float x, float y, float z, sol::optional<uint32> phasemask)
    {
        return map->GetAreaId(phasemask.value_or(PHASEMASK_NORMAL), x, y, z);
    }

    /**
     * Returns a [WorldObject] by its GUID from the map if it is spawned.
     *
     * @param ObjectGuid guid
     * @return [WorldObject] object
     */
    sol::object GetWorldObject(Map* map, ObjectGuid guid, sol::this_state s)
    {
        WorldObject* obj = nullptr;

        switch (guid.GetHigh())
        {
            case HighGuid::Player:
                obj = ObjectAccessor::GetPlayer(map, guid);
                break;
            case HighGuid::Transport:
            case HighGuid::Mo_Transport:
            case HighGuid::GameObject:
                obj = map->GetGameObject(guid);
                break;
            case HighGuid::Vehicle:
            case HighGuid::Unit:
                obj = map->GetCreature(guid);
                break;
            case HighGuid::Pet:
                obj = map->GetPet(guid);
                break;
            case HighGuid::DynamicObject:
                obj = map->GetDynamicObject(guid);
                break;
            case HighGuid::Corpse:
                obj = map->GetCorpse(guid);
                break;
            default:
                break;
        }

        return ALEBind::ToLuaDynamic(sol::state_view(s), obj);
    }

    /**
     * Sets the [Weather] type based on [WeatherType] and grade supplied.
     *
     *     enum WeatherType
     *     {
     *         WEATHER_TYPE_FINE       = 0,
     *         WEATHER_TYPE_RAIN       = 1,
     *         WEATHER_TYPE_SNOW       = 2,
     *         WEATHER_TYPE_STORM      = 3,
     *         WEATHER_TYPE_THUNDERS   = 86,
     *         WEATHER_TYPE_BLACKRAIN  = 90
     *     };
     *
     * @param uint32 zone : id of the zone to set the weather for
     * @param [WeatherType] type : the [WeatherType], see above available weather types
     * @param float grade : the intensity/grade of the [Weather], ranges from 0 to 1
     */
    void SetWeather(Map* map, uint32 zoneId, uint32 weatherType, float grade)
    {
        Weather* weather = map->GetOrGenerateZoneDefaultWeather(zoneId);
        if (weather)
            weather->SetWeather((WeatherType)weatherType, grade);
    }

    /**
     * Gets the instance data table for the [Map], if it exists.
     *
     * The instance must be scripted using ALE for this to succeed.
     * If the instance is scripted in C++ this will return `nil`.
     *
     * @return table instance_data : instance data table, or `nil`
     */
    sol::object GetInstanceData(Map* map, sol::this_state s)
    {
        ALEInstanceAI* iAI = nullptr;
        if (InstanceMap* inst = map->ToInstanceMap())
            iAI = dynamic_cast<ALEInstanceAI*>(inst->GetInstanceScript());

        sol::state_view lua(s);
        if (iAI)
            return sol::make_object(lua, sALE->GetInstanceData(iAI));

        return sol::make_object(lua, sol::lua_nil);
    }

    /**
     * Saves the [Map]'s instance data to the database.
     */
    void SaveInstanceData(Map* map)
    {
        ALEInstanceAI* iAI = nullptr;
        if (InstanceMap* inst = map->ToInstanceMap())
            iAI = dynamic_cast<ALEInstanceAI*>(inst->GetInstanceScript());

        if (iAI)
            iAI->SaveToDB();
    }

    /**
    * Returns a table with all the current [Player]s in the map
    *
    *     enum TeamId
    *     {
    *         TEAM_ALLIANCE = 0,
    *         TEAM_HORDE = 1,
    *         TEAM_NEUTRAL = 2
    *     };
    *
    * @param [TeamId] team : optional check team of the [Player], Alliance, Horde or Neutral (All)
    * @return table mapPlayers
    */
    sol::table GetPlayers(Map* map, sol::optional<uint32> teamArg, sol::this_state s)
    {
        uint32 team = teamArg.value_or(TEAM_NEUTRAL);

        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player)
                continue;
            if (player->GetSession() && (team >= TEAM_NEUTRAL || player->GetTeamId() == team))
                tbl[++i] = PlayerRef(player);
        }

        return tbl;
    }

    /**
     * Returns a table with all the current [Creature]s in the map
     *
     * @return table mapCreatures
     */
    sol::table GetCreatures(Map* map, sol::this_state s)
    {
        auto const& creatures = map->GetCreatureBySpawnIdStore();

        sol::table tbl = sol::state_view(s).create_table();

        for (auto const& pair : creatures)
        {
            Creature* creature = pair.second;
            tbl[creature->GetSpawnId()] = CreatureRef(creature);
        }

        return tbl;
    }

    /**
     * Returns a table with all the current [Creature]s in the specific area id
     *
     * @param number areaId : specific area id
     * @return table mapCreatures
     */
    sol::table GetCreaturesByAreaId(Map* map, sol::optional<int32> areaIdArg, sol::this_state s)
    {
        int32 areaId = areaIdArg.value_or(-1);
        std::vector<Creature*> filteredCreatures;

        for (auto const& pair : map->GetCreatureBySpawnIdStore())
        {
            Creature* creature = pair.second;
            if (areaId == -1 || creature->GetAreaId() == (uint32)areaId)
                filteredCreatures.push_back(creature);
        }

        sol::table tbl = sol::state_view(s).create_table();

        for (Creature* creature : filteredCreatures)
            tbl[creature->GetSpawnId()] = CreatureRef(creature);

        return tbl;
    }

    /**
     * Returns a table of all [Transport]s on the [Map]
     *
     * @return table transports
     */
    sol::table GetTransports(Map* map, sol::this_state s)
    {
        TransportsContainer const& transports = map->GetAllTransports();

        sol::table tbl = sol::state_view(s).create_table();
        int i = 1;

        for (Transport* transport : transports)
            tbl[i++] = TransportRef(transport);

        return tbl;
    }
}

void RegisterMapMethods(sol::state& lua)
{
    sol::usertype<MapRef> type = ALEBind::NewHandleType<MapRef>(lua, "Map");

    type["IsArena"]              = ALEBind::Method(&LuaMap::IsArena);
    type["IsBattleground"]       = ALEBind::Method(&LuaMap::IsBattleground);
    type["IsDungeon"]            = ALEBind::Method(&LuaMap::IsDungeon);
    type["IsEmpty"]              = ALEBind::Method(&LuaMap::IsEmpty);
    type["IsHeroic"]             = ALEBind::Method(&LuaMap::IsHeroic);
    type["IsRaid"]               = ALEBind::Method(&LuaMap::IsRaid);
    type["GetName"]              = ALEBind::Method(&LuaMap::GetName);
    type["GetHeight"]            = ALEBind::Method(&LuaMap::GetHeight);
    type["GetDifficulty"]        = ALEBind::Method(&LuaMap::GetDifficulty);
    type["GetInstanceId"]        = ALEBind::Method(&LuaMap::GetInstanceId);
    type["GetPlayerCount"]       = ALEBind::Method(&LuaMap::GetPlayerCount);
    type["GetMapId"]             = ALEBind::Method(&LuaMap::GetMapId);
    type["GetAreaId"]            = ALEBind::Method(&LuaMap::GetAreaId);
    type["GetWorldObject"]       = ALEBind::Method(&LuaMap::GetWorldObject);
    type["SetWeather"]           = ALEBind::Method(&LuaMap::SetWeather);
    type["GetInstanceData"]      = ALEBind::Method(&LuaMap::GetInstanceData);
    type["SaveInstanceData"]     = ALEBind::Method(&LuaMap::SaveInstanceData);
    type["GetPlayers"]           = ALEBind::Method(&LuaMap::GetPlayers);
    type["GetCreatures"]         = ALEBind::Method(&LuaMap::GetCreatures);
    type["GetCreaturesByAreaId"] = ALEBind::Method(&LuaMap::GetCreaturesByAreaId);
    type["GetTransports"]        = ALEBind::Method(&LuaMap::GetTransports);
}
