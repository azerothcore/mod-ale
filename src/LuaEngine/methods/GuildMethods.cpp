/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "DatabaseEnv.h"
#include "Guild.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "WorldPacket.h"

#include <shared_mutex>

/***
 * Represents a player guild. Used to manage guild members, ranks, guild bank.
 *
 * Inherits all methods from: none
 */
namespace LuaGuild
{
    /**
     * Returns a table with the [Player]s in this [Guild]
     *
     * Only the players that are online and on some map.
     *
     * @return table guildPlayers : table of [Player]s
     */
    sol::table GetMembers(Guild* guild, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        {
            std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
            HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
            for (HashMapHolder<Player>::MapType::const_iterator it = m.begin(); it != m.end(); ++it)
            {
                if (Player* player = it->second)
                {
                    if (player->IsInWorld() && player->GetGuildId() == guild->GetId())
                    {
                        tbl[++i] = PlayerRef(player);
                    }
                }
            }
        }

        return tbl;
    }

    /**
     * Returns the member count of this [Guild]
     *
     * @return uint32 memberCount
     */
    uint32 GetMemberCount(Guild* guild)
    {
        return guild->GetMemberCount();
    }

    /**
     * Finds and returns the [Guild] leader by their GUID if logged in
     *
     * @return [Player] leader
     */
    Player* GetLeader(Guild* guild)
    {
        return ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    }

    /**
     * Returns [Guild] leader GUID
     *
     * @return ObjectGuid leaderGUID
     */
    ObjectGuid GetLeaderGUID(Guild* guild)
    {
        return guild->GetLeaderGUID();
    }

    /**
     * Returns the [Guild]s entry ID
     *
     * @return uint32 entryId
     */
    uint32 GetId(Guild* guild)
    {
        return guild->GetId();
    }

    /**
     * Returns the [Guild]s name
     *
     * @return string guildName
     */
    std::string GetName(Guild* guild)
    {
        return guild->GetName();
    }

    /**
     * Returns the [Guild]s current Message Of The Day
     *
     * @return string guildMOTD
     */
    std::string GetMOTD(Guild* guild)
    {
        return guild->GetMOTD();
    }

    /**
     * Returns the [Guild]s current info
     *
     * @return string guildInfo
     */
    std::string GetInfo(Guild* guild)
    {
        return guild->GetInfo();
    }

    /**
     * Sets the leader of this [Guild]
     *
     * @param [Player] leader : the [Player] leader to change
     */
    void SetLeader(Guild* guild, Player* player)
    {
        guild->HandleSetLeader(player->GetSession(), player->GetName());
    }

    /**
     * Sets the information of the bank tab specified
     *
     * @param uint8 tabId : the ID of the tab specified
     * @param string info : the information to be set to the bank tab
     */
    void SetBankTabText(Guild* guild, uint8 tabId, std::string text)
    {
        guild->SetBankTabText(tabId, text);
    }

    // SendPacketToGuild(packet)
    /**
     * Sends a [WorldPacket] to all the [Player]s in the [Guild]
     *
     * @param [WorldPacket] packet : the [WorldPacket] to be sent to the [Player]s
     */
    void SendPacket(Guild* guild, WorldPacket* data)
    {
        guild->BroadcastPacket(data);
    }

    // SendPacketToRankedInGuild(packet, rankId)
    /**
     * Sends a [WorldPacket] to all the [Player]s at the specified rank in the [Guild]
     *
     * @param [WorldPacket] packet : the [WorldPacket] to be sent to the [Player]s
     * @param uint8 rankId : the rank ID
     */
    void SendPacketToRanked(Guild* guild, WorldPacket* data, uint8 ranked)
    {
        guild->BroadcastPacketToRank(data, ranked);
    }

    /**
     * Disbands the [Guild]
     */
    void Disband(Guild* guild)
    {
        guild->Disband();
    }

    /**
     * Adds the specified [Player] to the [Guild] at the specified rank.
     *
     * If no rank is specified, defaults to none.
     *
     * @param [Player] player : the [Player] to be added to the guild
     * @param uint8 rankId : the rank ID
     */
    void AddMember(Guild* guild, Player* player, sol::optional<uint8> rankId)
    {
        guild->AddMember(player->GetGUID(), rankId.value_or(GUILD_RANK_NONE));
    }

