/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Group.h"
#include "Player.h"
#include "WorldPacket.h"

/***
 * Represents a player group in the game, such as a party or raid.
 *
 * Inherits all methods from: none
 */
namespace LuaGroup
{
    /**
     * Returns 'true' if the [Player] is the [Group] leader
     *
     * @param ObjectGuid guid : guid of a possible leader
     * @return bool isLeader
     */
    bool IsLeader(Group* group, ObjectGuid guid)
    {
        return group->IsLeader(guid);
    }

    /**
     * Returns 'true' if the [Group] is full
     *
     * @return bool isFull
     */
    bool IsFull(Group* group)
    {
        return group->IsFull();
    }

    /**
     * Returns 'true' if the [Group] is a LFG group
     *
     * @return bool isLFGGroup
     */
    bool IsLFGGroup(Group* group)
    {
        return group->isLFGGroup();
    }

    /**
     * Returns 'true' if the [Group] is a raid [Group]
     *
     * @return bool isRaid
     */
    bool IsRaidGroup(Group* group)
    {
        return group->isRaidGroup();
    }

    /**
     * Returns 'true' if the [Group] is a battleground [Group]
     *
     * @return bool isBG
     */
    bool IsBGGroup(Group* group)
    {
        return group->isBGGroup();
    }

    /**
     * Returns 'true' if the [Player] is a member of this [Group]
     *
     * @param ObjectGuid guid : guid of a player
     * @return bool isMember
     */
    bool IsMember(Group* group, ObjectGuid guid)
    {
        return group->IsMember(guid);
    }

    /**
     * Returns 'true' if the [Player] is an assistant of this [Group]
     *
     * @param ObjectGuid guid : guid of a player
     * @return bool isAssistant
     */
    bool IsAssistant(Group* group, ObjectGuid guid)
    {
        return group->IsAssistant(guid);
    }

    /**
     * Returns 'true' if the [Player]s are in the same subgroup in this [Group]
     *
     * @param [Player] player1 : first [Player] to check
     * @param [Player] player2 : second [Player] to check
     * @return bool sameSubGroup
     */
    bool SameSubGroup(Group* group, Player* player1, Player* player2)
    {
        return group->SameSubGroup(player1, player2);
    }

    /**
     * Returns 'true' if the subgroup has free slots in this [Group]
     *
     * @param uint8 subGroup : subGroup ID to check
     * @return bool hasFreeSlot
     */
    bool HasFreeSlotSubGroup(Group* group, uint8 subGroup)
    {
        if (subGroup >= MAX_RAID_SUBGROUPS)
            throw std::invalid_argument("valid subGroup ID expected");

        return group->HasFreeSlotSubGroup(subGroup);
    }

    /**
     * Adds a new member to the [Group]
     *
     * @param [Player] player : [Player] to add to the group
     * @return bool added : true if member was added
     */
    bool AddMember(Group* group, Player* player)
    {
        if (player->GetGroup() || !group->IsCreated() || group->IsFull())
            return false;

        if (player->GetGroupInvite())
            player->UninviteFromGroup();

        bool success = group->AddMember(player);
        if (success)
            group->BroadcastGroupUpdate();

        return success;
    }

    /*bool IsLFGGroup(Group* group) // TODO: Implementation
    {
        return group->isLFGGroup();
    }*/

    /*bool IsBFGroup(Group* group) // TODO: Implementation
    {
        return group->isBFGroup();
    }*/

    /**
     * Returns a table with the [Player]s in this [Group]
     *
     * @return table groupPlayers : table of [Player]s
     */
    sol::table GetMembers(Group* group, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();

            if (!member || !member->GetSession())
                continue;

            tbl[++i] = PlayerRef(member);
        }

