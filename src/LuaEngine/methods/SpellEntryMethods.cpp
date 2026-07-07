/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "DBCStores.h"
#include "DBCStructure.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

/***
 * Represents spell data loaded from the DBCs, including effects, costs, attributes, and requirements.
 *
 * Used for inspecting the properties of any spell in the game, such as mana cost, targets, or effects.
 *
 * Inherits all methods from: none
 */
namespace LuaSpellEntry
{
    /**
     * Returns the ID of the [SpellEntry].
     *
     * @return uint32 id
     */
    uint32 GetId(SpellEntry* entry)
    {
        return entry->Id;
    }

    /**
     * Returns the category ID for the [SpellEntry].
     *
     * @return uint32 categoryId
     */
    uint32 GetCategory(SpellEntry* entry)
    {
        return entry->Category;
    }

    /**
     * Returns the dispel ID for the [SpellEntry].
     *
     * @return uint32 dispelId
     */
    uint32 GetDispel(SpellEntry* entry)
    {
        return entry->Dispel;
    }

    /**
     * Returns the mechanic ID for the [SpellEntry].
     *
     * @return uint32 mechanicId
     */
    uint32 GetMechanic(SpellEntry* entry)
    {
        return entry->Mechanic;
    }

    /**
     * Returns the attribute bitflags for the [SpellEntry].
     *
     * @return uint32 attribute : bitmask, but returned as uint32
     */
    uint32 GetAttributes(SpellEntry* entry)
    {
        return entry->Attributes;
    }

    /**
     * Returns the attributeEx bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx(SpellEntry* entry)
    {
        return entry->AttributesEx;
    }

    /**
     * Returns the attributeEx2 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx2 : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx2(SpellEntry* entry)
    {
        return entry->AttributesEx2;
    }

    /**
     * Returns the attributeEx3 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx3 : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx3(SpellEntry* entry)
    {
        return entry->AttributesEx3;
    }

    /**
     * Returns the attributeEx4 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx4 : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx4(SpellEntry* entry)
    {
        return entry->AttributesEx4;
    }

    /**
     * Returns the attributeEx5 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx5 : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx5(SpellEntry* entry)
    {
        return entry->AttributesEx5;
    }

    /**
     * Returns the attributeEx6 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx6 : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx6(SpellEntry* entry)
    {
        return entry->AttributesEx6;
    }

    /**
     * Returns the attributeEx7 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx7 : bitmask, but returned as uint32
     */
    uint32 GetAttributesEx7(SpellEntry* entry)
    {
        return entry->AttributesEx7;
    }

    /**
     * Returns the stance bitflags for the [SpellEntry].
     *
     * @return uint32 stance : bitmask, but returned as uint32
     */
    uint32 GetStances(SpellEntry* entry)
    {
        return entry->Stances;
    }

    /**
     * Returns the stance restriction bitmask for which the [SpellEntry] cannot be used.
     *
     * This mask indicates which shapeshift forms (stances) prevent the spell from being cast.
     *
     * @return uint32 stancesNotMask
     */
    uint32 GetStancesNot(SpellEntry* entry)
    {
        return entry->StancesNot;
    }

    /**
     * Returns the target bitmasks for the [SpellEntry].
     *
     * @return uint32 target : bitmasks, but returned as uint32.
     */
    uint32 GetTargets(SpellEntry* entry)
    {
        return entry->Targets;
    }

    /**
     * Returns the target creature type bitmasks for the [SpellEntry].
     *
     * @return uint32 targetCreatureType : bitmasks, but returned as uint32.
     */
    uint32 GetTargetCreatureType(SpellEntry* entry)
    {
        return entry->TargetCreatureType;
    }

    /**
     * Returns the SpellFocus ID required to cast this [SpellEntry].
     *
     * Some spells require proximity to a specific game object (e.g., a brazier or altar).
     *
     * @return uint32 spellFocusId
     */
    uint32 GetRequiresSpellFocus(SpellEntry* entry)
    {
        return entry->RequiresSpellFocus;
    }

    /**
     * Returns the facing flags for this [SpellEntry].
     *
     * Indicates whether the caster must be facing the target or meet other orientation constraints.
     *
     * @return uint32 facingFlags
     */
    uint32 GetFacingCasterFlags(SpellEntry* entry)
    {
        return entry->FacingCasterFlags;
    }

    /**
     * Returns the required caster aura state for this [SpellEntry].
     *
     * The spell can only be cast if the caster has a specific aura state active.
     *
     * @return uint32 casterAuraState
     */
    uint32 GetCasterAuraState(SpellEntry* entry)
    {
        return entry->CasterAuraState;
    }

    /**
     * Returns the required target aura state for this [SpellEntry].
     *
     * The spell can only be cast if the target has a specific aura state active.
     *
     * @return uint32 targetAuraState
     */
    uint32 GetTargetAuraState(SpellEntry* entry)
    {
        return entry->TargetAuraState;
    }

    /**
     * Returns the forbidden caster aura state for this [SpellEntry].
     *
     * The spell cannot be cast if the caster has this aura state active.
     *
     * @return uint32 casterAuraStateNot
     */
    uint32 GetCasterAuraStateNot(SpellEntry* entry)
    {
        return entry->CasterAuraStateNot;
    }

    /**
     * Returns the forbidden target aura state for this [SpellEntry].
     *
     * The spell cannot be cast if the target has this aura state active.
     *
     * @return uint32 targetAuraStateNot
     */
    uint32 GetTargetAuraStateNot(SpellEntry* entry)
    {
        return entry->TargetAuraStateNot;
    }

    /**
     * Returns the required aura spell ID that must be on the caster.
     *
     * The spell can only be cast if the caster has an aura from this spell.
     *
     * @return uint32 casterAuraSpellId
     */
    uint32 GetCasterAuraSpell(SpellEntry* entry)
    {
        return entry->CasterAuraSpell;
    }

    /**
     * Returns the required aura spell ID that must be on the target.
     *
     * The spell can only be cast if the target has an aura from this spell.
     *
     * @return uint32 targetAuraSpellId
     */
    uint32 GetTargetAuraSpell(SpellEntry* entry)
    {
        return entry->TargetAuraSpell;
    }

    /**
     * Returns the aura spell ID that must NOT be on the caster.
     *
     * The spell cannot be cast if the caster has an aura from this spell.
     *
     * @return uint32 excludeCasterAuraSpellId
     */
    uint32 GetExcludeCasterAuraSpell(SpellEntry* entry)
    {
        return entry->ExcludeCasterAuraSpell;
    }

    /**
     * Returns the aura spell ID that must NOT be on the target.
     *
     * The spell cannot be cast if the target has an aura from this spell.
     *
     * @return uint32 excludeTargetAuraSpellId
     */
    uint32 GetExcludeTargetAuraSpell(SpellEntry* entry)
    {
        return entry->ExcludeTargetAuraSpell;
    }

    /**
     * Returns the casting time index of this [SpellEntry].
     *
     * This index is used to look up the base casting time in SpellCastTimes.dbc.
     *
     * @return uint32 castingTimeIndex
     */
    uint32 GetCastingTimeIndex(SpellEntry* entry)
    {
        return entry->CastingTimeIndex;
    }

    /**
     * Returns the recovery time for the [SpellEntry].
     *
     * @return uint32 recoveryTime
     */
    uint32 GetRecoveryTime(SpellEntry* entry)
    {
        return entry->RecoveryTime;
    }

