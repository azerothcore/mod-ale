/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"

using namespace Hooks;

// Creature events can be bound to every creature of an entry and/or to one
// specific spawn (see RegisterCreatureEvent / RegisterUniqueCreatureEvent).
#define START_HOOK(EVENT, CREATURE) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto entry_key = EntryKey<CreatureEvents>(EVENT, CREATURE->GetEntry());\
    auto unique_key = UniqueObjectKey<CreatureEvents>(EVENT, CREATURE->GetGUID(), CREATURE->GetInstanceId());\
    if (!CreatureEventBindings->HasBindingsFor(entry_key) && !CreatureUniqueBindings->HasBindingsFor(unique_key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, CREATURE, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto entry_key = EntryKey<CreatureEvents>(EVENT, CREATURE->GetEntry());\
    auto unique_key = UniqueObjectKey<CreatureEvents>(EVENT, CREATURE->GetGUID(), CREATURE->GetInstanceId());\
    if (!CreatureEventBindings->HasBindingsFor(entry_key) && !CreatureUniqueBindings->HasBindingsFor(unique_key))\
        return RETVAL;\
    LOCK_ALE

void ALE::OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, Creature* pTarget)
{
    START_HOOK(CREATURE_EVENT_ON_DUMMY_EFFECT, pTarget);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, pCaster, spellId, effIndex, pTarget);
}

bool ALE::OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_QUEST_ACCEPT, pCreature, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, pPlayer, pCreature, pQuest);
}

bool ALE::OnQuestReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest, uint32 opt)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_QUEST_REWARD, pCreature, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, pPlayer, pCreature, pQuest, opt);
}

void ALE::GetDialogStatus(const Player* pPlayer, const Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_DIALOG_STATUS, pCreature);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, pPlayer, pCreature);
}

void ALE::OnAddToWorld(Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_ADD, pCreature);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, pCreature);
}

void ALE::OnRemoveFromWorld(Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_REMOVE, pCreature);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, pCreature);
}

bool ALE::OnSummoned(Creature* pCreature, Unit* pSummoner)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED, pCreature, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, pCreature, pSummoner);
}

bool ALE::UpdateAI(Creature* me, const uint32 diff)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_AIUPDATE, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, diff);
}

//Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
//Called at creature aggro either by MoveInLOS or Attack Start
bool ALE::EnterCombat(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_ENTER_COMBAT, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, target);
}

// Called at any Damage from any attacker (before damage apply)
bool ALE::DamageTaken(Creature* me, Unit* attacker, uint32& damage)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DAMAGE_TAKEN, me, false);
    bool result = false;

    // Handlers return (override, newDamage): `override` stops the default
    // behaviour, `newDamage` feeds the handlers after it and the damage apply.
    for (sol::protected_function const& callback : GetCreatureCallbacks(entry_key, unique_key))
    {
        sol::protected_function_result callResult = Call(callback, entry_key.event_id, me, attacker, damage);
        if (!callResult.valid())
            continue;

        if (callResult.get<sol::optional<bool>>(0) == sol::optional<bool>(true))
            result = true;

        if (sol::optional<uint32> newDamage = callResult.get<sol::optional<uint32>>(1))
            damage = *newDamage;
    }

    return result;
}

//Called at creature death
bool ALE::JustDied(Creature* me, Unit* killer)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DIED, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, killer);
}

//Called at creature killing another unit
bool ALE::KilledUnit(Creature* me, Unit* victim)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_TARGET_DIED, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, victim);
}

// Called when the creature summon successfully other creature
bool ALE::JustSummoned(Creature* me, Creature* summon)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, summon);
}

// Called when a summoned creature is despawned
bool ALE::SummonedCreatureDespawn(Creature* me, Creature* summon)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, summon);
}

//Called at waypoint reached or PointMovement end
bool ALE::MovementInform(Creature* me, uint32 type, uint32 id)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_REACH_WP, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, type, id);
}

// Called before EnterCombat even before the creature is in combat.
bool ALE::AttackStart(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_PRE_COMBAT, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, target);
}

// Called for reaction at stopping attack at no attackers or targets
bool ALE::EnterEvadeMode(Creature* me)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_LEAVE_COMBAT, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me);
}

// Called when creature is spawned or respawned (for reseting variables)
bool ALE::JustRespawned(Creature* me)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SPAWN, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me);
}

