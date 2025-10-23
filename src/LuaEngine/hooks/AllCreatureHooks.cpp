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

using namespace Hooks;

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<AllCreatureEvents>(EVENT);\
    if (!AllCreatureEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EventKey<AllCreatureEvents>(EVENT);\
    if (!AllCreatureEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

void ALE::OnAllCreatureAddToWorld(Creature* creature)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_ADD);
    Push(creature);
    CallAllFunctions(AllCreatureEventBindings, key);
}

void ALE::OnAllCreatureRemoveFromWorld(Creature* creature)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_REMOVE);
    Push(creature);
    CallAllFunctions(AllCreatureEventBindings, key);
}

void ALE::OnAllCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_SELECT_LEVEL);
    Push(cinfo);
    Push(creature);
    CallAllFunctions(AllCreatureEventBindings, key);
}

void ALE::OnAllCreatureBeforeSelectLevel(const CreatureTemplate* cinfo, Creature* creature, uint8& level)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_BEFORE_SELECT_LEVEL);
    Push(cinfo);
    Push(creature);
    Push(level);
    int levelIndex = lua_gettop(L);
    int n = SetupStack(AllCreatureEventBindings, key, 3);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 3, 1);

        if (lua_isnumber(L, r))
        {
            level = CHECKVAL<uint8>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(level, levelIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(3);
}