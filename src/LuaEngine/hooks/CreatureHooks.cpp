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

#define START_HOOK(EVENT, CREATURE) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto entry_key = EntryKey<CreatureEvents>(EVENT, CREATURE->GetEntry());\
    auto unique_key = UniqueObjectKey<CreatureEvents>(EVENT, CREATURE->GET_GUID(), CREATURE->GetInstanceId());\
    if (!CreatureEventBindings->HasBindingsFor(entry_key))\
        if (!CreatureUniqueBindings->HasBindingsFor(unique_key))\
            return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, CREATURE, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto entry_key = EntryKey<CreatureEvents>(EVENT, CREATURE->GetEntry());\
    auto unique_key = UniqueObjectKey<CreatureEvents>(EVENT, CREATURE->GET_GUID(), CREATURE->GetInstanceId());\
    if (!CreatureEventBindings->HasBindingsFor(entry_key))\
        if (!CreatureUniqueBindings->HasBindingsFor(unique_key))\
            return RETVAL;\
    LOCK_ALE

void ALE::OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, Creature* pTarget)
{
    START_HOOK(CREATURE_EVENT_ON_DUMMY_EFFECT, pTarget);
    Push(pCaster);
    Push(spellId);
    Push(effIndex);
    Push(pTarget);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool ALE::OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_QUEST_ACCEPT, pCreature, false);
    Push(pPlayer);
    Push(pCreature);
    Push(pQuest);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool ALE::OnQuestReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest, uint32 opt)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_QUEST_REWARD, pCreature, false);
    Push(pPlayer);
    Push(pCreature);
    Push(pQuest);
    Push(opt);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void ALE::GetDialogStatus(const Player* pPlayer, const Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_DIALOG_STATUS, pCreature);
    Push(pPlayer);
    Push(pCreature);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void ALE::OnAddToWorld(Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_ADD, pCreature);
    Push(pCreature);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void ALE::OnRemoveFromWorld(Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_REMOVE, pCreature);
    Push(pCreature);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool ALE::OnSummoned(Creature* pCreature, Unit* pSummoner)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED, pCreature, false);
    Push(pCreature);
    Push(pSummoner);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool ALE::UpdateAI(Creature* me, const uint32 diff)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_AIUPDATE, me, false);
    Push(me);
    Push(diff);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

//Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
//Called at creature aggro either by MoveInLOS or Attack Start
bool ALE::EnterCombat(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_ENTER_COMBAT, me, false);
    Push(me);
    Push(target);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called at any Damage from any attacker (before damage apply)
bool ALE::DamageTaken(Creature* me, Unit* attacker, uint32& damage)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DAMAGE_TAKEN, me, false);
    bool result = false;
    Push(me);
    Push(attacker);
    Push(damage);
    int damageIndex = lua_gettop(L);
    int n = SetupStack(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key, 3);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 3, 2);

        if (lua_isboolean(L, r + 0) && lua_toboolean(L, r + 0))
            result = true;

        if (lua_isnumber(L, r + 1))
        {
            damage = ALE::CHECKVAL<uint32>(L, r + 1);
            // Update the stack for subsequent calls.
            ReplaceArgument(damage, damageIndex);
        }

        lua_pop(L, 2);
    }

    CleanUpStack(3);
    return result;
}

//Called at creature death
bool ALE::JustDied(Creature* me, Unit* killer)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DIED, me, false);
    Push(me);
    Push(killer);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

//Called at creature killing another unit
bool ALE::KilledUnit(Creature* me, Unit* victim)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_TARGET_DIED, me, false);
    Push(me);
    Push(victim);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when the creature summon successfully other creature
bool ALE::JustSummoned(Creature* me, Creature* summon)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE, me, false);
    Push(me);
    Push(summon);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when a summoned creature is despawned
bool ALE::SummonedCreatureDespawn(Creature* me, Creature* summon)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN, me, false);
    Push(me);
    Push(summon);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