// Called at reaching home after evade
bool ALE::JustReachedHome(Creature* me)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_REACH_HOME, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me);
}

// Called at text emote receive from player
bool ALE::ReceiveEmote(Creature* me, Player* player, uint32 emoteId)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_RECEIVE_EMOTE, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, player, emoteId);
}

// called when the corpse of this creature gets removed
bool ALE::CorpseRemoved(Creature* me, uint32& respawnDelay)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_CORPSE_REMOVED, me, false);
    bool result = false;

    // Handlers return (override, newRespawnDelay).
    for (sol::protected_function const& callback : GetCreatureCallbacks(entry_key, unique_key))
    {
        sol::protected_function_result callResult = Call(callback, entry_key.event_id, me, respawnDelay);
        if (!callResult.valid())
            continue;

        if (callResult.get<sol::optional<bool>>(0) == sol::optional<bool>(true))
            result = true;

        if (sol::optional<uint32> newDelay = callResult.get<sol::optional<uint32>>(1))
            respawnDelay = *newDelay;
    }

    return result;
}

bool ALE::MoveInLineOfSight(Creature* me, Unit* who)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_MOVE_IN_LOS, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, who);
}

// Called on creature initial spawn, respawn, death, evade (leave combat)
void ALE::On_Reset(Creature* me) // Not an override, custom
{
    START_HOOK(CREATURE_EVENT_ON_RESET, me);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, me);
}

// Called when hit by a spell
bool ALE::SpellHit(Creature* me, WorldObject* caster, SpellInfo const* spell)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_HIT_BY_SPELL, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, caster, spell->Id);
}

// Called when spell hits a target
bool ALE::SpellHitTarget(Creature* me, WorldObject* target, SpellInfo const* spell)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SPELL_HIT_TARGET, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, target, spell->Id);
}

bool ALE::SummonedCreatureDies(Creature* me, Creature* summon, Unit* killer)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, summon, killer);
}

// Called when owner takes damage
bool ALE::OwnerAttackedBy(Creature* me, Unit* attacker)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_OWNER_ATTACKED_AT, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, attacker);
}

// Called when owner attacks something
bool ALE::OwnerAttacked(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_OWNER_ATTACKED, me, false);
    return CallAllBool(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, false, me, target);
}

void ALE::OnCreatureAuraApply(Creature* me, Aura* aura)
{
    START_HOOK(CREATURE_EVENT_ON_AURA_APPLY, me);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, me, aura);
}

void ALE::OnCreatureAuraRemove(Creature* me, Aura* aura, AuraRemoveMode mode)
{
    START_HOOK(CREATURE_EVENT_ON_AURA_REMOVE, me);
    CallAll(*CreatureEventBindings, *CreatureUniqueBindings, entry_key, unique_key, me, aura, mode);
}

void ALE::OnCreatureHeal(Creature* me, Unit* target, uint32& gain)
{
    START_HOOK(CREATURE_EVENT_ON_HEAL, me);
    gain = FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), gain, [&](auto const& callback, uint32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current);
    });
}

void ALE::OnCreatureDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(CREATURE_EVENT_ON_DAMAGE, me);
    damage = FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current);
    });
}

void ALE::OnCreatureModifyPeriodicDamageAurasTick(Creature* me, Unit* target, uint32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(CREATURE_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK, me);
    damage = FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current, spellInfo);
    });
}

void ALE::OnCreatureModifyMeleeDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(CREATURE_EVENT_ON_MODIFY_MELEE_DAMAGE, me);
    damage = FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current);
    });
}

void ALE::OnCreatureModifySpellDamageTaken(Creature* me, Unit* target, int32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(CREATURE_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN, me);
    damage = FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), damage, [&](auto const& callback, int32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current, spellInfo);
    });
}

void ALE::OnCreatureModifyHealReceived(Creature* me, Unit* target, uint32& heal, SpellInfo const* spellInfo)
{
    START_HOOK(CREATURE_EVENT_ON_MODIFY_HEAL_RECEIVED, me);
    heal = FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), heal, [&](auto const& callback, uint32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current, spellInfo);
    });
}

uint32 ALE::OnCreatureDealDamage(Creature* me, Unit* target, uint32 damage, DamageEffectType damagetype)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DEAL_DAMAGE, me, damage);
    return FoldCallbacks(GetCreatureCallbacks(entry_key, unique_key), damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, entry_key.event_id, me, target, current, damagetype);
    });
}
