/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_UNIT_METHODS_HPP
#define _ALE_UNIT_METHODS_HPP

#include <sol/sol.hpp>
#include "Unit.h"
#include "Player.h"
#include "Creature.h"
#include "Log.h"
#include "SpellMgr.h"

namespace ALE::Methods
{
    namespace UnitMethods
    {
        // ===========================
        // Health & Power
        // ===========================

        uint32 GetHealth(Unit* unit)
        {
            return unit->GetHealth();
        }

        uint32 GetMaxHealth(Unit* unit)
        {
            return unit->GetMaxHealth();
        }

        void SetHealth(Unit* unit, uint32 health)
        {
            unit->SetHealth(health);
        }

        uint32 GetPower(Unit* unit, uint8 powerType)
        {
            return unit->GetPower(static_cast<Powers>(powerType));
        }

        uint32 GetMaxPower(Unit* unit, uint8 powerType)
        {
            return unit->GetMaxPower(static_cast<Powers>(powerType));
        }

        void SetPower(Unit* unit, uint8 powerType, uint32 power)
        {
            unit->SetPower(static_cast<Powers>(powerType), power);
        }

        // ===========================
        // Position & Location
        // ===========================

        float GetX(Unit* unit)
        {
            return unit->GetPositionX();
        }

        float GetY(Unit* unit)
        {
            return unit->GetPositionY();
        }

        float GetZ(Unit* unit)
        {
            return unit->GetPositionZ();
        }

        float GetO(Unit* unit)
        {
            return unit->GetOrientation();
        }

        uint32 GetMapId(Unit* unit)
        {
            return unit->GetMapId();
        }

        uint32 GetZoneId(Unit* unit)
        {
            return unit->GetZoneId();
        }

        uint32 GetAreaId(Unit* unit)
        {
            return unit->GetAreaId();
        }

        // ===========================
        // Combat & State
        // ===========================

        bool IsInCombat(Unit* unit)
        {
            return unit->IsInCombat();
        }

        bool IsAlive(Unit* unit)
        {
            return unit->IsAlive();
        }

        bool IsDead(Unit* unit)
        {
            return !unit->IsAlive();
        }

        bool IsStandState(Unit* unit)
        {
            return unit->IsStandState();
        }

        bool IsMounted(Unit* unit)
        {
            return unit->IsMounted();
        }

        bool IsRooted(Unit* unit)
        {
            return unit->HasUnitState(UNIT_STATE_ROOT);
        }

        bool IsStunned(Unit* unit)
        {
            return unit->HasUnitState(UNIT_STATE_STUNNED);
        }

        // ===========================
        // Movement & Speed
        // ===========================

        float GetSpeed(Unit* unit, uint8 moveType)
        {
            return unit->GetSpeed(static_cast<UnitMoveType>(moveType));
        }

        void SetSpeed(Unit* unit, uint8 moveType, float speed, bool forced)
        {
            unit->SetSpeed(static_cast<UnitMoveType>(moveType), speed, forced);
        }

        // ===========================
        // Stats & Attributes
        // ===========================

        uint8 GetLevel(Unit* unit)
        {
            return unit->GetLevel();
        }

        void SetLevel(Unit* unit, uint8 level)
        {
            unit->SetLevel(level);
        }

        uint32 GetDisplayId(Unit* unit)
        {
            return unit->GetDisplayId();
        }

        void SetDisplayId(Unit* unit, uint32 displayId)
        {
            unit->SetDisplayId(displayId);
        }

        uint32 GetNativeDisplayId(Unit* unit)
        {
            return unit->GetNativeDisplayId();
        }

        // ===========================
        // Auras & Spells
        // ===========================

        bool HasAura(Unit* unit, uint32 spellId)
        {
            return unit->HasAura(spellId);
        }

        void AddAura(Unit* unit, uint32 spellId, uint32 duration)
        {
            unit->AddAura(spellId, unit);
        }

        void RemoveAura(Unit* unit, uint32 spellId)
        {
            unit->RemoveAura(spellId);
        }

        void RemoveAllAuras(Unit* unit)
        {
            unit->RemoveAllAuras();
        }

        // ===========================
        // Combat Actions
        // ===========================

        void CastSpell(Unit* unit, Unit* target, uint32 spellId, bool triggered)
        {
            unit->CastSpell(target, spellId, triggered);
        }

        void Kill(Unit* unit, Unit* victim, sol::optional<bool> durabilityLoss, sol::optional<uint8> attackType, sol::optional<uint32> spellId)
        {
            SpellInfo const* spellInfo = nullptr;
            if (spellId.has_value())
            {
                spellInfo = sSpellMgr->GetSpellInfo(spellId.value());
            }

            Unit::Kill(unit, victim, durabilityLoss.value_or(true), static_cast<WeaponAttackType>(attackType.value_or(BASE_ATTACK)), spellInfo, nullptr);
        }

