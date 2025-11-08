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

void ALE::OnAllCreatureAuraApply(Creature* me, Aura* aura)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_AURA_APPLY);
    Push(me);
    Push(aura);
    CallAllFunctions(AllCreatureEventBindings, key);
}

void ALE::OnAllCreatureAuraRemove(Creature* me, Aura* aura, AuraRemoveMode mode)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_AURA_REMOVE);
    Push(me);
    Push(aura);
    Push(mode);
    CallAllFunctions(AllCreatureEventBindings, key);
}

void ALE::OnAllCreatureHeal(Creature* me, Unit* target, uint32& gain)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_HEAL);
    Push(me);
    Push(target);
    Push(gain);

    int gainIndex = lua_gettop(L);
    int n = SetupStack(AllCreatureEventBindings, key, 3);
    while (n > 0)
    {
        int r = CallOneFunction(n--, 3, 1);
        if (lua_isnumber(L, r))
        {
            gain = CHECKVAL<uint32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(gain, gainIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(3);
}

void ALE::OnAllCreatureDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_DAMAGE);
    Push(me);
    Push(target);
    Push(damage);

    int damageIndex = lua_gettop(L);
    int n = SetupStack(AllCreatureEventBindings, key, 3);
    while (n > 0)
    {
        int r = CallOneFunction(n--, 3, 1);
        if (lua_isnumber(L, r))
        {
            damage = CHECKVAL<uint32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(damage, damageIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(3);
}

void ALE::OnAllCreatureModifyPeriodicDamageAurasTick(Creature* me, Unit* target, uint32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK);
    Push(me);
    Push(target);
    Push(damage);
    Push(spellInfo);

    int damageIndex = lua_gettop(L) - 1;
    int n = SetupStack(AllCreatureEventBindings, key, 4);
    while (n > 0)
    {
        int r = CallOneFunction(n--, 4, 1);
        if (lua_isnumber(L, r))
        {
            damage = CHECKVAL<uint32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(damage, damageIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(4);
}

void ALE::OnAllCreatureModifyMeleeDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE);
    Push(me);
    Push(target);
    Push(damage);

    int damageIndex = lua_gettop(L);
    int n = SetupStack(AllCreatureEventBindings, key, 3);
    while (n > 0)
    {
        int r = CallOneFunction(n--, 3, 1);
        if (lua_isnumber(L, r))
        {
            damage = CHECKVAL<uint32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(damage, damageIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(3);
}

void ALE::OnAllCreatureModifySpellDamageTaken(Creature* me, Unit* target, int32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN);
    Push(me);
    Push(target);
    Push(damage);
    Push(spellInfo);

    int damageIndex = lua_gettop(L) - 1;
    int n = SetupStack(AllCreatureEventBindings, key, 4);
    while (n > 0)
    {
        int r = CallOneFunction(n--, 4, 1);
        if (lua_isnumber(L, r))
        {
            damage = CHECKVAL<int32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(damage, damageIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(4);
}

void ALE::OnAllCreatureModifyHealReceived(Creature* me, Unit* target, uint32& heal, SpellInfo const* spellInfo)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED);
    Push(me);
    Push(target);
    Push(heal);
    Push(spellInfo);

    int healIndex = lua_gettop(L) - 1;
    int n = SetupStack(AllCreatureEventBindings, key, 4);
    while (n > 0)
    {
        int r = CallOneFunction(n--, 4, 1);
        if (lua_isnumber(L, r))
        {
            heal = CHECKVAL<uint32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(heal, healIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(4);
}

uint32 ALE::OnAllCreatureDealDamage(Creature* me, Unit* target, uint32 damage, DamageEffectType damagetype)
{
    START_HOOK_WITH_RETVAL(ALL_CREATURE_EVENT_ON_DEAL_DAMAGE, damage);
    uint32 result = damage;
    Push(me);
    Push(target);
    Push(damage);
    Push(damagetype);
    int damageIndex = lua_gettop(L) - 1;
    int n = SetupStack(AllCreatureEventBindings, key, 4);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 4, 1);

        if (lua_isnumber(L, r))
        {
            result = CHECKVAL<uint32>(L, r);
            // Update the stack for subsequent calls.
            ReplaceArgument(result, damageIndex);
        }

        lua_pop(L, 1);
    }

    CleanUpStack(4);
    return result;
}