        return tbl;
    }

    /**
     * Returns [Group] leader GUID
     *
     * @return ObjectGuid leaderGUID
     */
    ObjectGuid GetLeaderGUID(Group* group)
    {
        return group->GetLeaderGUID();
    }

    /**
     * Returns the [Group]'s GUID
     *
     * @return ObjectGuid groupGUID
     */
    ObjectGuid GetGUID(Group* group)
    {
        return group->GetGUID();
    }

    /**
     * Returns a [Group] member's GUID by their name
     *
     * @param string name : the [Player]'s name
     * @return ObjectGuid memberGUID
     */
    ObjectGuid GetMemberGUID(Group* group, std::string const& name)
    {
        return group->GetMemberGUID(name);
    }

    /**
     * Returns the member count of this [Group]
     *
     * @return uint32 memberCount
     */
    uint32 GetMembersCount(Group* group)
    {
        return group->GetMembersCount();
    }

    /**
     * Returns the type of this [Group]
     *
     * <pre>
     * enum GroupType
     * {
     *     GROUPTYPE_NORMAL         = 0,
     *     GROUPTYPE_BG             = 1,
     *     GROUPTYPE_RAID           = 2,
     *     GROUPTYPE_LFG_RESTRICTED = 4,
     *     GROUPTYPE_LFG            = 8
     * };
     * </pre>
     *
     * @return [GroupType] groupType
     */
    GroupType GetGroupType(Group* group)
    {
        return group->GetGroupType();
    }

    /**
     * Returns the [Player]'s subgroup ID of this [Group]
     *
     * @param ObjectGuid guid : guid of the player
     * @return uint8 subGroupID : a valid subgroup ID or MAX_RAID_SUBGROUPS+1
     */
    uint8 GetMemberGroup(Group* group, ObjectGuid guid)
    {
        return group->GetMemberGroup(guid);
    }

    /**
     * Sets the leader of this [Group]
     *
     * @param ObjectGuid guid : guid of the new leader
     */
    void SetLeader(Group* group, ObjectGuid guid)
    {
        group->ChangeLeader(guid);
        group->SendUpdate();
    }

    /**
     * Sends a specified [WorldPacket] to this [Group]
     *
     * @param [WorldPacket] packet : the [WorldPacket] to send
     * @param bool ignorePlayersInBg : ignores [Player]s in a battleground
     * @param ObjectGuid ignore : ignore a [Player] by their GUID
     */
    void SendPacket(Group* group, WorldPacket* data, bool ignorePlayersInBg, ObjectGuid ignore)
    {
        group->BroadcastPacket(data, ignorePlayersInBg, -1, ignore);
    }

    /**
     * Removes a [Player] from this [Group] and returns 'true' if successful
     *
     * <pre>
     * enum RemoveMethod
     * {
     *     GROUP_REMOVEMETHOD_DEFAULT  = 0,
     *     GROUP_REMOVEMETHOD_KICK     = 1,
     *     GROUP_REMOVEMETHOD_LEAVE    = 2,
     *     GROUP_REMOVEMETHOD_KICK_LFG = 3
     * };
     * </pre>
     *
     * @param ObjectGuid guid : guid of the player to remove
     * @param [RemoveMethod] method : method used to remove the player
     * @return bool removed
     */
    bool RemoveMember(Group* group, ObjectGuid guid, sol::optional<uint32> method)
    {
        return group->RemoveMember(guid, (RemoveMethod)method.value_or(0));
    }

    /**
     * Disbands this [Group]
     *
     */
    void Disband(Group* group)
    {
        group->Disband();
    }

    /**
     * Converts this [Group] to a raid [Group]
     *
     */
    void ConvertToRaid(Group* group)
    {
        group->ConvertToRaid();
    }

    /**
     * Sets the member's subGroup
     *
     * @param ObjectGuid guid : guid of the player to move
     * @param uint8 groupID : the subGroup's ID
     */
    void SetMembersGroup(Group* group, ObjectGuid guid, uint8 subGroup)
    {
        if (subGroup >= MAX_RAID_SUBGROUPS)
            throw std::invalid_argument("valid subGroup ID expected");

        if (!group->HasFreeSlotSubGroup(subGroup))
            return;

        group->ChangeMembersGroup(guid, subGroup);
    }

    /**
     * Sets the target icon of an object for the [Group]
     *
     * @param uint8 icon : the icon (Skull, Square, etc)
     * @param ObjectGuid target : GUID of the icon target, 0 is to clear the icon
     * @param ObjectGuid setter : GUID of the icon setter
     */
    void SetTargetIcon(Group* group, uint8 icon, ObjectGuid target, sol::optional<ObjectGuid> setter)
    {
        if (icon >= TARGETICONCOUNT)
            throw std::invalid_argument("valid target icon expected");

        group->SetTargetIcon(icon, setter.value_or(ObjectGuid()), target);
    }

    /**
     * Sets or removes a flag for a [Group] member
     *
     * <pre>
     * enum GroupMemberFlags
     * {
     *     MEMBER_FLAG_ASSISTANT   = 0x01,
     *     MEMBER_FLAG_MAINTANK    = 0x02,
     *     MEMBER_FLAG_MAINASSIST  = 0x04,
     * };
     * </pre>
     *
     * @param ObjectGuid target : GUID of the target
     * @param bool apply : add the `flag` if `true`, remove the `flag` otherwise
     * @param [GroupMemberFlags] flag : the flag to set or unset
     */
    void SetMemberFlag(Group* group, ObjectGuid target, bool apply, uint32 flag)
    {
        group->SetGroupMemberFlag(target, apply, static_cast<GroupMemberFlags>(flag));
    }

    /*void ConvertToLFG(Group* group) // TODO: Implementation
    {
        group->ConvertToLFG();
    }*/
}