    /**
     * Returns the category recovery time for the [SpellEntry].
     *
     * @return uint32 categoryRecoveryTime : in milliseconds, returned as uint32
     */
    uint32 GetCategoryRecoveryTime(SpellEntry* entry)
    {
        return entry->CategoryRecoveryTime;
    }

    /**
     * Returns the interrupt flags for this [SpellEntry].
     *
     * Determines what can interrupt this spell while casting (e.g., movement, taking damage).
     *
     * @return uint32 interruptFlags
     */
    uint32 GetInterruptFlags(SpellEntry* entry)
    {
        return entry->InterruptFlags;
    }

    /**
     * Returns the aura interrupt flags for this [SpellEntry].
     *
     * Indicates what actions will break or remove the aura applied by this spell.
     *
     * @return uint32 auraInterruptFlags
     */
    uint32 GetAuraInterruptFlags(SpellEntry* entry)
    {
        return entry->AuraInterruptFlags;
    }

    /**
     * Returns the channel interrupt flags for this [SpellEntry].
     *
     * Specifies conditions under which a channeled spell will be interrupted (e.g., moving or turning).
     *
     * @return uint32 channelInterruptFlags
     */
    uint32 GetChannelInterruptFlags(SpellEntry* entry)
    {
        return entry->ChannelInterruptFlags;
    }

    /**
     * Returns the proc flags for this [SpellEntry].
     *
     * Determines the types of actions or triggers that can cause this spell to proc.
     *
     * @return uint32 procFlags
     */
    uint32 GetProcFlags(SpellEntry* entry)
    {
        return entry->ProcFlags;
    }

    /**
     * Returns the proc chance of [SpellEntry].
     *
     * @return uint32 procChance
     */
    uint32 GetProcChance(SpellEntry* entry)
    {
        return entry->ProcChance;
    }

    /**
     * Returns the proc charges of [SpellEntry].
     *
     * @return uint32 procCharges
     */
    uint32 GetProcCharges(SpellEntry* entry)
    {
        return entry->ProcCharges;
    }

    /**
     * Returns the max level for the [SpellEntry].
     *
     * @return uint32 maxLevel : the [SpellEntry] max level.
     */
    uint32 GetMaxLevel(SpellEntry* entry)
    {
        return entry->MaxLevel;
    }

    /**
     * Returns the base level required for the [SpellEntry].
     *
     * @return uint32 baseLevel
     */
    uint32 GetBaseLevel(SpellEntry* entry)
    {
        return entry->BaseLevel;
    }

    /**
     * Returns the spell level for the [SpellEntry].
     *
     * @return uint32 spellLevel
     */
    uint32 GetSpellLevel(SpellEntry* entry)
    {
        return entry->SpellLevel;
    }

    /**
     * Returns the duration index for the [SpellEntry].
     *
     * @return uint32 durationIndex
     */
    uint32 GetDurationIndex(SpellEntry* entry)
    {
        return entry->DurationIndex;
    }

    /**
     * Returns the power type ID for the [SpellEntry].
     *
     * @return uint32 powerTypeId
     */
    uint32 GetPowerType(SpellEntry* entry)
    {
        return entry->PowerType;
    }

    /**
     * Returns the mana cost for the [SpellEntry].
     *
     * @return uint32 manaCost
     */
    uint32 GetManaCost(SpellEntry* entry)
    {
        return entry->ManaCost;
    }

    /**
     * Returns the mana cost per level for [SpellEntry].
     *
     * @return uint32 manaCostPerLevel
     */
    uint32 GetManaCostPerlevel(SpellEntry* entry)
    {
        return entry->ManaCostPerlevel;
    }

    /**
     * Returns the mana per second for [SpellEntry].
     *
     * @return uint32 manaPerSecond
     */
    uint32 GetManaPerSecond(SpellEntry* entry)
    {
        return entry->ManaPerSecond;
    }

    /**
     * Returns the mana per second per level for [SpellEntry].
     *
     * @return uint32 manaPerSecondPerLevel
     */
    uint32 GetManaPerSecondPerLevel(SpellEntry* entry)
    {
        return entry->ManaPerSecondPerLevel;
    }

    /**
     * Returns the range index for [SpellEntry].
     *
     * @return uint32 rangeIndex
     */
    uint32 GetRangeIndex(SpellEntry* entry)
    {
        return entry->RangeIndex;
    }

    /**
     * Returns speed for [SpellEntry].
     *
     * @return uint32 speed
     */
    float GetSpeed(SpellEntry* entry)
    {
        return entry->Speed;
    }

    /**
     * Returns the stack amount for [SpellEntry].
     *
     * @return uint32 stackAmount
     */
    uint32 GetStackAmount(SpellEntry* entry)
    {
        return entry->StackAmount;
    }

