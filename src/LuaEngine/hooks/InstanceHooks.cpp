/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEIncludes.h"
#include "ALETemplate.h"
#include "ALEInstanceAI.h"

using namespace Hooks;

#define START_HOOK(EVENT, AI) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto mapKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetId());\
    auto instanceKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetInstanceId());\
    if (!MapEventBindings->HasBindingsFor(mapKey) && !InstanceEventBindings->HasBindingsFor(instanceKey))\
        return;\
    LOCK_ALE;\
    PushInstanceData(L, AI);\
    Push(AI->instance)

#define START_HOOK_WITH_RETVAL(EVENT, AI, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto mapKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetId());\
    auto instanceKey = EntryKey<InstanceEvents>(EVENT, AI->instance->GetInstanceId());\
    if (!MapEventBindings->HasBindingsFor(mapKey) && !InstanceEventBindings->HasBindingsFor(instanceKey))\
        return RETVAL;\
    LOCK_ALE;\
    PushInstanceData(L, AI);\
    Push(AI->instance)

void ALE::OnInitialize(ALEInstanceAI* ai)
{
    START_HOOK(INSTANCE_EVENT_ON_INITIALIZE, ai);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void ALE::OnLoad(ALEInstanceAI* ai)
{
    START_HOOK(INSTANCE_EVENT_ON_LOAD, ai);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void ALE::OnUpdateInstance(ALEInstanceAI* ai, uint32 diff)
{
    START_HOOK(INSTANCE_EVENT_ON_UPDATE, ai);
    Push(diff);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void ALE::OnPlayerEnterInstance(ALEInstanceAI* ai, Player* player)
{
    START_HOOK(INSTANCE_EVENT_ON_PLAYER_ENTER, ai);
    Push(player);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void ALE::OnCreatureCreate(ALEInstanceAI* ai, Creature* creature)
{
    START_HOOK(INSTANCE_EVENT_ON_CREATURE_CREATE, ai);
    Push(creature);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

void ALE::OnGameObjectCreate(ALEInstanceAI* ai, GameObject* gameobject)
{
    START_HOOK(INSTANCE_EVENT_ON_GAMEOBJECT_CREATE, ai);
    Push(gameobject);
    CallAllFunctions(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}

bool ALE::OnCheckEncounterInProgress(ALEInstanceAI* ai)
{
    START_HOOK_WITH_RETVAL(INSTANCE_EVENT_ON_CHECK_ENCOUNTER_IN_PROGRESS, ai, false);
    return CallAllFunctionsBool(MapEventBindings, InstanceEventBindings, mapKey, instanceKey);
}