void RegisterGroupMethods(sol::state& lua)
{
    sol::usertype<GroupRef> type = ALEBind::NewHandleType<GroupRef>(lua, "Group");

    type["IsLeader"]            = ALEBind::Method(&LuaGroup::IsLeader);
    type["IsFull"]              = ALEBind::Method(&LuaGroup::IsFull);
    type["IsLFGGroup"]          = ALEBind::Method(&LuaGroup::IsLFGGroup);
    type["IsRaidGroup"]         = ALEBind::Method(&LuaGroup::IsRaidGroup);
    type["IsBGGroup"]           = ALEBind::Method(&LuaGroup::IsBGGroup);
    type["IsMember"]            = ALEBind::Method(&LuaGroup::IsMember);
    type["IsAssistant"]         = ALEBind::Method(&LuaGroup::IsAssistant);
    type["SameSubGroup"]        = ALEBind::Method(&LuaGroup::SameSubGroup);
    type["HasFreeSlotSubGroup"] = ALEBind::Method(&LuaGroup::HasFreeSlotSubGroup);
    type["AddMember"]           = ALEBind::Method(&LuaGroup::AddMember);
    type["GetMembers"]          = ALEBind::Method(&LuaGroup::GetMembers);
    type["GetLeaderGUID"]       = ALEBind::Method(&LuaGroup::GetLeaderGUID);
    type["GetGUID"]             = ALEBind::Method(&LuaGroup::GetGUID);
    type["GetMemberGUID"]       = ALEBind::Method(&LuaGroup::GetMemberGUID);
    type["GetMembersCount"]     = ALEBind::Method(&LuaGroup::GetMembersCount);
    type["GetGroupType"]        = ALEBind::Method(&LuaGroup::GetGroupType);
    type["GetMemberGroup"]      = ALEBind::Method(&LuaGroup::GetMemberGroup);
    type["SetLeader"]           = ALEBind::Method(&LuaGroup::SetLeader);
    type["SendPacket"]          = ALEBind::Method(&LuaGroup::SendPacket);
    type["RemoveMember"]        = ALEBind::Method(&LuaGroup::RemoveMember);
    type["Disband"]             = ALEBind::Method(&LuaGroup::Disband);
    type["ConvertToRaid"]       = ALEBind::Method(&LuaGroup::ConvertToRaid);
    type["SetMembersGroup"]     = ALEBind::Method(&LuaGroup::SetMembersGroup);
    type["SetTargetIcon"]       = ALEBind::Method(&LuaGroup::SetTargetIcon);
    type["SetMemberFlag"]       = ALEBind::Method(&LuaGroup::SetMemberFlag);
}