    /**
     * Removes the specified [Player] from the [Guild].
     *
     * @param [Player] player : the [Player] to be removed from the guild
     * @param bool isDisbanding : default 'false', should only be set to 'true' if the guild is triggered to disband
     */
    void DeleteMember(Guild* guild, Player* player, sol::optional<bool> isDisbanding)
    {
        guild->DeleteMember(player->GetGUID(), isDisbanding.value_or(false));
    }

    /**
     * Promotes/demotes the [Player] to the specified rank.
     *
     * @param [Player] player : the [Player] to be promoted/demoted
     * @param uint8 rankId : the rank ID
     */
    void SetMemberRank(Guild* guild, Player* player, uint8 newRank)
    {
        guild->ChangeMemberRank(player->GetGUID(), newRank);
    }

    /**
     * Sets the new name of the specified [Guild].
     *
     * @param string name : new name of this guild
     */
    void SetName(Guild* guild, std::string name)
    {
        guild->SetName(name);
    }

    /**
     * Update [Player] data in [Guild] member list.
     *
     *     enum GuildMemberData
     *     {
     *         GUILD_MEMBER_DATA_ZONEID =  0
     *         GUILD_MEMBER_DATA_LEVEL  =  1
     *     };
     *
     *  @param [Player] player : plkayer you need to update data
     *  @param [GuildMemberData] dataid : data you need to update
     *  @param uint32 value
     */
    void UpdateMemberData(Guild* guild, Player* player, uint8 dataid, uint32 value)
    {
        guild->UpdateMemberData(player, dataid, value);
    }

    /**
     * Send message to [Guild] from specific [Player].
     *
     * @param [Player] player : the [Player] is the author of the message
     * @param bool officerOnly : send message only on officer channel
     * @param string msg : the message you need to send
     * @param uint32 lang : language the [Player] will speak
     */
    void SendMessage(Guild* guild, Player* player, sol::optional<bool> officerOnly, std::string msg, sol::optional<uint32> language)
    {
        guild->BroadcastToGuild(player->GetSession(), officerOnly.value_or(false), msg, language.value_or(0));
    }

    /**
     * Invites [Guild] members to events based on level and rank filters.
     *
     * @param [Player] player : who sends the invitation
     * @param uint32 minLevel : the required min level
     * @param uint32 maxLevel : the required max level
     * @param uint32 minRank : the required min rank
     */
    void MassInviteToEvent(Guild* guild, Player* player, uint32 minLevel, uint32 maxLevel, uint32 minRank)
    {
        guild->MassInviteToEvent(player->GetSession(), minLevel, maxLevel, minRank);
    }

    /**
     * Swap item from a specific tab and slot [Guild] bank to another one.
     *
     * @param [Player] player : who Swap the item
     * @param uint8 tabId : source tab id
     * @param uint8 slotId : source slot id
     * @param uint8 destTabId : destination tab id
     * @param uint8 destSlotId : destination slot id
     * @param uint8 splitedAmount : if the item is stackable, how much should be swaped
     */
    void SwapItems(Guild* guild, Player* player, uint8 tabId, uint8 slotId, uint8 destTabId, uint8 destSlotId, uint32 splitedAmount)
    {
        guild->SwapItems(player, tabId, slotId, destTabId, destSlotId, splitedAmount);
    }

    /**
     * Swap an item from a specific tab and location in the [guild] bank to the bags and locations in the inventory of a specific [player] and vice versa.
     *
     * @param [Player] player : who Swap the item
     * @param bool toChar : the item goes to the [Player]'s inventory or comes from the [Player]'s inventory
     * @param uint8 tabId : tab id
     * @param uint8 slotId : slot id
     * @param uint8 playerBag : bag id
     * @param uint8 playerSlotId : slot id
     * @param uint32 splitedAmount : if the item is stackable, how much should be swaped
     */
    void SwapItemsWithInventory(Guild* guild, Player* player, sol::optional<bool> toChar, uint8 tabId, uint8 slotId, uint8 playerBag, uint8 playerSlotId, uint32 splitedAmount)
    {
        guild->SwapItemsWithInventory(player, toChar.value_or(false), tabId, slotId, playerBag, playerSlotId, splitedAmount);
    }