    /**
     * Returns a table with all totem values for [SpellEntry].
     *
     * @return table totem
     */
    sol::table GetTotem(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->Totem.size(); ++index)
        {
            tbl[++i] = entry->Totem[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all reagent values for [SpellEntry].
     *
     * @return table reagent
     */
    sol::table GetReagent(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->Reagent.size(); ++index)
        {
            tbl[++i] = entry->Reagent[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all reagent count values for [SpellEntry].
     *
     * @return table reagentCount
     */
    sol::table GetReagentCount(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->ReagentCount.size(); ++index)
        {
            tbl[++i] = entry->ReagentCount[index];
        }

        return tbl;
    }

    /**
     * Returns the equipped item class ID for [SpellEntry].
     *
     * @return uint32 equippedItemClassId
     */
    int32 GetEquippedItemClass(SpellEntry* entry)
    {
        return entry->EquippedItemClass;
    }

    /**
     * Returns the equipped item sub class masks for [SpellEntry].
     *
     * @return uint32 equippedItemSubClassMasks : bitmasks, returned as uint32.
     */
    int32 GetEquippedItemSubClassMask(SpellEntry* entry)
    {
        return entry->EquippedItemSubClassMask;
    }

    /**
     * Returns the equipped item inventory type masks for [SpellEntry].
     *
     * @return uint32 equippedItemInventoryTypeMasks : bitmasks, returned as uint32.
     */
    int32 GetEquippedItemInventoryTypeMask(SpellEntry* entry)
    {
        return entry->EquippedItemInventoryTypeMask;
    }

    /**
     * Returns a table with all spell effect IDs for [SpellEntry].
     *
     * @return table effect
     */
    sol::table GetEffect(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->Effect.size(); ++index)
        {
            tbl[++i] = entry->Effect[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect die sides values for [SpellEntry].
     *
     * @return table effectDieSides
     */
    sol::table GetEffectDieSides(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectDieSides.size(); ++index)
        {
            tbl[++i] = entry->EffectDieSides[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect real points per level values for [SpellEntry].
     *
     * @return table effectRealPointsPerLevel
     */
    sol::table GetEffectRealPointsPerLevel(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectRealPointsPerLevel.size(); ++index)
        {
            tbl[++i] = entry->EffectRealPointsPerLevel[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect base points values for [SpellEntry].
     *
     * @return table effectBasePoints
     */
    sol::table GetEffectBasePoints(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectBasePoints.size(); ++index)
        {
            tbl[++i] = entry->EffectBasePoints[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect mechanic IDs for [SpellEntry].
     *
     * @return table effectMechanic
     */
    sol::table GetEffectMechanic(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectMechanic.size(); ++index)
        {
            tbl[++i] = entry->EffectMechanic[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect implicit target a IDs for [SpellEntry].
     *
     * @return table effectImplicitTargetA
     */
    sol::table GetEffectImplicitTargetA(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectImplicitTargetA.size(); ++index)
        {
            tbl[++i] = entry->EffectImplicitTargetA[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect implicit target b IDs for [SpellEntry].
     *
     * @return table effectImplicitTargetB
     */
    sol::table GetEffectImplicitTargetB(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectImplicitTargetB.size(); ++index)
        {
            tbl[++i] = entry->EffectImplicitTargetB[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect radius index for [SpellEntry].
     *
     * @return table effectRadiusIndex
     */
    sol::table GetEffectRadiusIndex(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectRadiusIndex.size(); ++index)
        {
            tbl[++i] = entry->EffectRadiusIndex[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect apply aura IDs for [SpellEntry].
     *
     * @return table effectApplyAura
     */
    sol::table GetEffectApplyAuraName(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectApplyAuraName.size(); ++index)
        {
            tbl[++i] = entry->EffectApplyAuraName[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect amplitude values for [SpellEntry].
     *
     * @return table effectAmplitude
     */
    sol::table GetEffectAmplitude(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectAmplitude.size(); ++index)
        {
            tbl[++i] = entry->EffectAmplitude[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect value multiplier for [SpellEntry].
     *
     * @return table effectValueMultiplier
     */
    sol::table GetEffectValueMultiplier(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectValueMultiplier.size(); ++index)
        {
            tbl[++i] = entry->EffectValueMultiplier[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect chain target values for [SpellEntry].
     *
     * @return table effectChainTarget
     */
    sol::table GetEffectChainTarget(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectChainTarget.size(); ++index)
        {
            tbl[++i] = entry->EffectChainTarget[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect item type values for [SpellEntry].
     *
     * @return table effectItemType
     */
    sol::table GetEffectItemType(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectItemType.size(); ++index)
        {
            tbl[++i] = entry->EffectItemType[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect misc value A for [SpellEntry].
     *
     * @return table effectMiscValueA
     */
    sol::table GetEffectMiscValue(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectMiscValue.size(); ++index)
        {
            tbl[++i] = entry->EffectMiscValue[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect misc value B for [SpellEntry].
     *
     * @return table effectMiscValueB
     */
    sol::table GetEffectMiscValueB(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectMiscValueB.size(); ++index)
        {
            tbl[++i] = entry->EffectMiscValueB[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect trigger spell for [SpellEntry].
     *
     * @return table effectTriggerSpell
     */
    sol::table GetEffectTriggerSpell(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectTriggerSpell.size(); ++index)
        {
            tbl[++i] = entry->EffectTriggerSpell[index];
        }

        return tbl;
    }

    /**
     * Returns a table with all effect points per combo point of [SpellEntry]
     *
     * @return table effectPointsPerComboPoint : returns a table containing all the effect points per combo point values of [SpellEntry]
     */
    sol::table GetEffectPointsPerComboPoint(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectPointsPerComboPoint.size(); ++index)
        {
            tbl[++i] = entry->EffectPointsPerComboPoint[index];
        }

        return tbl;
    }

    /**
     * Returns a table of [SpellFamilyFlags] for each effect of this [SpellEntry].
     *
     * These flags are used to categorize spell effects for use with spell group logic.
     * The table contains up to 3 bitmask entries, one per effect.
     *
     * @return table effectSpellClassMask : table of [SpellFamilyFlags] per effect
     */
    sol::table GetEffectSpellClassMask(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectSpellClassMask.size(); ++index)
        {
            tbl[++i] = static_cast<bool>(entry->EffectSpellClassMask[index]);
        }

        return tbl;
    }

    /**
     * Returns a table with both spell visuals of [SpellEntry]
     *
     * @return table spellVisuals : returns a table containing both spellVisuals for [SpellEntry].
     */
    sol::table GetSpellVisual(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->SpellVisual.size(); ++index)
        {
            tbl[++i] = entry->SpellVisual[index];
        }

        return tbl;
    }

    /**
     * Returns the spell icon ID for the [SpellEntry].
     *
     * @return uint32 spellIconId
     */
    uint32 GetSpellIconID(SpellEntry* entry)
    {
        return entry->SpellIconID;
    }

    /**
     * Returns the active icon ID for the [SpellEntry].
     *
     * @return uint32 activeIconId
     */
    uint32 GetActiveIconID(SpellEntry* entry)
    {
        return entry->ActiveIconID;
    }

    /**
     * Returns the spell Priority for the [SpellEntry].
     *
     * @return uint32 spellPriority
     */
    uint32 GetSpellPriority(SpellEntry* entry)
    {
        return entry->SpellPriority;
    }

    /**
     * Returns a table of the [SpellEntry] names of all locals.
     *
     * @return table spellNames
     */
    sol::table GetSpellName(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->SpellName.size(); ++index)
        {
            tbl[++i] = entry->SpellName[index];
        }

        return tbl;
    }

    /**
     * Returns a table of the [SpellEntry] ranks.
     *
     * @return table spellRanks
     */
    sol::table GetRank(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->Rank.size(); ++index)
        {
            tbl[++i] = entry->Rank[index];
        }

        return tbl;
    }

    /**
     * Returns the mana cost percentage of [SpellEntry].
     *
     * @return uint32 manaCostPercentage : the mana cost in percentage, returned as uint32.
     */
    uint32 GetManaCostPercentage(SpellEntry* entry)
    {
        return entry->ManaCostPercentage;
    }

    /**
     * Returns the global cooldown time value for [SpellEntry].
     *
     * @return uint32 globalCooldownTime
     */
    uint32 GetStartRecoveryCategory(SpellEntry* entry)
    {
        return entry->StartRecoveryCategory;
    }

    /**
     * Returns the global cooldown category value for [SpellEntry].
     *
     * @return uint32 globalCooldownCategory
     */
    uint32 GetStartRecoveryTime(SpellEntry* entry)
    {
        return entry->StartRecoveryTime;
    }

    /**
     * Returns the max target level value for [SpellEntry].
     *
     * @return uint32 maxTargetLevel
     */
    uint32 GetMaxTargetLevel(SpellEntry* entry)
    {
        return entry->MaxTargetLevel;
    }

    /**
     * Returns the spell family name of this [SpellEntry].
     *
     * This identifies the broader category or class of spells (e.g., Mage, Warrior, Rogue).
     *
     * @return uint32 spellFamilyName
     */
    uint32 GetSpellFamilyName(SpellEntry* entry)
    {
        return entry->SpellFamilyName;
    }

    /**
     * Returns the spell family flags of this [SpellEntry].
     *
     * These bitflags represent specific characteristics or subcategories of spells within a family.
     *
     * @return uint64 spellFamilyFlags
     */
    bool GetSpellFamilyFlags(SpellEntry* entry)
    {
        return entry->SpellFamilyFlags;
    }

    /**
     * Returns the max affected targets value [SpellEntry].
     *
     * @return uint32 maxAffectedTargets
     */
    uint32 GetMaxAffectedTargets(SpellEntry* entry)
    {
        return entry->MaxAffectedTargets;
    }

    /**
     * Returns the spell damage type ID [SpellEntry].
     *
     * @return uint32 spellDamageTypeId
     */
    uint32 GetDmgClass(SpellEntry* entry)
    {
        return entry->DmgClass;
    }

    /**
     * Returns the prevention type ID [SpellEntry].
     *
     * @return uint32 preventionTypeId
     */
    uint32 GetPreventionType(SpellEntry* entry)
    {
        return entry->PreventionType;
    }

    /**
     * Returns a table with all effect damage multiplier values [SpellEntry].
     *
     * @return table effectDamageMultipliers
     */
    sol::table GetEffectDamageMultiplier(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectDamageMultiplier.size(); ++index)
        {
            tbl[++i] = entry->EffectDamageMultiplier[index];
        }

        return tbl;
    }

    /**
     * Returns a table with totem categories IDs [SpellEntry].
     *
     * @return table totemCategory
     */
    sol::table GetTotemCategory(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->TotemCategory.size(); ++index)
        {
            tbl[++i] = entry->TotemCategory[index];
        }

        return tbl;
    }

    /**
     * Returns the Area Group ID associated with this [SpellEntry].
     *
     * AreaGroupId is used to restrict spell usage to specific zones or areas.
     *
     * @return uint32 areaGroupId
     */
    int32 GetAreaGroupId(SpellEntry* entry)
    {
        return entry->AreaGroupId;
    }

    /**
     * Returns the school mask of [SpellEntry].
     *
     * @return uint32 schoolMask : bitmask, returned as uint32.
     */
    uint32 GetSchoolMask(SpellEntry* entry)
    {
        return entry->SchoolMask;
    }

    /**
     * Returns the rune cost id for the [SpellEntry].
     *
     * @return uint32 runeCostId
     */
    uint32 GetRuneCostID(SpellEntry* entry)
    {
        return entry->RuneCostID;
    }

    /**
     * Returns a table with all effect bonus multiplier values [SpellEntry].
     *
     * @return table effectBonusMultipliers
     */
    sol::table GetEffectBonusMultiplier(SpellEntry* entry, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectBonusMultiplier.size(); ++index)
        {
            tbl[++i] = entry->EffectBonusMultiplier[index];
        }

        return tbl;
    }

    /**
     * Sets the category for the [SpellEntry].
     *
     * @param uint32 category : the new category value
     */
    void SetCategory(SpellEntry* entry, uint32 category)
    {
        entry->Category = category;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CategoryEntry = category ? sSpellCategoryStore.LookupEntry(category) : nullptr;
        }
    }

    /**
     * Sets the dispel type for the [SpellEntry].
     *
     * @param uint32 dispel : the new dispel type value
     */
    void SetDispel(SpellEntry* entry, uint32 dispel)
    {
        entry->Dispel = dispel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Dispel = dispel;
        }
    }

    /**
     * Sets the mechanic for the [SpellEntry].
     *
     * @param uint32 mechanic : the new mechanic value
     */
    void SetMechanic(SpellEntry* entry, uint32 mechanic)
    {
        entry->Mechanic = mechanic;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Mechanic = mechanic;
        }
    }

    /**
     * Sets the attributes for the [SpellEntry].
     *
     * @param uint32 attributes : the new attributes bitmask
     */
    void SetAttributes(SpellEntry* entry, uint32 attributes)
    {
        entry->Attributes = attributes;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Attributes = attributes;
        }
    }

    /**
     * Sets the attributesEx for the [SpellEntry].
     *
     * @param uint32 attributesEx : the new attributesEx bitmask
     */
    void SetAttributesEx(SpellEntry* entry, uint32 attributesEx)
    {
        entry->AttributesEx = attributesEx;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx = attributesEx;
        }
    }

    /**
     * Sets the attributesEx2 for the [SpellEntry].
     *
     * @param uint32 attributesEx2 : the new attributesEx2 bitmask
     */
    void SetAttributesEx2(SpellEntry* entry, uint32 attributesEx2)
    {
        entry->AttributesEx2 = attributesEx2;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx2 = attributesEx2;
        }
    }

    /**
     * Sets the attributesEx3 for the [SpellEntry].
     *
     * @param uint32 attributesEx3 : the new attributesEx3 bitmask
     */
    void SetAttributesEx3(SpellEntry* entry, uint32 attributesEx3)
    {
        entry->AttributesEx3 = attributesEx3;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx3 = attributesEx3;
        }
    }

    /**
     * Sets the attributesEx4 for the [SpellEntry].
     *
     * @param uint32 attributesEx4 : the new attributesEx4 bitmask
     */
    void SetAttributesEx4(SpellEntry* entry, uint32 attributesEx4)
    {
        entry->AttributesEx4 = attributesEx4;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx4 = attributesEx4;
        }
    }

    /**
     * Sets the attributesEx5 for the [SpellEntry].
     *
     * @param uint32 attributesEx5 : the new attributesEx5 bitmask
     */
    void SetAttributesEx5(SpellEntry* entry, uint32 attributesEx5)
    {
        entry->AttributesEx5 = attributesEx5;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx5 = attributesEx5;
        }
    }

    /**
     * Sets the attributesEx6 for the [SpellEntry].
     *
     * @param uint32 attributesEx6 : the new attributesEx6 bitmask
     */
    void SetAttributesEx6(SpellEntry* entry, uint32 attributesEx6)
    {
        entry->AttributesEx6 = attributesEx6;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx6 = attributesEx6;
        }
    }

    /**
     * Sets the attributesEx7 for the [SpellEntry].
     *
     * @param uint32 attributesEx7 : the new attributesEx7 bitmask
     */
    void SetAttributesEx7(SpellEntry* entry, uint32 attributesEx7)
    {
        entry->AttributesEx7 = attributesEx7;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx7 = attributesEx7;
        }
    }

    /**
     * Sets the stances for the [SpellEntry].
     *
     * @param uint32 stances : the new stances bitmask
     */
    void SetStances(SpellEntry* entry, uint32 stances)
    {
        entry->Stances = stances;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Stances = stances;
        }
    }

    /**
     * Sets the stancesNot for the [SpellEntry].
     *
     * @param uint32 stancesNot : the new stancesNot bitmask
     */
    void SetStancesNot(SpellEntry* entry, uint32 stancesNot)
    {
        entry->StancesNot = stancesNot;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StancesNot = stancesNot;
        }
    }

    /**
     * Sets the targets for the [SpellEntry].
     *
     * @param uint32 targets : the new targets bitmask
     */
    void SetTargets(SpellEntry* entry, uint32 targets)
    {
        entry->Targets = targets;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Targets = targets;
        }
    }

    /**
     * Sets the target creature type for the [SpellEntry].
     *
     * @param uint32 targetCreatureType : the new target creature type bitmask
     */
    void SetTargetCreatureType(SpellEntry* entry, uint32 targetCreatureType)
    {
        entry->TargetCreatureType = targetCreatureType;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetCreatureType = targetCreatureType;
        }
    }

    /**
     * Sets the requires spell focus for the [SpellEntry].
     *
     * @param uint32 requiresSpellFocus : the new requires spell focus value
     */
    void SetRequiresSpellFocus(SpellEntry* entry, uint32 requiresSpellFocus)
    {
        entry->RequiresSpellFocus = requiresSpellFocus;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->RequiresSpellFocus = requiresSpellFocus;
        }
    }

    /**
     * Sets the facing caster flags for the [SpellEntry].
     *
     * @param uint32 facingCasterFlags : the new facing caster flags value
     */
    void SetFacingCasterFlags(SpellEntry* entry, uint32 facingCasterFlags)
    {
        entry->FacingCasterFlags = facingCasterFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->FacingCasterFlags = facingCasterFlags;
        }
    }

    /**
     * Sets the caster aura state for the [SpellEntry].
     *
     * @param uint32 casterAuraState : the new caster aura state value
     */
    void SetCasterAuraState(SpellEntry* entry, uint32 casterAuraState)
    {
        entry->CasterAuraState = casterAuraState;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CasterAuraState = casterAuraState;
        }
    }

    /**
     * Sets the target aura state for the [SpellEntry].
     *
     * @param uint32 targetAuraState : the new target aura state value
     */
    void SetTargetAuraState(SpellEntry* entry, uint32 targetAuraState)
    {
        entry->TargetAuraState = targetAuraState;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetAuraState = targetAuraState;
        }
    }

    /**
     * Sets the caster aura state not for the [SpellEntry].
     *
     * @param uint32 casterAuraStateNot : the new caster aura state not value
     */
    void SetCasterAuraStateNot(SpellEntry* entry, uint32 casterAuraStateNot)
    {
        entry->CasterAuraStateNot = casterAuraStateNot;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CasterAuraStateNot = casterAuraStateNot;
        }
    }

    /**
     * Sets the target aura state not for the [SpellEntry].
     *
     * @param uint32 targetAuraStateNot : the new target aura state not value
     */
    void SetTargetAuraStateNot(SpellEntry* entry, uint32 targetAuraStateNot)
    {
        entry->TargetAuraStateNot = targetAuraStateNot;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetAuraStateNot = targetAuraStateNot;
        }
    }

    /**
     * Sets the caster aura spell for the [SpellEntry].
     *
     * @param uint32 casterAuraSpell : the new caster aura spell ID
     */
    void SetCasterAuraSpell(SpellEntry* entry, uint32 casterAuraSpell)
    {
        entry->CasterAuraSpell = casterAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CasterAuraSpell = casterAuraSpell;
        }
    }

    /**
     * Sets the target aura spell for the [SpellEntry].
     *
     * @param uint32 targetAuraSpell : the new target aura spell ID
     */
    void SetTargetAuraSpell(SpellEntry* entry, uint32 targetAuraSpell)
    {
        entry->TargetAuraSpell = targetAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetAuraSpell = targetAuraSpell;
        }
    }

    /**
     * Sets the exclude caster aura spell for the [SpellEntry].
     *
     * @param uint32 excludeCasterAuraSpell : the new exclude caster aura spell ID
     */
    void SetExcludeCasterAuraSpell(SpellEntry* entry, uint32 excludeCasterAuraSpell)
    {
        entry->ExcludeCasterAuraSpell = excludeCasterAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ExcludeCasterAuraSpell = excludeCasterAuraSpell;
        }
    }

