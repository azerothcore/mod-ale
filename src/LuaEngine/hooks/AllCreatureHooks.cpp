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
    CallAll(*AllCreatureEventBindings, key, creature);
}

void ALE::OnAllCreatureRemoveFromWorld(Creature* creature)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_REMOVE);
    CallAll(*AllCreatureEventBindings, key, creature);
}

void ALE::OnAllCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_SELECT_LEVEL);
    CallAll(*AllCreatureEventBindings, key, cinfo, creature);
}

void ALE::OnAllCreatureBeforeSelectLevel(const CreatureTemplate* cinfo, Creature* creature, uint8& level)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_BEFORE_SELECT_LEVEL);
    level = CallAllFold(*AllCreatureEventBindings, key, level, [&](auto const& callback, uint8 current)
    {
        return Call(callback, key.event_id, cinfo, creature, current);
    });
}

void ALE::OnAllCreatureAuraApply(Creature* me, Aura* aura)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_AURA_APPLY);
    CallAll(*AllCreatureEventBindings, key, me, aura);
}

void ALE::OnAllCreatureAuraRemove(Creature* me, Aura* aura, AuraRemoveMode mode)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_AURA_REMOVE);
    CallAll(*AllCreatureEventBindings, key, me, aura, mode);
}

void ALE::OnAllCreatureHeal(Creature* me, Unit* target, uint32& gain)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_HEAL);
    gain = CallAllFold(*AllCreatureEventBindings, key, gain, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, me, target, current);
    });
}

void ALE::OnAllCreatureDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_DAMAGE);
    damage = CallAllFold(*AllCreatureEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, me, target, current);
    });
}

void ALE::OnAllCreatureModifyPeriodicDamageAurasTick(Creature* me, Unit* target, uint32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK);
    damage = CallAllFold(*AllCreatureEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, me, target, current, spellInfo);
    });
}

void ALE::OnAllCreatureModifyMeleeDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE);
    damage = CallAllFold(*AllCreatureEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, me, target, current);
    });
}

void ALE::OnAllCreatureModifySpellDamageTaken(Creature* me, Unit* target, int32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN);
    damage = CallAllFold(*AllCreatureEventBindings, key, damage, [&](auto const& callback, int32 current)
    {
        return Call(callback, key.event_id, me, target, current, spellInfo);
    });
}

void ALE::OnAllCreatureModifyHealReceived(Creature* me, Unit* target, uint32& heal, SpellInfo const* spellInfo)
{
    START_HOOK(ALL_CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED);
    heal = CallAllFold(*AllCreatureEventBindings, key, heal, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, me, target, current, spellInfo);
    });
}

uint32 ALE::OnAllCreatureDealDamage(Creature* me, Unit* target, uint32 damage, DamageEffectType damagetype)
{
    START_HOOK_WITH_RETVAL(ALL_CREATURE_EVENT_ON_DEAL_DAMAGE, damage);
    return CallAllFold(*AllCreatureEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, me, target, current, damagetype);
    });
}
