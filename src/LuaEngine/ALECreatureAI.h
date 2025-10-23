/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_CREATURE_AI_H
#define _ALE_CREATURE_AI_H

#include "LuaEngine.h"

struct ScriptedAI;

struct ALECreatureAI : ScriptedAI
{
    // used to delay the spawn hook triggering on AI creation
    bool justSpawned;
    // used to delay movementinform hook (WP hook)
    std::vector< std::pair<uint32, uint32> > movepoints;

    ALECreatureAI(Creature* creature) : ScriptedAI(creature), justSpawned(true)
    {
    }
    ~ALECreatureAI() { }

    //Called at World update tick
    void UpdateAI(uint32 diff) override
    {
        if (justSpawned)
        {
            justSpawned = false;

            JustRespawned();
        }

        if (!movepoints.empty())
        {
            for (auto& point : movepoints)
            {
                if (!sALE->MovementInform(me, point.first, point.second))
                    ScriptedAI::MovementInform(point.first, point.second);
            }
            movepoints.clear();
        }

        if (!sALE->UpdateAI(me, diff))
        {
            if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC))
                ScriptedAI::UpdateAI(diff);
        }
    }

    // Called for reaction when initially engaged - this will always happen _after_ JustEnteredCombat
    // Called at creature aggro either by MoveInLOS or Attack Start
    void JustEngagedWith(Unit* target) override
    {
        if (!sALE->EnterCombat(me, target))
            ScriptedAI::JustEngagedWith(target);
    }

    // Called at any Damage from any attacker (before damage apply)
    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask) override
    {
        if (!sALE->DamageTaken(me, attacker, damage))
        {
            ScriptedAI::DamageTaken(attacker, damage, damagetype, damageSchoolMask);
        }
    }

    //Called at creature death
    void JustDied(Unit* killer) override
    {
        if (!sALE->JustDied(me, killer))
            ScriptedAI::JustDied(killer);
    }

    //Called at creature killing another unit
    void KilledUnit(Unit* victim) override
    {
        if (!sALE->KilledUnit(me, victim))
            ScriptedAI::KilledUnit(victim);
    }

    // Called when the creature summon successfully other creature
    void JustSummoned(Creature* summon) override
    {
        if (!sALE->JustSummoned(me, summon))
            ScriptedAI::JustSummoned(summon);
    }

    // Called when a summoned creature is despawned
    void SummonedCreatureDespawn(Creature* summon) override
    {
        if (!sALE->SummonedCreatureDespawn(me, summon))
            ScriptedAI::SummonedCreatureDespawn(summon);
    }

    //Called at waypoint reached or PointMovement end
    void MovementInform(uint32 type, uint32 id) override
    {
        // delayed since hook triggers before actually reaching the point
        // and starting new movement would bug
        movepoints.push_back(std::make_pair(type, id));
    }

    // Called before EnterCombat even before the creature is in combat.
    void AttackStart(Unit* target) override
    {
        if (!sALE->AttackStart(me, target))
            ScriptedAI::AttackStart(target);
    }

    // Called for reaction at stopping attack at no attackers or targets
    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        if (!sALE->EnterEvadeMode(me))
            ScriptedAI::EnterEvadeMode();
    }

    // Called when creature is spawned or respawned (for reseting variables)
    void JustRespawned() override
    {
        if (!sALE->JustRespawned(me))
            ScriptedAI::JustRespawned();
    }

    // Called at reaching home after evade
    void JustReachedHome() override
    {
        if (!sALE->JustReachedHome(me))
            ScriptedAI::JustReachedHome();
    }

    // Called at text emote receive from player
    void ReceiveEmote(Player* player, uint32 emoteId) override
    {
        if (!sALE->ReceiveEmote(me, player, emoteId))
            ScriptedAI::ReceiveEmote(player, emoteId);
    }

    // called when the corpse of this creature gets removed
    void CorpseRemoved(uint32& respawnDelay) override
    {
        if (!sALE->CorpseRemoved(me, respawnDelay))
            ScriptedAI::CorpseRemoved(respawnDelay);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!sALE->MoveInLineOfSight(me, who))
            ScriptedAI::MoveInLineOfSight(who);
    }

    // Called when hit by a spell
    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (!sALE->SpellHit(me, caster, spell))
            ScriptedAI::SpellHit(caster, spell);
    }

    // Called when spell hits a target
    void SpellHitTarget(Unit* target, SpellInfo const* spell) override
    {
        if (!sALE->SpellHitTarget(me, target, spell))
            ScriptedAI::SpellHitTarget(target, spell);
    }

    // Called when the creature is summoned successfully by other creature
    void IsSummonedBy(WorldObject* summoner) override
    {
        if (!summoner->ToUnit() || !sALE->OnSummoned(me, summoner->ToUnit()))
            ScriptedAI::IsSummonedBy(summoner);
    }

    void SummonedCreatureDies(Creature* summon, Unit* killer) override
    {
        if (!sALE->SummonedCreatureDies(me, summon, killer))
            ScriptedAI::SummonedCreatureDies(summon, killer);
    }

    // Called when owner takes damage
    void OwnerAttackedBy(Unit* attacker) override
    {
        if (!sALE->OwnerAttackedBy(me, attacker))
            ScriptedAI::OwnerAttackedBy(attacker);
    }

    // Called when owner attacks something
    void OwnerAttacked(Unit* target) override
    {
        if (!sALE->OwnerAttacked(me, target))
            ScriptedAI::OwnerAttacked(target);
    }
};

#endif
