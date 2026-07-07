/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Group.h"

/***
 * Represents a group loot roll session for an item, including player votes and roll statistics.
 *
 * Provides access to the item being rolled, player vote types, and counts of each roll type (Need, Greed, Pass).
 *
 * Inherits all methods from: none
 */
namespace LuaRoll
{
    /**
     * Returns the rolled [Item]'s GUID.
     *
     * @return ObjectGuid guid
     */
    uint32 GetItemGUID(Roll* roll)
    {
        return roll->itemGUID.GetCounter();
    }

    /**
     * Returns the rolled [Item]'s entry.
     *
     * @return uint32 entry
     */
    uint32 GetItemId(Roll* roll)
    {
        return roll->itemid;
    }

    /**
     * Returns the rolled [Item]'s random property ID.
     *
     * @return int32 randomPropId
     */
    int32 GetItemRandomPropId(Roll* roll)
    {
        return roll->itemRandomPropId;
    }

    /**
     * Returns the rolled [Item]'s random suffix ID.
     *
     * @return uint32 randomSuffix
     */
    uint32 GetItemRandomSuffix(Roll* roll)
    {
        return roll->itemRandomSuffix;
    }

    /**
     * Returns the rolled [Item]'s count.
     *
     * @return uint8 count
     */
    uint8 GetItemCount(Roll* roll)
    {
        return roll->itemCount;
    }

    /**
     * Returns the vote type for a [Player] on this [Roll].
     * See [Roll:GetPlayerVoteGUIDs] to obtain the GUIDs of the [Player]s who rolled.
     *
     * <pre>
     * enum RollVote
     * {
     *     PASS              = 0,
     *     NEED              = 1,
     *     GREED             = 2,
     *     DISENCHANT        = 3,
     *     NOT_EMITED_YET    = 4,
     *     NOT_VALID         = 5
     * };
     * </pre>
     *
     * @param ObjectGuid guid
     * @return [RollVote] vote
     */
    sol::optional<RollVote> GetPlayerVote(Roll* roll, ObjectGuid guid)
    {
        sol::optional<RollVote> vote;
        for (std::pair<const ObjectGuid, RollVote>& pair : roll->playerVote)
        {
            if (pair.first == guid)
            {
                vote = pair.second;
            }
        }

        return vote;
    }

    /**
     * Returns the GUIDs of the [Player]s who rolled.
     * See [Roll:GetPlayerVote] to obtain the vote type of a [Player].
     *
     * @return table guids
     */
    sol::table GetPlayerVoteGUIDs(Roll* roll, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 1;
        for (std::pair<const ObjectGuid, RollVote>& pair : roll->playerVote)
        {
            tbl[i] = pair.first;
            ++i;
        }

        return tbl;
    }

    /**
     * Returns the total number of players who rolled.
     *
     * @return uint8 playersCount
     */
    uint8 GetTotalPlayersRolling(Roll* roll)
    {
        return roll->totalPlayersRolling;
    }

    /**
     * Returns the total number of players who rolled need.
     *
     * @return uint8 playersCount
     */
    uint8 GetTotalNeed(Roll* roll)
    {
        return roll->totalNeed;
    }

    /**
     * Returns the total number of players who rolled greed.
     *
     * @return uint8 playersCount
     */
    uint8 GetTotalGreed(Roll* roll)
    {
        return roll->totalGreed;
    }

    /**
     * Returns the total number of players who passed.
     *
     * @return uint8 playersCount
     */
    uint8 GetTotalPass(Roll* roll)
    {
        return roll->totalPass;
    }

    /**
     * Returns the rolled [Item]'s slot in the loot window.
     *
     * @return uint8 slot
     */
    uint8 GetItemSlot(Roll* roll)
    {
        return roll->itemSlot;
    }

    /**
     * Returns the mask applied to this [Roll].
     *
     * <pre>
     * enum RollMask
     * {
     *     ROLL_FLAG_TYPE_PASS                 = 0x01,
     *     ROLL_FLAG_TYPE_NEED                 = 0x02,
     *     ROLL_FLAG_TYPE_GREED                = 0x04,
     *     ROLL_FLAG_TYPE_DISENCHANT           = 0x08,
     *
     *     ROLL_ALL_TYPE_NO_DISENCHANT         = 0x07,
     *     ROLL_ALL_TYPE_MASK                  = 0x0F
     * };
     * </pre>
     *
     * @return [RollMask] rollMask
     */
    uint8 GetRollVoteMask(Roll* roll)
    {
        return roll->rollVoteMask;
    }
}

void RegisterRollMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<Roll>> type = ALEBind::NewHandleType<ScopedRef<Roll>>(lua, "Roll");

    type["GetItemGUID"]            = ALEBind::Method(&LuaRoll::GetItemGUID);
    type["GetItemId"]              = ALEBind::Method(&LuaRoll::GetItemId);
    type["GetItemRandomPropId"]    = ALEBind::Method(&LuaRoll::GetItemRandomPropId);
    type["GetItemRandomSuffix"]    = ALEBind::Method(&LuaRoll::GetItemRandomSuffix);
    type["GetItemCount"]           = ALEBind::Method(&LuaRoll::GetItemCount);
    type["GetPlayerVote"]          = ALEBind::Method(&LuaRoll::GetPlayerVote);
    type["GetPlayerVoteGUIDs"]     = ALEBind::Method(&LuaRoll::GetPlayerVoteGUIDs);
    type["GetTotalPlayersRolling"] = ALEBind::Method(&LuaRoll::GetTotalPlayersRolling);
    type["GetTotalNeed"]           = ALEBind::Method(&LuaRoll::GetTotalNeed);
    type["GetTotalGreed"]          = ALEBind::Method(&LuaRoll::GetTotalGreed);
    type["GetTotalPass"]           = ALEBind::Method(&LuaRoll::GetTotalPass);
    type["GetItemSlot"]            = ALEBind::Method(&LuaRoll::GetItemSlot);
    type["GetRollVoteMask"]        = ALEBind::Method(&LuaRoll::GetRollVoteMask);
}