        void DealDamage(Unit* unit, Unit* victim, uint32 damage, sol::optional<uint8> damageType, sol::optional<uint8> schoolMask, sol::optional<uint32> spellId, sol::optional<bool> durabilityLoss)
        {
            SpellInfo const* spellInfo = nullptr;
            if (spellId.has_value())
                spellInfo = sSpellMgr->GetSpellInfo(spellId.value());

            unit->DealDamage(unit, victim, damage, nullptr, static_cast<DamageEffectType>(damageType.value_or(DIRECT_DAMAGE)), static_cast<SpellSchoolMask>(schoolMask.value_or(SPELL_SCHOOL_MASK_NORMAL)), spellInfo, durabilityLoss.value_or(true), false, nullptr);
        }

        // ===========================
        // Target & Selection
        // ===========================

        Unit* GetVictim(Unit* unit)
        {
            return unit->GetVictim();
        }

        void SetVictim(Unit* unit, Unit* victim)
        {
            unit->SetInCombatWith(victim);
        }

        void ClearVictim(Unit* unit)
        {
            unit->ClearInCombat();
        }

    } // namespace UnitMethods

    /**
     * @brief Register Unit methods to Lua state
     * 
     * Creates the Unit usertype with all common methods.
     * Player and Creature will inherit from this via sol::bases<Unit>().
     * 
     * MUST be called BEFORE RegisterPlayerMethods and RegisterCreatureMethods!
     * 
     * @param state Lua state to register Unit type to
     */
    inline void RegisterUnitMethods(sol::state& state)
    {
        auto unitType = state.new_usertype<Unit>("Unit",
            sol::no_constructor
        );

        // Health & Power
        unitType["GetHealth"] = &UnitMethods::GetHealth;
        unitType["GetMaxHealth"] = &UnitMethods::GetMaxHealth;
        unitType["SetHealth"] = &UnitMethods::SetHealth;
        unitType["GetPower"] = &UnitMethods::GetPower;
        unitType["GetMaxPower"] = &UnitMethods::GetMaxPower;
        unitType["SetPower"] = &UnitMethods::SetPower;

        // Position & Location
        unitType["GetX"] = &UnitMethods::GetX;
        unitType["GetY"] = &UnitMethods::GetY;
        unitType["GetZ"] = &UnitMethods::GetZ;
        unitType["GetO"] = &UnitMethods::GetO;
        unitType["GetMapId"] = &UnitMethods::GetMapId;
        unitType["GetZoneId"] = &UnitMethods::GetZoneId;
        unitType["GetAreaId"] = &UnitMethods::GetAreaId;

        // Combat & State
        unitType["IsInCombat"] = &UnitMethods::IsInCombat;
        unitType["IsAlive"] = &UnitMethods::IsAlive;
        unitType["IsDead"] = &UnitMethods::IsDead;
        unitType["IsStandState"] = &UnitMethods::IsStandState;
        unitType["IsMounted"] = &UnitMethods::IsMounted;
        unitType["IsRooted"] = &UnitMethods::IsRooted;
        unitType["IsStunned"] = &UnitMethods::IsStunned;

        // Movement & Speed
        unitType["GetSpeed"] = &UnitMethods::GetSpeed;
        unitType["SetSpeed"] = &UnitMethods::SetSpeed;

        // Stats & Attributes
        unitType["GetLevel"] = &UnitMethods::GetLevel;
        unitType["SetLevel"] = &UnitMethods::SetLevel;
        unitType["GetDisplayId"] = &UnitMethods::GetDisplayId;
        unitType["SetDisplayId"] = &UnitMethods::SetDisplayId;
        unitType["GetNativeDisplayId"] = &UnitMethods::GetNativeDisplayId;

        // Auras & Spells
        unitType["HasAura"] = &UnitMethods::HasAura;
        unitType["AddAura"] = &UnitMethods::AddAura;
        unitType["RemoveAura"] = &UnitMethods::RemoveAura;
        unitType["RemoveAllAuras"] = &UnitMethods::RemoveAllAuras;

        // Combat Actions
        unitType["CastSpell"] = &UnitMethods::CastSpell;
        unitType["Kill"] = &UnitMethods::Kill;
        unitType["DealDamage"] = &UnitMethods::DealDamage;

        // Target & Selection
        unitType["GetVictim"] = &UnitMethods::GetVictim;
        unitType["SetVictim"] = &UnitMethods::SetVictim;
        unitType["ClearVictim"] = &UnitMethods::ClearVictim;
    }

} // namespace ALE::Methods

#endif // _ALE_UNIT_METHODS_HPP
