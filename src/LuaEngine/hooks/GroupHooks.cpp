/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALETemplate.h"

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
    Push(group);
    Push(guid);
    CallAllFunctions(GroupEventBindings, key);
}

void ALE::OnInviteMember(Group* group, ObjectGuid guid)
{
    START_HOOK(GROUP_EVENT_ON_MEMBER_INVITE);
    Push(group);
    Push(guid);
    CallAllFunctions(GroupEventBindings, key);
}

void ALE::OnRemoveMember(Group* group, ObjectGuid guid, uint8 method)
{
    START_HOOK(GROUP_EVENT_ON_MEMBER_REMOVE);
    Push(group);
    Push(guid);
    Push(method);
    CallAllFunctions(GroupEventBindings, key);
}

void ALE::OnChangeLeader(Group* group, ObjectGuid newLeaderGuid, ObjectGuid oldLeaderGuid)
{
    START_HOOK(GROUP_EVENT_ON_LEADER_CHANGE);
    Push(group);
    Push(newLeaderGuid);
    Push(oldLeaderGuid);
    CallAllFunctions(GroupEventBindings, key);
}

void ALE::OnDisband(Group* group)
{
    START_HOOK(GROUP_EVENT_ON_DISBAND);
    Push(group);
    CallAllFunctions(GroupEventBindings, key);
}

void ALE::OnCreate(Group* group, ObjectGuid leaderGuid, GroupType groupType)
{
    START_HOOK(GROUP_EVENT_ON_CREATE);
    Push(group);
    Push(leaderGuid);
    Push(groupType);
    CallAllFunctions(GroupEventBindings, key);
}