    /**
     * Sets the exclude target aura spell for the [SpellEntry].
     *
     * @param uint32 excludeTargetAuraSpell : the new exclude target aura spell ID
     */
    void SetExcludeTargetAuraSpell(SpellEntry* entry, uint32 excludeTargetAuraSpell)
    {
        entry->ExcludeTargetAuraSpell = excludeTargetAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ExcludeTargetAuraSpell = excludeTargetAuraSpell;
        }
    }

    /**
     * Sets the recovery time for the [SpellEntry].
     *
     * @param uint32 recoveryTime : the new recovery time value
     */
    void SetRecoveryTime(SpellEntry* entry, uint32 recoveryTime)
    {
        entry->RecoveryTime = recoveryTime;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->RecoveryTime = recoveryTime;
        }
    }

    /**
     * Sets the category recovery time for the [SpellEntry].
     *
     * @param uint32 categoryRecoveryTime : the new category recovery time value in milliseconds
     */
    void SetCategoryRecoveryTime(SpellEntry* entry, uint32 categoryRecoveryTime)
    {
        entry->CategoryRecoveryTime = categoryRecoveryTime;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CategoryRecoveryTime = categoryRecoveryTime;
        }
    }

    /**
     * Sets the interrupt flags for the [SpellEntry].
     *
     * @param uint32 interruptFlags : the new interrupt flags bitmask
     */
    void SetInterruptFlags(SpellEntry* entry, uint32 interruptFlags)
    {
        entry->InterruptFlags = interruptFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->InterruptFlags = interruptFlags;
        }
    }

    /**
     * Sets the aura interrupt flags for the [SpellEntry].
     *
     * @param uint32 auraInterruptFlags : the new aura interrupt flags bitmask
     */
    void SetAuraInterruptFlags(SpellEntry* entry, uint32 auraInterruptFlags)
    {
        entry->AuraInterruptFlags = auraInterruptFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AuraInterruptFlags = auraInterruptFlags;
        }
    }

    /**
     * Sets the channel interrupt flags for the [SpellEntry].
     *
     * @param uint32 channelInterruptFlags : the new channel interrupt flags bitmask
     */
    void SetChannelInterruptFlags(SpellEntry* entry, uint32 channelInterruptFlags)
    {
        entry->ChannelInterruptFlags = channelInterruptFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ChannelInterruptFlags = channelInterruptFlags;
        }
    }

    /**
     * Sets the proc flags for the [SpellEntry].
     *
     * @param uint32 procFlags : the new proc flags bitmask
     */
    void SetProcFlags(SpellEntry* entry, uint32 procFlags)
    {
        entry->ProcFlags = procFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ProcFlags = procFlags;
        }
    }

    /**
     * Sets the proc chance for the [SpellEntry].
     *
     * @param uint32 procChance : the new proc chance value
     */
    void SetProcChance(SpellEntry* entry, uint32 procChance)
    {
        entry->ProcChance = procChance;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ProcChance = procChance;
        }
    }

    /**
     * Sets the proc charges for the [SpellEntry].
     *
     * @param uint32 procCharges : the new proc charges value
     */
    void SetProcCharges(SpellEntry* entry, uint32 procCharges)
    {
        entry->ProcCharges = procCharges;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ProcCharges = procCharges;
        }
    }

    /**
     * Sets the max level for the [SpellEntry].
     *
     * @param uint32 maxLevel : the new max level value
     */
    void SetMaxLevel(SpellEntry* entry, uint32 maxLevel)
    {
        entry->MaxLevel = maxLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->MaxLevel = maxLevel;
        }
    }

    /**
     * Sets the base level for the [SpellEntry].
     *
     * @param uint32 baseLevel : the new base level value
     */
    void SetBaseLevel(SpellEntry* entry, uint32 baseLevel)
    {
        entry->BaseLevel = baseLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->BaseLevel = baseLevel;
        }
    }

    /**
     * Sets the spell level for the [SpellEntry].
     *
     * @param uint32 spellLevel : the new spell level value
     */
    void SetSpellLevel(SpellEntry* entry, uint32 spellLevel)
    {
        entry->SpellLevel = spellLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellLevel = spellLevel;
        }
    }

    /**
     * Sets the mana cost for the [SpellEntry].
     *
     * @param uint32 manaCost : the new mana cost value
     */
    void SetManaCost(SpellEntry* entry, uint32 manaCost)
    {
        entry->ManaCost = manaCost;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaCost = manaCost;
        }
    }

    /**
     * Sets the power type for the [SpellEntry].
     *
     * @param uint32 powerType : the new power type ID
     */
    void SetPowerType(SpellEntry* entry, uint32 powerType)
    {
        entry->PowerType = powerType;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->PowerType = powerType;
        }
    }

    /**
     * Sets the mana cost per level for the [SpellEntry].
     *
     * @param uint32 manaCostPerlevel : the new mana cost per level value
     */
    void SetManaCostPerlevel(SpellEntry* entry, uint32 manaCostPerlevel)
    {
        entry->ManaCostPerlevel = manaCostPerlevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaCostPerlevel = manaCostPerlevel;
        }
    }

    /**
     * Sets the mana per second for the [SpellEntry].
     *
     * @param uint32 manaPerSecond : the new mana per second value
     */
    void SetManaPerSecond(SpellEntry* entry, uint32 manaPerSecond)
    {
        entry->ManaPerSecond = manaPerSecond;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaPerSecond = manaPerSecond;
        }
    }

    /**
     * Sets the mana per second per level for the [SpellEntry].
     *
     * @param uint32 manaPerSecondPerLevel : the new mana per second per level value
     */
    void SetManaPerSecondPerLevel(SpellEntry* entry, uint32 manaPerSecondPerLevel)
    {
        entry->ManaPerSecondPerLevel = manaPerSecondPerLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaPerSecondPerLevel = manaPerSecondPerLevel;
        }
    }

    /**
     * Sets the speed for the [SpellEntry].
     *
     * @param float speed : the new speed value
     */
    void SetSpeed(SpellEntry* entry, float speed)
    {
        entry->Speed = speed;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Speed = speed;
        }
    }

    /**
     * Sets the stack amount for the [SpellEntry].
     *
     * @param uint32 stackAmount : the new stack amount value
     */
    void SetStackAmount(SpellEntry* entry, uint32 stackAmount)
    {
        entry->StackAmount = stackAmount;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StackAmount = stackAmount;
        }
    }

    /**
     * Sets the equipped item class for the [SpellEntry].
     *
     * @param int32 equippedItemClass : the new equipped item class value
     */
    void SetEquippedItemClass(SpellEntry* entry, int32 equippedItemClass)
    {
        entry->EquippedItemClass = equippedItemClass;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->EquippedItemClass = equippedItemClass;
        }
    }

    /**
     * Sets the equipped item sub class mask for the [SpellEntry].
     *
     * @param int32 equippedItemSubClassMask : the new equipped item sub class mask bitmasks
     */
    void SetEquippedItemSubClassMask(SpellEntry* entry, int32 equippedItemSubClassMask)
    {
        entry->EquippedItemSubClassMask = equippedItemSubClassMask;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->EquippedItemSubClassMask = equippedItemSubClassMask;
        }
    }

    /**
     * Sets the equipped item inventory type mask for the [SpellEntry].
     *
     * @param int32 equippedItemInventoryTypeMask : the new equipped item inventory type mask bitmasks
     */
    void SetEquippedItemInventoryTypeMask(SpellEntry* entry, int32 equippedItemInventoryTypeMask)
    {
        entry->EquippedItemInventoryTypeMask = equippedItemInventoryTypeMask;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->EquippedItemInventoryTypeMask = equippedItemInventoryTypeMask;
        }
    }

    /**
     * Sets the spell icon ID for the [SpellEntry].
     *
     * @param uint32 spellIconID : the new spell icon ID value
     */
    void SetSpellIconID(SpellEntry* entry, uint32 spellIconID)
    {
        entry->SpellIconID = spellIconID;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellIconID = spellIconID;
        }
    }

    /**
     * Sets the active icon ID for the [SpellEntry].
     *
     * @param uint32 activeIconID : the new active icon ID value
     */
    void SetActiveIconID(SpellEntry* entry, uint32 activeIconID)
    {
        entry->ActiveIconID = activeIconID;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ActiveIconID = activeIconID;
        }
    }

    /**
     * Sets the spell priority for the [SpellEntry].
     *
     * @param uint32 spellPriority : the new spell priority value
     */
    void SetSpellPriority(SpellEntry* entry, uint32 spellPriority)
    {
        entry->SpellPriority = spellPriority;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellPriority = spellPriority;
        }
    }

    /**
     * Sets the mana cost percentage for the [SpellEntry].
     *
     * @param uint32 manaCostPercentage : the new mana cost percentage value
     */
    void SetManaCostPercentage(SpellEntry* entry, uint32 manaCostPercentage)
    {
        entry->ManaCostPercentage = manaCostPercentage;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaCostPercentage = manaCostPercentage;
        }
    }

    /**
     * Sets the start recovery category for the [SpellEntry].
     *
     * @param uint32 startRecoveryCategory : the new start recovery category value
     */
    void SetStartRecoveryCategory(SpellEntry* entry, uint32 startRecoveryCategory)
    {
        entry->StartRecoveryCategory = startRecoveryCategory;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StartRecoveryCategory = startRecoveryCategory;
        }
    }

    /**
     * Sets the start recovery time for the [SpellEntry].
     *
     * @param uint32 startRecoveryTime : the new start recovery time value
     */
    void SetStartRecoveryTime(SpellEntry* entry, uint32 startRecoveryTime)
    {
        entry->StartRecoveryTime = startRecoveryTime;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StartRecoveryTime = startRecoveryTime;
        }
    }

    /**
     * Sets the max target level for the [SpellEntry].
     *
     * @param uint32 maxTargetLevel : the new max target level value
     */
    void SetMaxTargetLevel(SpellEntry* entry, uint32 maxTargetLevel)
    {
        entry->MaxTargetLevel = maxTargetLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->MaxTargetLevel = maxTargetLevel;
        }
    }

    /**
     * Sets the spell family name for the [SpellEntry].
     *
     * @param uint32 spellFamilyName : the new spell family name value
     */
    void SetSpellFamilyName(SpellEntry* entry, uint32 spellFamilyName)
    {
        entry->SpellFamilyName = spellFamilyName;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellFamilyName = spellFamilyName;
        }
    }

    /**
     * Sets the max affected targets for the [SpellEntry].
     *
     * @param uint32 maxAffectedTargets : the new max affected targets value
     */
    void SetMaxAffectedTargets(SpellEntry* entry, uint32 maxAffectedTargets)
    {
        entry->MaxAffectedTargets = maxAffectedTargets;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->MaxAffectedTargets = maxAffectedTargets;
        }
    }

    /**
     * Sets the damage class for the [SpellEntry].
     *
     * @param uint32 dmgClass : the new damage class ID value
     */
    void SetDmgClass(SpellEntry* entry, uint32 dmgClass)
    {
        entry->DmgClass = dmgClass;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->DmgClass = dmgClass;
        }
    }

    /**
     * Sets the prevention type for the [SpellEntry].
     *
     * @param uint32 preventionType : the new prevention type ID value
     */
    void SetPreventionType(SpellEntry* entry, uint32 preventionType)
    {
        entry->PreventionType = preventionType;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->PreventionType = preventionType;
        }
    }

    /**
     * Sets the school mask for the [SpellEntry].
     *
     * @param uint32 schoolMask : the new school mask bitmask value
     */
    void SetSchoolMask(SpellEntry* entry, uint32 schoolMask)
    {
        entry->SchoolMask = schoolMask;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SchoolMask = schoolMask;
        }
    }

    /**
     * Sets the rune cost ID for the [SpellEntry].
     *
     * @param uint32 runeCostID : the new rune cost ID value
     */
    void SetRuneCostID(SpellEntry* entry, uint32 runeCostID)
    {
        entry->RuneCostID = runeCostID;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->RuneCostID = runeCostID;
        }
    }
}

