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
    auto key = EventKey<GroupEvents>(EVENT);\
    if (!GroupEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

void ALE::OnAddMember(Group* group, ObjectGuid guid)
{
    START_HOOK(GROUP_EVENT_ON_MEMBER_ADD);
    CallAll(*GroupEventBindings, key, group, guid);
}

void ALE::OnInviteMember(Group* group, ObjectGuid guid)
{
    START_HOOK(GROUP_EVENT_ON_MEMBER_INVITE);
    CallAll(*GroupEventBindings, key, group, guid);
}

void ALE::OnRemoveMember(Group* group, ObjectGuid guid, uint8 method)
{
    START_HOOK(GROUP_EVENT_ON_MEMBER_REMOVE);
    CallAll(*GroupEventBindings, key, group, guid, method);
}

void ALE::OnChangeLeader(Group* group, ObjectGuid newLeaderGuid, ObjectGuid oldLeaderGuid)
{
    START_HOOK(GROUP_EVENT_ON_LEADER_CHANGE);
    CallAll(*GroupEventBindings, key, group, newLeaderGuid, oldLeaderGuid);
}

void ALE::OnDisband(Group* group)
{
    START_HOOK(GROUP_EVENT_ON_DISBAND);
    CallAll(*GroupEventBindings, key, group);
}

void ALE::OnCreate(Group* group, ObjectGuid leaderGuid, GroupType groupType)
{
    START_HOOK(GROUP_EVENT_ON_CREATE);
    CallAll(*GroupEventBindings, key, group, leaderGuid, groupType);
}