    /**
     * Return the total bank money.
     *
     * @return number totalBankMoney
     */
    uint64 GetTotalBankMoney(Guild* guild)
    {
        return guild->GetTotalBankMoney();
    }

    /**
     * Return the created date.
     *
     * @return uint64 created date
     */
    time_t GetCreatedDate(Guild* guild)
    {
        return guild->GetCreatedDate();
    }

    /**
     * Resets the number of item withdraw in all tab's for all [Guild] members.
     */
    void ResetTimes(Guild* guild)
    {
        guild->ResetTimes();
    }

    /**
     * Modify the [Guild] bank money. You can deposit or withdraw.
     *
     * @param uint64 amount : amount to add or remove
     * @param bool add : true (add money) | false (withdraw money)
     * @return bool is_applied
     */
    bool ModifyBankMoney(Guild* guild, uint64 amount, bool add)
    {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        bool applied = guild->ModifyBankMoney(trans, amount, add);

        CharacterDatabase.CommitTransaction(trans);
        return applied;
    }
}

void RegisterGuildMethods(sol::state& lua)
{
    sol::usertype<GuildRef> type = ALEBind::NewHandleType<GuildRef>(lua, "Guild");

    type["GetMembers"]             = ALEBind::Method(&LuaGuild::GetMembers);
    type["GetMemberCount"]         = ALEBind::Method(&LuaGuild::GetMemberCount);
    type["GetLeader"]              = ALEBind::Method(&LuaGuild::GetLeader);
    type["GetLeaderGUID"]          = ALEBind::Method(&LuaGuild::GetLeaderGUID);
    type["GetId"]                  = ALEBind::Method(&LuaGuild::GetId);
    type["GetName"]                = ALEBind::Method(&LuaGuild::GetName);
    type["GetMOTD"]                = ALEBind::Method(&LuaGuild::GetMOTD);
    type["GetInfo"]                = ALEBind::Method(&LuaGuild::GetInfo);
    type["SetLeader"]              = ALEBind::Method(&LuaGuild::SetLeader);
    type["SetBankTabText"]         = ALEBind::Method(&LuaGuild::SetBankTabText);
    type["SendPacket"]             = ALEBind::Method(&LuaGuild::SendPacket);
    type["SendPacketToRanked"]     = ALEBind::Method(&LuaGuild::SendPacketToRanked);
    type["Disband"]                = ALEBind::Method(&LuaGuild::Disband);
    type["AddMember"]              = ALEBind::Method(&LuaGuild::AddMember);
    type["DeleteMember"]           = ALEBind::Method(&LuaGuild::DeleteMember);
    type["SetMemberRank"]          = ALEBind::Method(&LuaGuild::SetMemberRank);
    type["SetName"]                = ALEBind::Method(&LuaGuild::SetName);
    type["UpdateMemberData"]       = ALEBind::Method(&LuaGuild::UpdateMemberData);
    type["SendMessage"]            = ALEBind::Method(&LuaGuild::SendMessage);
    type["MassInviteToEvent"]      = ALEBind::Method(&LuaGuild::MassInviteToEvent);
    type["SwapItems"]              = ALEBind::Method(&LuaGuild::SwapItems);
    type["SwapItemsWithInventory"] = ALEBind::Method(&LuaGuild::SwapItemsWithInventory);
    type["GetTotalBankMoney"]      = ALEBind::Method(&LuaGuild::GetTotalBankMoney);
    type["GetCreatedDate"]         = ALEBind::Method(&LuaGuild::GetCreatedDate);
    type["ResetTimes"]             = ALEBind::Method(&LuaGuild::ResetTimes);
    type["ModifyBankMoney"]        = ALEBind::Method(&LuaGuild::ModifyBankMoney);
}