//Called at waypoint reached or PointMovement end
bool ALE::MovementInform(Creature* me, uint32 type, uint32 id)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_REACH_WP, me, false);
    Push(me);
    Push(type);
    Push(id);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called before EnterCombat even before the creature is in combat.
bool ALE::AttackStart(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_PRE_COMBAT, me, false);
    Push(me);
    Push(target);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called for reaction at stopping attack at no attackers or targets
bool ALE::EnterEvadeMode(Creature* me)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_LEAVE_COMBAT, me, false);
    Push(me);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when creature is spawned or respawned (for reseting variables)
bool ALE::JustRespawned(Creature* me)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SPAWN, me, false);
    Push(me);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called at reaching home after evade
bool ALE::JustReachedHome(Creature* me)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_REACH_HOME, me, false);
    Push(me);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called at text emote receive from player
bool ALE::ReceiveEmote(Creature* me, Player* player, uint32 emoteId)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_RECEIVE_EMOTE, me, false);
    Push(me);
    Push(player);
    Push(emoteId);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// called when the corpse of this creature gets removed
bool ALE::CorpseRemoved(Creature* me, uint32& respawnDelay)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_CORPSE_REMOVED, me, false);
    bool result = false;
    Push(me);
    Push(respawnDelay);
    int respawnDelayIndex = lua_gettop(L);
    int n = SetupStack(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key, 2);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 2, 2);

        if (lua_isboolean(L, r + 0) && lua_toboolean(L, r + 0))
            result = true;

        if (lua_isnumber(L, r + 1))
        {
            respawnDelay = ALE::CHECKVAL<uint32>(L, r + 1);
            // Update the stack for subsequent calls.
            ReplaceArgument(respawnDelay, respawnDelayIndex);
        }

        lua_pop(L, 2);
    }

    CleanUpStack(2);
    return result;
}

bool ALE::MoveInLineOfSight(Creature* me, Unit* who)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_MOVE_IN_LOS, me, false);
    Push(me);
    Push(who);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called on creature initial spawn, respawn, death, evade (leave combat)
void ALE::On_Reset(Creature* me) // Not an override, custom
{
    START_HOOK(CREATURE_EVENT_ON_RESET, me);
    Push(me);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when hit by a spell
bool ALE::SpellHit(Creature* me, WorldObject* caster, SpellInfo const* spell)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_HIT_BY_SPELL, me, false);
    Push(me);
    Push(caster);
    Push(spell->Id); // Pass spell object?
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when spell hits a target
bool ALE::SpellHitTarget(Creature* me, WorldObject* target, SpellInfo const* spell)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SPELL_HIT_TARGET, me, false);
    Push(me);
    Push(target);
    Push(spell->Id); // Pass spell object?
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool ALE::SummonedCreatureDies(Creature* me, Creature* summon, Unit* killer)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED, me, false);
    Push(me);
    Push(summon);
    Push(killer);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when owner takes damage
bool ALE::OwnerAttackedBy(Creature* me, Unit* attacker)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_OWNER_ATTACKED_AT, me, false);
    Push(me);
    Push(attacker);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when owner attacks something
bool ALE::OwnerAttacked(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_OWNER_ATTACKED, me, false);
    Push(me);
    Push(target);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void ALE::OnCreatureAuraApply(Creature* me, Aura* aura)
{
    START_HOOK(CREATURE_EVENT_ON_AURA_APPLY, me);
    Push(me);
    Push(aura);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void ALE::OnCreatureHeal(Creature* me, Unit* target, uint32& gain)
{
    START_HOOK(CREATURE_EVENT_ON_HEAL, me);
    Push(me);
    Push(target);
    Push(gain);

    int gainIndex = lua_gettop(L);
    int n = SetupStack(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key, 3);
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

void ALE::OnCreatureDamage(Creature* me, Unit* target, uint32& damage)
{
    START_HOOK(CREATURE_EVENT_ON_DAMAGE, me);
    Push(me);
    Push(target);
    Push(damage);

    int damageIndex = lua_gettop(L);
    int n = SetupStack(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key, 3);
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