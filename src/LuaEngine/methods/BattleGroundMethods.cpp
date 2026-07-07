/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Battleground.h"
#include "Map.h"

/***
 * Contains the state of a battleground, e.g. Warsong Gulch, Arathi Basin, etc.
 *
 * Inherits all methods from: none
 */
namespace LuaBattleGround
{
    /**
     * Returns the name of the [BattleGround].
     *
     * @return string name
     */
    std::string GetName(Battleground* bg)
    {
        return bg->GetName();
    }

    /**
     * Returns the amount of alive players in the [BattleGround] by the team ID.
     *
     * @param [Team] team : team ID
     * @return uint32 count
     */
    uint32 GetAlivePlayersCountByTeam(Battleground* bg, uint32 team)
    {
        return bg->GetAlivePlayersCountByTeam((TeamId)team);
    }

    /**
     * Returns the [Map] of the [BattleGround].
     *
     * @return [Map] map
     */
    Map* GetMap(Battleground* bg)
    {
        return bg->GetBgMap();
    }

    /**
     * Returns the bonus honor given by amount of kills in the specific [BattleGround].
     *
     * @param uint32 kills : amount of kills
     * @return uint32 bonusHonor
     */
    uint32 GetBonusHonorFromKillCount(Battleground* bg, uint32 kills)
    {
        return bg->GetBonusHonorFromKill(kills);
    }

    /**
     * Returns the end time of the [BattleGround].
     *
     * @return uint32 endTime
     */
    uint32 GetEndTime(Battleground* bg)
    {
        return bg->GetEndTime();
    }

    /**
     * Returns the amount of free slots for the selected team in the specific [BattleGround].
     *
     * @param [Team] team : team ID
     * @return uint32 freeSlots
     */
    uint32 GetFreeSlotsForTeam(Battleground* bg, uint32 team)
    {
        return bg->GetFreeSlotsForTeam((TeamId)team);
    }

    /**
     * Returns the instance ID of the [BattleGround].
     *
     * @return uint32 instanceId
     */
    uint32 GetInstanceId(Battleground* bg)
    {
        return bg->GetInstanceID();
    }

    /**
     * Returns the map ID of the [BattleGround].
     *
     * @return uint32 mapId
     */
    uint32 GetMapId(Battleground* bg)
    {
        return bg->GetMapId();
    }

    /**
     * Returns the type ID of the [BattleGround].
     *
     * @return [BattleGroundTypeId] typeId
     */
    BattlegroundTypeId GetTypeId(Battleground* bg)
    {
        return bg->GetBgTypeID();
    }

    /**
     * Returns the max allowed [Player] level of the specific [BattleGround].
     *
     * @return uint32 maxLevel
     */
    uint32 GetMaxLevel(Battleground* bg)
    {
        return bg->GetMaxLevel();
    }

    /**
     * Returns the minimum allowed [Player] level of the specific [BattleGround].
     *
     * @return uint32 minLevel
     */
    uint32 GetMinLevel(Battleground* bg)
    {
        return bg->GetMinLevel();
    }

    /**
     * Returns the maximum allowed [Player] count of the specific [BattleGround].
     *
     * @return uint32 maxPlayerCount
     */
    uint32 GetMaxPlayers(Battleground* bg)
    {
        return bg->GetMaxPlayersPerTeam() * 2;
    }

    /**
     * Returns the minimum allowed [Player] count of the specific [BattleGround].
     *
     * @return uint32 minPlayerCount
     */
    uint32 GetMinPlayers(Battleground* bg)
    {
        return bg->GetMinPlayersPerTeam() * 2;
    }

    /**
     * Returns the maximum allowed [Player] count per team of the specific [BattleGround].
     *
     * @return uint32 maxTeamPlayerCount
     */
    uint32 GetMaxPlayersPerTeam(Battleground* bg)
    {
        return bg->GetMaxPlayersPerTeam();
    }

    /**
     * Returns the minimum allowed [Player] count per team of the specific [BattleGround].
     *
     * @return uint32 minTeamPlayerCount
     */
    uint32 GetMinPlayersPerTeam(Battleground* bg)
    {
        return bg->GetMinPlayersPerTeam();
    }

    /**
     * Returns the winning team of the specific [BattleGround].
     *
     * @return [Team] team
     */
    PvPTeamId GetWinner(Battleground* bg)
    {
        return bg->GetWinner();
    }

    /**
     * Returns the status of the specific [BattleGround].
     *
     * @return [BattleGroundStatus] status
     */
    BattlegroundStatus GetStatus(Battleground* bg)
    {
        return bg->GetStatus();
    }
}

void RegisterBattleGroundMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<Battleground>> type = ALEBind::NewHandleType<ScopedRef<Battleground>>(lua, "BattleGround");

    type["GetName"]                    = ALEBind::Method(&LuaBattleGround::GetName);
    type["GetAlivePlayersCountByTeam"] = ALEBind::Method(&LuaBattleGround::GetAlivePlayersCountByTeam);
    type["GetMap"]                     = ALEBind::Method(&LuaBattleGround::GetMap);
    type["GetBonusHonorFromKillCount"] = ALEBind::Method(&LuaBattleGround::GetBonusHonorFromKillCount);
    type["GetEndTime"]                 = ALEBind::Method(&LuaBattleGround::GetEndTime);
    type["GetFreeSlotsForTeam"]        = ALEBind::Method(&LuaBattleGround::GetFreeSlotsForTeam);
    type["GetInstanceId"]              = ALEBind::Method(&LuaBattleGround::GetInstanceId);
    type["GetMapId"]                   = ALEBind::Method(&LuaBattleGround::GetMapId);
    type["GetTypeId"]                  = ALEBind::Method(&LuaBattleGround::GetTypeId);
    type["GetMaxLevel"]                = ALEBind::Method(&LuaBattleGround::GetMaxLevel);
    type["GetMinLevel"]                = ALEBind::Method(&LuaBattleGround::GetMinLevel);
    type["GetMaxPlayers"]              = ALEBind::Method(&LuaBattleGround::GetMaxPlayers);
    type["GetMinPlayers"]              = ALEBind::Method(&LuaBattleGround::GetMinPlayers);
    type["GetMaxPlayersPerTeam"]       = ALEBind::Method(&LuaBattleGround::GetMaxPlayersPerTeam);
    type["GetMinPlayersPerTeam"]       = ALEBind::Method(&LuaBattleGround::GetMinPlayersPerTeam);
    type["GetWinner"]                  = ALEBind::Method(&LuaBattleGround::GetWinner);
    type["GetStatus"]                  = ALEBind::Method(&LuaBattleGround::GetStatus);
}
