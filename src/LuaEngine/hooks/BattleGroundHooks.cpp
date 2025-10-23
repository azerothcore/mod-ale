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
    auto key = EventKey<BGEvents>(EVENT);\
    if (!BGEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

void ALE::OnBGStart(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId)
{
    START_HOOK(BG_EVENT_ON_START);
    Push(bg);
    Push(bgId);
    Push(instanceId);
    CallAllFunctions(BGEventBindings, key);
}

void ALE::OnBGEnd(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId, TeamId winner)
{
    START_HOOK(BG_EVENT_ON_END);
    Push(bg);
    Push(bgId);
    Push(instanceId);
    Push(winner);
    CallAllFunctions(BGEventBindings, key);
}

void ALE::OnBGCreate(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId)
{
    START_HOOK(BG_EVENT_ON_CREATE);
    Push(bg);
    Push(bgId);
    Push(instanceId);
    CallAllFunctions(BGEventBindings, key);
}

void ALE::OnBGDestroy(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId)
{
    START_HOOK(BG_EVENT_ON_PRE_DESTROY);
    Push(bg);
    Push(bgId);
    Push(instanceId);
    CallAllFunctions(BGEventBindings, key);
}