void RegisterSpellEntryMethods(sol::state& lua)
{
    sol::usertype<SpellEntry> type = lua.new_usertype<SpellEntry>("SpellEntry", sol::no_constructor);

    type["GetId"]                            = &LuaSpellEntry::GetId;
    type["GetCategory"]                      = &LuaSpellEntry::GetCategory;
    type["GetDispel"]                        = &LuaSpellEntry::GetDispel;
    type["GetMechanic"]                      = &LuaSpellEntry::GetMechanic;
    type["GetAttributes"]                    = &LuaSpellEntry::GetAttributes;
    type["GetAttributesEx"]                  = &LuaSpellEntry::GetAttributesEx;
    type["GetAttributesEx2"]                 = &LuaSpellEntry::GetAttributesEx2;
    type["GetAttributesEx3"]                 = &LuaSpellEntry::GetAttributesEx3;
    type["GetAttributesEx4"]                 = &LuaSpellEntry::GetAttributesEx4;
    type["GetAttributesEx5"]                 = &LuaSpellEntry::GetAttributesEx5;
    type["GetAttributesEx6"]                 = &LuaSpellEntry::GetAttributesEx6;
    type["GetAttributesEx7"]                 = &LuaSpellEntry::GetAttributesEx7;
    type["GetStances"]                       = &LuaSpellEntry::GetStances;
    type["GetStancesNot"]                    = &LuaSpellEntry::GetStancesNot;
    type["GetTargets"]                       = &LuaSpellEntry::GetTargets;
    type["GetTargetCreatureType"]            = &LuaSpellEntry::GetTargetCreatureType;
    type["GetRequiresSpellFocus"]            = &LuaSpellEntry::GetRequiresSpellFocus;
    type["GetFacingCasterFlags"]             = &LuaSpellEntry::GetFacingCasterFlags;
    type["GetCasterAuraState"]               = &LuaSpellEntry::GetCasterAuraState;
    type["GetTargetAuraState"]               = &LuaSpellEntry::GetTargetAuraState;
    type["GetCasterAuraStateNot"]            = &LuaSpellEntry::GetCasterAuraStateNot;
    type["GetTargetAuraStateNot"]            = &LuaSpellEntry::GetTargetAuraStateNot;
    type["GetCasterAuraSpell"]               = &LuaSpellEntry::GetCasterAuraSpell;
    type["GetTargetAuraSpell"]               = &LuaSpellEntry::GetTargetAuraSpell;
    type["GetExcludeCasterAuraSpell"]        = &LuaSpellEntry::GetExcludeCasterAuraSpell;
    type["GetExcludeTargetAuraSpell"]        = &LuaSpellEntry::GetExcludeTargetAuraSpell;
    type["GetCastingTimeIndex"]              = &LuaSpellEntry::GetCastingTimeIndex;
    type["GetRecoveryTime"]                  = &LuaSpellEntry::GetRecoveryTime;
    type["GetCategoryRecoveryTime"]          = &LuaSpellEntry::GetCategoryRecoveryTime;
    type["GetInterruptFlags"]                = &LuaSpellEntry::GetInterruptFlags;
    type["GetAuraInterruptFlags"]            = &LuaSpellEntry::GetAuraInterruptFlags;
    type["GetChannelInterruptFlags"]         = &LuaSpellEntry::GetChannelInterruptFlags;
    type["GetProcFlags"]                     = &LuaSpellEntry::GetProcFlags;
    type["GetProcChance"]                    = &LuaSpellEntry::GetProcChance;
    type["GetProcCharges"]                   = &LuaSpellEntry::GetProcCharges;
    type["GetMaxLevel"]                      = &LuaSpellEntry::GetMaxLevel;
    type["GetBaseLevel"]                     = &LuaSpellEntry::GetBaseLevel;
    type["GetSpellLevel"]                    = &LuaSpellEntry::GetSpellLevel;
    type["GetDurationIndex"]                 = &LuaSpellEntry::GetDurationIndex;
    type["GetPowerType"]                     = &LuaSpellEntry::GetPowerType;
    type["GetManaCost"]                      = &LuaSpellEntry::GetManaCost;
    type["GetManaCostPerlevel"]              = &LuaSpellEntry::GetManaCostPerlevel;
    type["GetManaPerSecond"]                 = &LuaSpellEntry::GetManaPerSecond;
    type["GetManaPerSecondPerLevel"]         = &LuaSpellEntry::GetManaPerSecondPerLevel;
    type["GetRangeIndex"]                    = &LuaSpellEntry::GetRangeIndex;
    type["GetSpeed"]                         = &LuaSpellEntry::GetSpeed;
    type["GetStackAmount"]                   = &LuaSpellEntry::GetStackAmount;
    type["GetTotem"]                         = &LuaSpellEntry::GetTotem;
    type["GetReagent"]                       = &LuaSpellEntry::GetReagent;
    type["GetReagentCount"]                  = &LuaSpellEntry::GetReagentCount;
    type["GetEquippedItemClass"]             = &LuaSpellEntry::GetEquippedItemClass;
    type["GetEquippedItemSubClassMask"]      = &LuaSpellEntry::GetEquippedItemSubClassMask;
    type["GetEquippedItemInventoryTypeMask"] = &LuaSpellEntry::GetEquippedItemInventoryTypeMask;
    type["GetEffect"]                        = &LuaSpellEntry::GetEffect;
    type["GetEffectDieSides"]                = &LuaSpellEntry::GetEffectDieSides;
    type["GetEffectRealPointsPerLevel"]      = &LuaSpellEntry::GetEffectRealPointsPerLevel;
    type["GetEffectBasePoints"]              = &LuaSpellEntry::GetEffectBasePoints;
    type["GetEffectMechanic"]                = &LuaSpellEntry::GetEffectMechanic;
    type["GetEffectImplicitTargetA"]         = &LuaSpellEntry::GetEffectImplicitTargetA;
    type["GetEffectImplicitTargetB"]         = &LuaSpellEntry::GetEffectImplicitTargetB;
    type["GetEffectRadiusIndex"]             = &LuaSpellEntry::GetEffectRadiusIndex;
    type["GetEffectApplyAuraName"]           = &LuaSpellEntry::GetEffectApplyAuraName;
    type["GetEffectAmplitude"]               = &LuaSpellEntry::GetEffectAmplitude;
    type["GetEffectValueMultiplier"]         = &LuaSpellEntry::GetEffectValueMultiplier;
    type["GetEffectChainTarget"]             = &LuaSpellEntry::GetEffectChainTarget;
    type["GetEffectItemType"]                = &LuaSpellEntry::GetEffectItemType;
    type["GetEffectMiscValue"]               = &LuaSpellEntry::GetEffectMiscValue;
    type["GetEffectMiscValueB"]              = &LuaSpellEntry::GetEffectMiscValueB;
    type["GetEffectTriggerSpell"]            = &LuaSpellEntry::GetEffectTriggerSpell;
    type["GetEffectPointsPerComboPoint"]     = &LuaSpellEntry::GetEffectPointsPerComboPoint;
    type["GetEffectSpellClassMask"]          = &LuaSpellEntry::GetEffectSpellClassMask;
    type["GetSpellVisual"]                   = &LuaSpellEntry::GetSpellVisual;
    type["GetSpellIconID"]                   = &LuaSpellEntry::GetSpellIconID;
    type["GetActiveIconID"]                  = &LuaSpellEntry::GetActiveIconID;
    type["GetSpellPriority"]                 = &LuaSpellEntry::GetSpellPriority;
    type["GetSpellName"]                     = &LuaSpellEntry::GetSpellName;
    type["GetRank"]                          = &LuaSpellEntry::GetRank;
    type["GetManaCostPercentage"]            = &LuaSpellEntry::GetManaCostPercentage;
    type["GetStartRecoveryCategory"]         = &LuaSpellEntry::GetStartRecoveryCategory;
    type["GetStartRecoveryTime"]             = &LuaSpellEntry::GetStartRecoveryTime;
    type["GetMaxTargetLevel"]                = &LuaSpellEntry::GetMaxTargetLevel;
    type["GetSpellFamilyName"]               = &LuaSpellEntry::GetSpellFamilyName;
    type["GetSpellFamilyFlags"]              = &LuaSpellEntry::GetSpellFamilyFlags;
    type["GetMaxAffectedTargets"]            = &LuaSpellEntry::GetMaxAffectedTargets;
    type["GetDmgClass"]                      = &LuaSpellEntry::GetDmgClass;
    type["GetPreventionType"]                = &LuaSpellEntry::GetPreventionType;
    type["GetEffectDamageMultiplier"]        = &LuaSpellEntry::GetEffectDamageMultiplier;
    type["GetTotemCategory"]                 = &LuaSpellEntry::GetTotemCategory;
    type["GetAreaGroupId"]                   = &LuaSpellEntry::GetAreaGroupId;
    type["GetSchoolMask"]                    = &LuaSpellEntry::GetSchoolMask;
    type["GetRuneCostID"]                    = &LuaSpellEntry::GetRuneCostID;
    type["GetEffectBonusMultiplier"]         = &LuaSpellEntry::GetEffectBonusMultiplier;
    type["SetCategory"]                      = &LuaSpellEntry::SetCategory;
    type["SetDispel"]                        = &LuaSpellEntry::SetDispel;
    type["SetMechanic"]                      = &LuaSpellEntry::SetMechanic;
    type["SetAttributes"]                    = &LuaSpellEntry::SetAttributes;
    type["SetAttributesEx"]                  = &LuaSpellEntry::SetAttributesEx;
    type["SetAttributesEx2"]                 = &LuaSpellEntry::SetAttributesEx2;
    type["SetAttributesEx3"]                 = &LuaSpellEntry::SetAttributesEx3;
    type["SetAttributesEx4"]                 = &LuaSpellEntry::SetAttributesEx4;
    type["SetAttributesEx5"]                 = &LuaSpellEntry::SetAttributesEx5;
    type["SetAttributesEx6"]                 = &LuaSpellEntry::SetAttributesEx6;
    type["SetAttributesEx7"]                 = &LuaSpellEntry::SetAttributesEx7;
    type["SetStances"]                       = &LuaSpellEntry::SetStances;
    type["SetStancesNot"]                    = &LuaSpellEntry::SetStancesNot;
    type["SetTargets"]                       = &LuaSpellEntry::SetTargets;
    type["SetTargetCreatureType"]            = &LuaSpellEntry::SetTargetCreatureType;
    type["SetRequiresSpellFocus"]            = &LuaSpellEntry::SetRequiresSpellFocus;
    type["SetFacingCasterFlags"]             = &LuaSpellEntry::SetFacingCasterFlags;
    type["SetCasterAuraState"]               = &LuaSpellEntry::SetCasterAuraState;
    type["SetTargetAuraState"]               = &LuaSpellEntry::SetTargetAuraState;
    type["SetCasterAuraStateNot"]            = &LuaSpellEntry::SetCasterAuraStateNot;
    type["SetTargetAuraStateNot"]            = &LuaSpellEntry::SetTargetAuraStateNot;
    type["SetCasterAuraSpell"]               = &LuaSpellEntry::SetCasterAuraSpell;
    type["SetTargetAuraSpell"]               = &LuaSpellEntry::SetTargetAuraSpell;
    type["SetExcludeCasterAuraSpell"]        = &LuaSpellEntry::SetExcludeCasterAuraSpell;
    type["SetExcludeTargetAuraSpell"]        = &LuaSpellEntry::SetExcludeTargetAuraSpell;
    type["SetRecoveryTime"]                  = &LuaSpellEntry::SetRecoveryTime;
    type["SetCategoryRecoveryTime"]          = &LuaSpellEntry::SetCategoryRecoveryTime;
    type["SetInterruptFlags"]                = &LuaSpellEntry::SetInterruptFlags;
    type["SetAuraInterruptFlags"]            = &LuaSpellEntry::SetAuraInterruptFlags;
    type["SetChannelInterruptFlags"]         = &LuaSpellEntry::SetChannelInterruptFlags;
    type["SetProcFlags"]                     = &LuaSpellEntry::SetProcFlags;
    type["SetProcChance"]                    = &LuaSpellEntry::SetProcChance;
    type["SetProcCharges"]                   = &LuaSpellEntry::SetProcCharges;
    type["SetMaxLevel"]                      = &LuaSpellEntry::SetMaxLevel;
    type["SetBaseLevel"]                     = &LuaSpellEntry::SetBaseLevel;
    type["SetSpellLevel"]                    = &LuaSpellEntry::SetSpellLevel;
    type["SetManaCost"]                      = &LuaSpellEntry::SetManaCost;
    type["SetPowerType"]                     = &LuaSpellEntry::SetPowerType;
    type["SetManaCostPerlevel"]              = &LuaSpellEntry::SetManaCostPerlevel;
    type["SetManaPerSecond"]                 = &LuaSpellEntry::SetManaPerSecond;
    type["SetManaPerSecondPerLevel"]         = &LuaSpellEntry::SetManaPerSecondPerLevel;
    type["SetSpeed"]                         = &LuaSpellEntry::SetSpeed;
    type["SetStackAmount"]                   = &LuaSpellEntry::SetStackAmount;
    type["SetEquippedItemClass"]             = &LuaSpellEntry::SetEquippedItemClass;
    type["SetEquippedItemSubClassMask"]      = &LuaSpellEntry::SetEquippedItemSubClassMask;
    type["SetEquippedItemInventoryTypeMask"] = &LuaSpellEntry::SetEquippedItemInventoryTypeMask;
    type["SetSpellIconID"]                   = &LuaSpellEntry::SetSpellIconID;
    type["SetActiveIconID"]                  = &LuaSpellEntry::SetActiveIconID;
    type["SetSpellPriority"]                 = &LuaSpellEntry::SetSpellPriority;
    type["SetManaCostPercentage"]            = &LuaSpellEntry::SetManaCostPercentage;
    type["SetStartRecoveryCategory"]         = &LuaSpellEntry::SetStartRecoveryCategory;
    type["SetStartRecoveryTime"]             = &LuaSpellEntry::SetStartRecoveryTime;
    type["SetMaxTargetLevel"]                = &LuaSpellEntry::SetMaxTargetLevel;
    type["SetSpellFamilyName"]               = &LuaSpellEntry::SetSpellFamilyName;
    type["SetMaxAffectedTargets"]            = &LuaSpellEntry::SetMaxAffectedTargets;
    type["SetDmgClass"]                      = &LuaSpellEntry::SetDmgClass;
    type["SetPreventionType"]                = &LuaSpellEntry::SetPreventionType;
    type["SetSchoolMask"]                    = &LuaSpellEntry::SetSchoolMask;
    type["SetRuneCostID"]                    = &LuaSpellEntry::SetRuneCostID;
}
