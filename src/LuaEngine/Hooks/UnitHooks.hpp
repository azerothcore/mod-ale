/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_UNIT_HOOKS_H
#define _ALE_UNIT_HOOKS_H

#include "ScriptMgr.h"

#include "TimedEventManager.h"
#include "EventManager.h"
#include "PlayerHooks.hpp"

namespace ALE::Hooks
{
    /**
     * @class UnitHooks
     * @brief AzerothCore unit event interceptor for ALE Lua system
     * @note Registered in ALEScriptLoader via sScriptMgr->AddScript<UnitHooks>()
     */
    class UnitHooks : public UnitScript
    {
    public:
        /**
         * @brief Constructor - registers UnitHooks with AzerothCore
         */
        UnitHooks() : UnitScript("ALE_UnitHooks") { }

        void OnAuraApply(Unit* unit, Aura* aura) override
        {
            if(!unit || !aura)
                return;

            if(unit->IsPlayer())
                TriggerPlayerEvent(Hooks::PlayerEvent::ON_AURA_APPLY, unit->ToPlayer(), aura);

            // if (unit->IsCreature())
            // {
            //     Creature* creature = unit->ToCreature();
            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_AURA_APPLY, creature, aura);
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_AURA_APPLY, creature, aura);
            // }
        }

        void OnAuraRemove(Unit* unit, AuraApplication* auraApp, AuraRemoveMode mode) override
        {
            if(!unit || !auraApp)
                return;

            if(unit->IsPlayer())
                TriggerPlayerEvent(Hooks::PlayerEvent::ON_AURA_REMOVE, unit->ToPlayer(), auraApp, mode);

            // if (unit->IsCreature())
            // {
            //     Creature* creature = unit->ToCreature();
            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_AURA_REMOVE, creature, auraApp, mode);
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_AURA_REMOVE, creature, auraApp, mode);
            // }
        }

        void OnHeal(Unit* healer, Unit* receiver, uint32& gain) override
        {
            if(!healer || !receiver)
                return;

            if(healer->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_HEAL);
                gain = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                    Hooks::PlayerEvent::ON_HEAL,
                    gain,
                    eventId,
                    healer->ToPlayer(),
                    receiver,
                    gain
                );
            }

            // if (healer->IsCreature())
            // {
            //     Creature* creature = healer->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_HEAL);
            //     gain = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
            //         Hooks::AllCreatureEvent::ON_HEAL,
            //         gain,
            //         allCreatureEventId,
            //         creature,
            //         receiver,
            //         gain
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_HEAL);
            //     gain = Core::EventManager::GetInstance().TriggerEntryEventWithReturn<uint32>(
            //         Hooks::CreatureEvent::ON_HEAL,
            //         creature,
            //         gain,
            //         creatureEventId,
            //         creature,
            //         receiver,
            //         gain
            //     );
            // }
        }

        void OnDamage(Unit* attacker, Unit* receiver, uint32& damage) override
        {
            if(!attacker || !receiver)
                return;

            if(attacker->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_DAMAGE);
                damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                    Hooks::PlayerEvent::ON_DAMAGE,
                    damage,
                    eventId,
                    attacker->ToPlayer(),
                    receiver,
                    damage
                );
            }

            // if (attacker->IsCreature())
            // {
            //     Creature* creature = attacker->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_DAMAGE);
            //     damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
            //         Hooks::AllCreatureEvent::ON_DAMAGE,
            //         damage,
            //         allCreatureEventId,
            //         creature,
            //         receiver,
            //         damage
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_DAMAGE);
            //     damage = Core::EventManager::GetInstance().TriggerEntryEventWithReturn<uint32>(
            //         Hooks::CreatureEvent::ON_DAMAGE,
            //         creature,
            //         damage,
            //         creatureEventId,
            //         creature,
            //         receiver,
            //         damage
            //     );
            // }
        }

        void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* spellInfo) override
        {
            if(!target || !attacker || !spellInfo)
                return;

            if(attacker->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_PERIODIC_DAMAGE_TICK);
                damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                    Hooks::PlayerEvent::ON_PERIODIC_DAMAGE_TICK,
                    damage,
                    eventId,
                    attacker->ToPlayer(),
                    target,
                    damage,
                    spellInfo
                );
            }

            // if (attacker->IsCreature())
            // {
            //     Creature* creature = attacker->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_PERIODIC_DAMAGE_TICK);
            //     damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
            //         Hooks::AllCreatureEvent::ON_PERIODIC_DAMAGE_TICK,
            //         damage,
            //         allCreatureEventId,
            //         creature,
            //         target,
            //         damage,
            //         spellInfo
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_PERIODIC_DAMAGE_TICK);
            //     damage = Core::EventManager::GetInstance().TriggerEntryEventWithReturn<uint32>(
            //         Hooks::CreatureEvent::ON_PERIODIC_DAMAGE_TICK,
            //         creature,
            //         damage,
            //         creatureEventId,
            //         creature,
            //         target,
            //         damage,
            //         spellInfo
            //     );
            // }
        }

        void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
        {
            if(!target || !attacker)
                return;

            if(attacker->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_MELEE_DAMAGE);
                damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                    Hooks::PlayerEvent::ON_MELEE_DAMAGE,
                    damage,
                    eventId,
                    attacker->ToPlayer(),
                    target,
                    damage
                );
            }

            // if (attacker->IsCreature())
            // {
            //     Creature* creature = attacker->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_MELEE_DAMAGE);
            //     damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
            //         Hooks::AllCreatureEvent::ON_MELEE_DAMAGE,
            //         damage,
            //         allCreatureEventId,
            //         creature,
            //         target,
            //         damage
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_MELEE_DAMAGE);
            //     damage = Core::EventManager::GetInstance().TriggerEntryEventWithReturn<uint32>(
            //         Hooks::CreatureEvent::ON_MELEE_DAMAGE,
            //         creature,
            //         damage,
            //         creatureEventId,
            //         creature,
            //         target,
            //         damage
            //     );
            // }
        }

        void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
        {
            if(!target || !attacker || !spellInfo)
                return;

            if(attacker->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_SPELL_DAMAGE_TAKEN);
                damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<int32>(
                    Hooks::PlayerEvent::ON_SPELL_DAMAGE_TAKEN,
                    damage,
                    eventId,
                    attacker->ToPlayer(),
                    target,
                    damage,
                    spellInfo
                );
            }

            // if (attacker->IsCreature())
            // {
            //     Creature* creature = attacker->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_SPELL_DAMAGE_TAKEN);
            //     damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<int32>(
            //         Hooks::AllCreatureEvent::ON_SPELL_DAMAGE_TAKEN,
            //         damage,
            //         allCreatureEventId,
            //         creature,
            //         target,
            //         damage,
            //         spellInfo
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_SPELL_DAMAGE_TAKEN);
            //     damage = Core::EventManager::GetInstance().TriggerEntryEventWithReturn<int32>(
            //         Hooks::CreatureEvent::ON_SPELL_DAMAGE_TAKEN,
            //         creature,
            //         damage,
            //         creatureEventId,
            //         creature,
            //         target,
            //         damage,
            //         spellInfo
            //     );
            // }
        }

        void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo) override
        {
            if(!target || !healer || !spellInfo)
                return;

            if(healer->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_HEAL_RECEIVED);
                heal = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                    Hooks::PlayerEvent::ON_HEAL_RECEIVED,
                    heal,
                    eventId,
                    healer->ToPlayer(),
                    target,
                    heal,
                    spellInfo
                );
            }

            // if (healer->IsCreature())
            // {
            //     Creature* creature = healer->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_HEAL_RECEIVED);
            //     heal = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
            //         Hooks::AllCreatureEvent::ON_HEAL_RECEIVED,
            //         heal,
            //         allCreatureEventId,
            //         creature,
            //         target,
            //         heal,
            //         spellInfo
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_HEAL_RECEIVED);
            //     heal = Core::EventManager::GetInstance().TriggerEntryEventWithReturn<uint32>(
            //         Hooks::CreatureEvent::ON_HEAL_RECEIVED,
            //         creature,
            //         heal,
            //         creatureEventId,
            //         creature,
            //         target,
            //         heal,
            //         spellInfo
            //     );
            // }
        }

        uint32 DealDamage(Unit* AttackerUnit, Unit* pVictim, uint32 damage, DamageEffectType damagetype) override
        {
            if(!AttackerUnit || !pVictim)
                return damage;

            if(AttackerUnit->IsPlayer())
            {
                uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_DEAL_DAMAGE);
                return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                    Hooks::PlayerEvent::ON_DEAL_DAMAGE,
                    damage,
                    eventId,
                    AttackerUnit->ToPlayer(),
                    pVictim,
                    damage,
                    damagetype
                );
            }

            // if(AttackerUnit->IsCreature())
            // {
            //     Creature* creature = AttackerUnit->ToCreature();

            //     uint32 allCreatureEventId = static_cast<uint32>(Hooks::AllCreatureEvent::ON_DEAL_DAMAGE);
            //     damage = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
            //         Hooks::AllCreatureEvent::ON_DEAL_DAMAGE,
            //         damage,
            //         allCreatureEventId,
            //         creature,
            //         pVictim,
            //         damage,
            //         damagetype
            //     );

            //     uint32 creatureEventId = static_cast<uint32>(Hooks::CreatureEvent::ON_DEAL_DAMAGE);
            //     return Core::EventManager::GetInstance().TriggerEntryEventWithReturn<uint32>(
            //         Hooks::CreatureEvent::ON_DEAL_DAMAGE,
            //         creature,
            //         damage,
            //         creatureEventId,
            //         creature,
            //         pVictim,
            //         damage,
            //         damagetype
            //     );
            // }

            return damage;
        }

        void OnBeforeRollMeleeOutcomeAgainst(Unit const* attacker, Unit const* victim, WeaponAttackType attType, int32& attackerMaxSkillValueForLevel, int32& victimMaxSkillValueForLevel,int32& attackerWeaponSkill, int32& victimDefenseSkill, int32& crit_chance, int32& miss_chance, int32& dodge_chance, int32& parry_chance, int32& block_chance) override
        {
            if (!attacker || !victim)
                return;

            if (attacker->IsPlayer())
            {
                TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_ROLL_MELEE_OUTCOME,
                    const_cast<Player*>(attacker->ToPlayer()),
                    const_cast<Unit*>(victim),
                    attType,
                    attackerMaxSkillValueForLevel,
                    victimMaxSkillValueForLevel,
                    attackerWeaponSkill,
                    victimDefenseSkill,
                    crit_chance,
                    miss_chance,
                    dodge_chance,
                    parry_chance,
                    block_chance
                );
            }

            // if (attacker->IsCreature())
            // {
            //     Creature* creature = const_cast<Creature*>(attacker->ToCreature());
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_BEFORE_ROLL_MELEE_OUTCOME,
            //         creature,
            //         const_cast<Unit*>(victim),
            //         attType,
            //         attackerMaxSkillValueForLevel,
            //         victimMaxSkillValueForLevel,
            //         attackerWeaponSkill,
            //         victimDefenseSkill,
            //         crit_chance,
            //         miss_chance,
            //         dodge_chance,
            //         parry_chance,
            //         block_chance
            //     );

            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_BEFORE_ROLL_MELEE_OUTCOME,
            //         creature,
            //         const_cast<Unit*>(victim),
            //         attType,
            //         attackerMaxSkillValueForLevel,
            //         victimMaxSkillValueForLevel,
            //         attackerWeaponSkill,
            //         victimDefenseSkill,
            //         crit_chance,
            //         miss_chance,
            //         dodge_chance,
            //         parry_chance,
            //         block_chance
            //     );
            // }
        }

        void OnDisplayIdChange(Unit* unit, uint32 displayId) override
        {
            if (!unit)
                return;

            if (unit->IsPlayer())
                TriggerPlayerEvent(Hooks::PlayerEvent::ON_DISPLAY_ID_CHANGE, unit->ToPlayer(), displayId);

            // if (unit->IsCreature())
            // {
            //     Creature* creature = unit->ToCreature();
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_DISPLAY_ID_CHANGE, creature, displayId);
            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_DISPLAY_ID_CHANGE, creature, displayId);
            // }
        }

        void OnUnitEnterEvadeMode(Unit* unit, uint8 evadeReason) override
        {
            if (!unit)
                return;

            // if (unit->IsCreature())
            // {
            //     Creature* creature = unit->ToCreature();
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_ENTER_EVADE_MODE, creature, evadeReason);
            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_ENTER_EVADE_MODE, creature, evadeReason);
            // }
        }

        void OnUnitDeath(Unit* unit, Unit* killer) override
        {
            if (!unit)
                return;

            if (unit->IsPlayer())
                TriggerPlayerEvent(Hooks::PlayerEvent::ON_DEATH, unit->ToPlayer(), killer);

            // if (unit->IsCreature())
            // {
            //     Creature* creature = unit->ToCreature();
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_DEATH, creature, killer);
            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_DEATH, creature, killer);
            // }
        }

        void OnUnitSetShapeshiftForm(Unit* unit, uint8 form) override
        {
            if (!unit)
                return;

            if (unit->IsPlayer())
                TriggerPlayerEvent(Hooks::PlayerEvent::ON_SET_SHAPESHIFT_FORM, unit->ToPlayer(), form);

            // if (unit->IsCreature())
            // {
            //     Creature* creature = unit->ToCreature();
            //     TriggerAllCreatureEvent(Hooks::AllCreatureEvent::ON_SET_SHAPESHIFT_FORM, creature, form);
            //     TriggerCreatureEvent(Hooks::CreatureEvent::ON_SET_SHAPESHIFT_FORM, creature, form);
            // }
        }
    };

} // namespace ALE::Hooks

#endif // _ALE_UNIT_HOOKS_H
