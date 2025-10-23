/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef SPELLENTRYMETHODS_H
#define SPELLENTRYMETHODS_H

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
    int GetId(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Id);
        return 1;
    }

    /**
     * Returns the category ID for the [SpellEntry].
     *
     * @return uint32 categoryId
     */
    int GetCategory(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Category);
        return 1;
    }

    /**
     * Returns the dispel ID for the [SpellEntry].
     *
     * @return uint32 dispelId
     */
    int GetDispel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Dispel);
        return 1;
    }

    /**
     * Returns the mechanic ID for the [SpellEntry].
     *
     * @return uint32 mechanicId
     */
    int GetMechanic(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Mechanic);
        return 1;
    }

    /**
     * Returns the attribute bitflags for the [SpellEntry].
     *
     * @return uint32 attribute : bitmask, but returned as uint32
     */
    int GetAttributes(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Attributes);
        return 1;
    }

    /**
     * Returns the attributeEx bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx : bitmask, but returned as uint32
     */
    int GetAttributesEx(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx);
        return 1;
    }

    /**
     * Returns the attributeEx2 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx2 : bitmask, but returned as uint32
     */
    int GetAttributesEx2(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx2);
        return 1;
    }

    /**
     * Returns the attributeEx3 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx3 : bitmask, but returned as uint32
     */
    int GetAttributesEx3(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx3);
        return 1;
    }

    /**
     * Returns the attributeEx4 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx4 : bitmask, but returned as uint32
     */
    int GetAttributesEx4(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx4);
        return 1;
    }

    /**
     * Returns the attributeEx5 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx5 : bitmask, but returned as uint32
     */
    int GetAttributesEx5(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx5);
        return 1;
    }

    /**
     * Returns the attributeEx6 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx6 : bitmask, but returned as uint32
     */
    int GetAttributesEx6(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx6);
        return 1;
    }

    /**
     * Returns the attributeEx7 bitflags for the [SpellEntry].
     *
     * @return uint32 attributeEx7 : bitmask, but returned as uint32
     */
    int GetAttributesEx7(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AttributesEx7);
        return 1;
    }

    /**
     * Returns the stance bitflags for the [SpellEntry].
     *
     * @return uint32 stance : bitmask, but returned as uint32
     */
    int GetStances(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Stances);
        return 1;
    }

    /**
     * Returns the stance restriction bitmask for which the [SpellEntry] cannot be used.
     *
     * This mask indicates which shapeshift forms (stances) prevent the spell from being cast.
     *
     * @return uint32 stancesNotMask
     */
    int GetStancesNot(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->StancesNot);
        return 1;
    }

    /**
     * Returns the target bitmasks for the [SpellEntry].
     *
     * @return uint32 target : bitmasks, but returned as uint32.
     */
    int GetTargets(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Targets);
        return 1;
    }

    /**
     * Returns the target creature type bitmasks for the [SpellEntry].
     *
     * @return uint32 targetCreatureType : bitmasks, but returned as uint32.
     */
    int GetTargetCreatureType(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->TargetCreatureType);
        return 1;
    }

    /**
     * Returns the SpellFocus ID required to cast this [SpellEntry].
     *
     * Some spells require proximity to a specific game object (e.g., a brazier or altar).
     *
     * @return uint32 spellFocusId
     */
    int GetRequiresSpellFocus(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->RequiresSpellFocus);
        return 1;
    }

    /**
     * Returns the facing flags for this [SpellEntry].
     *
     * Indicates whether the caster must be facing the target or meet other orientation constraints.
     *
     * @return uint32 facingFlags
     */
    int GetFacingCasterFlags(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->FacingCasterFlags);
        return 1;
    }

    /**
     * Returns the required caster aura state for this [SpellEntry].
     *
     * The spell can only be cast if the caster has a specific aura state active.
     *
     * @return uint32 casterAuraState
     */
    int GetCasterAuraState(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->CasterAuraState);
        return 1;
    }

    /**
     * Returns the required target aura state for this [SpellEntry].
     *
     * The spell can only be cast if the target has a specific aura state active.
     *
     * @return uint32 targetAuraState
     */
    int GetTargetAuraState(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->TargetAuraState);
        return 1;
    }

    /**
     * Returns the forbidden caster aura state for this [SpellEntry].
     *
     * The spell cannot be cast if the caster has this aura state active.
     *
     * @return uint32 casterAuraStateNot
     */
    int GetCasterAuraStateNot(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->CasterAuraStateNot);
        return 1;
    }

    /**
     * Returns the forbidden target aura state for this [SpellEntry].
     *
     * The spell cannot be cast if the target has this aura state active.
     *
     * @return uint32 targetAuraStateNot
     */
    int GetTargetAuraStateNot(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->TargetAuraStateNot);
        return 1;
    }

    /**
     * Returns the required aura spell ID that must be on the caster.
     *
     * The spell can only be cast if the caster has an aura from this spell.
     *
     * @return uint32 casterAuraSpellId
     */
    int GetCasterAuraSpell(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->CasterAuraSpell);
        return 1;
    }

    /**
     * Returns the required aura spell ID that must be on the target.
     *
     * The spell can only be cast if the target has an aura from this spell.
     *
     * @return uint32 targetAuraSpellId
     */
    int GetTargetAuraSpell(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->TargetAuraSpell);
        return 1;
    }

    /**
     * Returns the aura spell ID that must NOT be on the caster.
     *
     * The spell cannot be cast if the caster has an aura from this spell.
     *
     * @return uint32 excludeCasterAuraSpellId
     */
    int GetExcludeCasterAuraSpell(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ExcludeCasterAuraSpell);
        return 1;
    }

    /**
     * Returns the aura spell ID that must NOT be on the target.
     *
     * The spell cannot be cast if the target has an aura from this spell.
     *
     * @return uint32 excludeTargetAuraSpellId
     */
    int GetExcludeTargetAuraSpell(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ExcludeTargetAuraSpell);
        return 1;
    }

    /**
     * Returns the casting time index of this [SpellEntry].
     *
     * This index is used to look up the base casting time in SpellCastTimes.dbc.
     *
     * @return uint32 castingTimeIndex
     */
    int GetCastingTimeIndex(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->CastingTimeIndex);
        return 1;
    }

    /**
     * Returns the recovery time for the [SpellEntry].
     *
     * @return uint32 recoveryTime
     */
    int GetRecoveryTime(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->RecoveryTime);
        return 1;
    }

    /**
     * Returns the category recovery time for the [SpellEntry].
     *
     * @return uint32 categoryRecoveryTime : in milliseconds, returned as uint32
     */
    int GetCategoryRecoveryTime(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->CategoryRecoveryTime);
        return 1;
    }

    /**
     * Returns the interrupt flags for this [SpellEntry].
     *
     * Determines what can interrupt this spell while casting (e.g., movement, taking damage).
     *
     * @return uint32 interruptFlags
     */
    int GetInterruptFlags(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->InterruptFlags);
        return 1;
    }

    /**
     * Returns the aura interrupt flags for this [SpellEntry].
     *
     * Indicates what actions will break or remove the aura applied by this spell.
     *
     * @return uint32 auraInterruptFlags
     */
    int GetAuraInterruptFlags(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AuraInterruptFlags);
        return 1;
    }

    /**
     * Returns the channel interrupt flags for this [SpellEntry].
     *
     * Specifies conditions under which a channeled spell will be interrupted (e.g., moving or turning).
     *
     * @return uint32 channelInterruptFlags
     */
    int GetChannelInterruptFlags(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ChannelInterruptFlags);
        return 1;
    }

    /**
     * Returns the proc flags for this [SpellEntry].
     *
     * Determines the types of actions or triggers that can cause this spell to proc.
     *
     * @return uint32 procFlags
     */
    int GetProcFlags(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ProcFlags);
        return 1;
    }

    /**
     * Returns the proc chance of [SpellEntry].
     *
     * @return uint32 procChance
     */
    int GetProcChance(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ProcChance);
        return 1;
    }

    /**
     * Returns the proc charges of [SpellEntry].
     *
     * @return uint32 procCharges
     */
    int GetProcCharges(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ProcCharges);
        return 1;
    }

    /**
     * Returns the max level for the [SpellEntry].
     *
     * @return uint32 maxLevel : the [SpellEntry] max level.
     */
    int GetMaxLevel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->MaxLevel);
        return 1;
    }

    /**
     * Returns the base level required for the [SpellEntry].
     *
     * @return uint32 baseLevel
     */
    int GetBaseLevel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->BaseLevel);
        return 1;
    }

    /**
     * Returns the spell level for the [SpellEntry].
     *
     * @return uint32 spellLevel
     */
    int GetSpellLevel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->SpellLevel);
        return 1;
    }

    /**
     * Returns the duration index for the [SpellEntry].
     *
     * @return uint32 durationIndex
     */
    int GetDurationIndex(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->DurationIndex);
        return 1;
    }

    /**
     * Returns the power type ID for the [SpellEntry].
     *
     * @return uint32 powerTypeId
     */
    int GetPowerType(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->PowerType);
        return 1;
    }

    /**
     * Returns the mana cost for the [SpellEntry].
     *
     * @return uint32 manaCost
     */
    int GetManaCost(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ManaCost);
        return 1;
    }

    /**
     * Returns the mana cost per level for [SpellEntry].
     *
     * @return uint32 manaCostPerLevel
     */
    int GetManaCostPerlevel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ManaCostPerlevel);
        return 1;
    }

    /**
     * Returns the mana per second for [SpellEntry].
     *
     * @return uint32 manaPerSecond
     */
    int GetManaPerSecond(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ManaPerSecond);
        return 1;
    }

    /**
     * Returns the mana per second per level for [SpellEntry].
     *
     * @return uint32 manaPerSecondPerLevel
     */
    int GetManaPerSecondPerLevel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ManaPerSecondPerLevel);
        return 1;
    }

    /**
     * Returns the range index for [SpellEntry].
     *
     * @return uint32 rangeIndex
     */
    int GetRangeIndex(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->RangeIndex);
        return 1;
    }

    /**
     * Returns speed for [SpellEntry].
     *
     * @return uint32 speed
     */
    int GetSpeed(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->Speed);
        return 1;
    }

    /**
     * Returns the stack amount for [SpellEntry].
     *
     * @return uint32 stackAmount
     */
    int GetStackAmount(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->StackAmount);
        return 1;
    }

    /**
     * Returns a table with all totem values for [SpellEntry].
     *
     * @return table totem
     */
    int GetTotem(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->Totem.size(); ++index)
        {
            ALE::Push(L, entry->Totem[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all reagent values for [SpellEntry].
     *
     * @return table reagent
     */
    int GetReagent(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->Reagent.size(); ++index)
        {
            ALE::Push(L, entry->Reagent[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all reagent count values for [SpellEntry].
     *
     * @return table reagentCount
     */
    int GetReagentCount(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->ReagentCount.size(); ++index)
        {
            ALE::Push(L, entry->ReagentCount[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns the equipped item class ID for [SpellEntry].
     *
     * @return uint32 equippedItemClassId
     */
    int GetEquippedItemClass(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->EquippedItemClass);
        return 1;
    }

    /**
     * Returns the equipped item sub class masks for [SpellEntry].
     *
     * @return uint32 equippedItemSubClassMasks : bitmasks, returned as uint32.
     */
    int GetEquippedItemSubClassMask(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->EquippedItemSubClassMask);
        return 1;
    }

    /**
     * Returns the equipped item inventory type masks for [SpellEntry].
     *
     * @return uint32 equippedItemInventoryTypeMasks : bitmasks, returned as uint32.
     */
    int GetEquippedItemInventoryTypeMask(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->EquippedItemInventoryTypeMask);
        return 1;
    }

    /**
     * Returns a table with all spell effect IDs for [SpellEntry].
     *
     * @return table effect
     */
    int GetEffect(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->Effect.size(); ++index)
        {
            ALE::Push(L, entry->Effect[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect die sides values for [SpellEntry].
     *
     * @return table effectDieSides
     */
    int GetEffectDieSides(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectDieSides.size(); ++index)
        {
            ALE::Push(L, entry->EffectDieSides[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect real points per level values for [SpellEntry].
     *
     * @return table effectRealPointsPerLevel
     */
    int GetEffectRealPointsPerLevel(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectRealPointsPerLevel.size(); ++index)
        {
            ALE::Push(L, entry->EffectRealPointsPerLevel[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect base points values for [SpellEntry].
     *
     * @return table effectBasePoints
     */
    int GetEffectBasePoints(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectBasePoints.size(); ++index)
        {
            ALE::Push(L, entry->EffectBasePoints[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect mechanic IDs for [SpellEntry].
     *
     * @return table effectMechanic
     */
    int GetEffectMechanic(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectMechanic.size(); ++index)
        {
            ALE::Push(L, entry->EffectMechanic[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect implicit target a IDs for [SpellEntry].
     *
     * @return table effectImplicitTargetA
     */
    int GetEffectImplicitTargetA(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectImplicitTargetA.size(); ++index)
        {
            ALE::Push(L, entry->EffectImplicitTargetA[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect implicit target b IDs for [SpellEntry].
     *
     * @return table effectImplicitTargetB
     */
    int GetEffectImplicitTargetB(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectImplicitTargetB.size(); ++index)
        {
            ALE::Push(L, entry->EffectImplicitTargetB[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect radius index for [SpellEntry].
     *
     * @return table effectRadiusIndex
     */
    int GetEffectRadiusIndex(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectRadiusIndex.size(); ++index)
        {
            ALE::Push(L, entry->EffectRadiusIndex[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect apply aura IDs for [SpellEntry].
     *
     * @return table effectApplyAura
     */
    int GetEffectApplyAuraName(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectApplyAuraName.size(); ++index)
        {
            ALE::Push(L, entry->EffectApplyAuraName[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect amplitude values for [SpellEntry].
     *
     * @return table effectAmplitude
     */
    int GetEffectAmplitude(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectAmplitude.size(); ++index)
        {
            ALE::Push(L, entry->EffectAmplitude[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect value multiplier for [SpellEntry].
     *
     * @return table effectValueMultiplier
     */
    int GetEffectValueMultiplier(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectValueMultiplier.size(); ++index)
        {
            ALE::Push(L, entry->EffectValueMultiplier[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect chain target values for [SpellEntry].
     *
     * @return table effectChainTarget
     */
    int GetEffectChainTarget(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectChainTarget.size(); ++index)
        {
            ALE::Push(L, entry->EffectChainTarget[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect item type values for [SpellEntry].
     *
     * @return table effectItemType
     */
    int GetEffectItemType(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectItemType.size(); ++index)
        {
            ALE::Push(L, entry->EffectItemType[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect misc value A for [SpellEntry].
     *
     * @return table effectMiscValueA
     */
    int GetEffectMiscValue(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectMiscValue.size(); ++index)
        {
            ALE::Push(L, entry->EffectMiscValue[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect misc value B for [SpellEntry].
     *
     * @return table effectMiscValueB
     */
    int GetEffectMiscValueB(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectMiscValueB.size(); ++index)
        {
            ALE::Push(L, entry->EffectMiscValueB[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect trigger spell for [SpellEntry].
     *
     * @return table effectTriggerSpell
     */
    int GetEffectTriggerSpell(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectTriggerSpell.size(); ++index)
        {
            ALE::Push(L, entry->EffectTriggerSpell[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with all effect points per combo point of [SpellEntry]
     *
     * @return table effectPointsPerComboPoint : returns a table containing all the effect points per combo point values of [SpellEntry]
     */
    int GetEffectPointsPerComboPoint(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectPointsPerComboPoint.size(); ++index)
        {
            ALE::Push(L, entry->EffectPointsPerComboPoint[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table of [SpellFamilyFlags] for each effect of this [SpellEntry].
     *
     * These flags are used to categorize spell effects for use with spell group logic.
     * The table contains up to 3 bitmask entries, one per effect.
     *
     * @return table effectSpellClassMask : table of [SpellFamilyFlags] per effect
     */
    int GetEffectSpellClassMask(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectSpellClassMask.size(); ++index)
        {
            ALE::Push(L, entry->EffectSpellClassMask[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with both spell visuals of [SpellEntry]
     *
     * @return table spellVisuals : returns a table containing both spellVisuals for [SpellEntry].
     */
    int GetSpellVisual(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->SpellVisual.size(); ++index)
        {
            ALE::Push(L, entry->SpellVisual[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns the spell icon ID for the [SpellEntry].
     *
     * @return uint32 spellIconId
     */
    int GetSpellIconID(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->SpellIconID);
        return 1;
    }

    /**
     * Returns the active icon ID for the [SpellEntry].
     *
     * @return uint32 activeIconId
     */
    int GetActiveIconID(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ActiveIconID);
        return 1;
    }

    /**
     * Returns the spell Priority for the [SpellEntry].
     *
     * @return uint32 spellPriority
     */
    int GetSpellPriority(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->SpellPriority);
        return 1;
    }

    /**
     * Returns a table of the [SpellEntry] names of all locals.
     *
     * @return table spellNames
     */
    int GetSpellName(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->SpellName.size(); ++index)
        {
            ALE::Push(L, entry->SpellName[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table of the [SpellEntry] ranks.
     *
     * @return table spellRanks
     */
    int GetRank(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->Rank.size(); ++index)
        {
            ALE::Push(L, entry->Rank[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns the mana cost percentage of [SpellEntry].
     *
     * @return uint32 manaCostPercentage : the mana cost in percentage, returned as uint32.
     */
    int GetManaCostPercentage(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->ManaCostPercentage);
        return 1;
    }

    /**
     * Returns the global cooldown time value for [SpellEntry].
     *
     * @return uint32 globalCooldownTime
     */
    int GetStartRecoveryCategory(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->StartRecoveryCategory);
        return 1;
    }

    /**
     * Returns the global cooldown category value for [SpellEntry].
     *
     * @return uint32 globalCooldownCategory
     */
    int GetStartRecoveryTime(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->StartRecoveryTime);
        return 1;
    }

    /**
     * Returns the max target level value for [SpellEntry].
     *
     * @return uint32 maxTargetLevel
     */
    int GetMaxTargetLevel(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->MaxTargetLevel);
        return 1;
    }

    /**
     * Returns the spell family name of this [SpellEntry].
     *
     * This identifies the broader category or class of spells (e.g., Mage, Warrior, Rogue).
     *
     * @return uint32 spellFamilyName
     */
    int GetSpellFamilyName(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->SpellFamilyName);
        return 1;
    }

    /**
     * Returns the spell family flags of this [SpellEntry].
     *
     * These bitflags represent specific characteristics or subcategories of spells within a family.
     *
     * @return uint64 spellFamilyFlags
     */
    int GetSpellFamilyFlags(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->SpellFamilyFlags);
        return 1;
    }

    /**
     * Returns the max affected targets value [SpellEntry].
     *
     * @return uint32 maxAffectedTargets
     */
    int GetMaxAffectedTargets(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->MaxAffectedTargets);
        return 1;
    }

    /**
     * Returns the spell damage type ID [SpellEntry].
     *
     * @return uint32 spellDamageTypeId
     */
    int GetDmgClass(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->DmgClass);
        return 1;
    }

    /**
     * Returns the prevention type ID [SpellEntry].
     *
     * @return uint32 preventionTypeId
     */
    int GetPreventionType(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->PreventionType);
        return 1;
    }

    /**
     * Returns a table with all effect damage multiplier values [SpellEntry].
     *
     * @return table effectDamageMultipliers
     */
    int GetEffectDamageMultiplier(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectDamageMultiplier.size(); ++index)
        {
            ALE::Push(L, entry->EffectDamageMultiplier[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns a table with totem categories IDs [SpellEntry].
     *
     * @return table totemCategory
     */
    int GetTotemCategory(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->TotemCategory.size(); ++index)
        {
            ALE::Push(L, entry->TotemCategory[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Returns the Area Group ID associated with this [SpellEntry].
     *
     * AreaGroupId is used to restrict spell usage to specific zones or areas.
     *
     * @return uint32 areaGroupId
     */
    int GetAreaGroupId(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->AreaGroupId);
        return 1;
    }

    /**
     * Returns the school mask of [SpellEntry].
     *
     * @return uint32 schoolMask : bitmask, returned as uint32.
     */
    int GetSchoolMask(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->SchoolMask);
        return 1;
    }

    /**
     * Returns the rune cost id for the [SpellEntry].
     *
     * @return uint32 runeCostId
     */
    int GetRuneCostID(lua_State* L, SpellEntry* entry)
    {
        ALE::Push(L, entry->RuneCostID);
        return 1;
    }

    /**
     * Returns a table with all effect bonus multiplier values [SpellEntry].
     *
     * @return table effectBonusMultipliers
     */
    int GetEffectBonusMultiplier(lua_State* L, SpellEntry* entry)
    {
        lua_newtable(L);
        int tbl = lua_gettop(L);
        uint32 i = 0;

        for (size_t index = 0; index < entry->EffectBonusMultiplier.size(); ++index)
        {
            ALE::Push(L, entry->EffectBonusMultiplier[index]);
            lua_rawseti(L, tbl, ++i);
        }
        
        lua_settop(L, tbl); // push table to top of stack
        return 1;
    }

    /**
     * Sets the category for the [SpellEntry].
     *
     * @param uint32 category : the new category value
     */
    int SetCategory(lua_State* L, SpellEntry* entry)
    {
        uint32 category = ALE::CHECKVAL<uint32>(L, 2);
        entry->Category = category;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CategoryEntry = category ? sSpellCategoryStore.LookupEntry(category) : nullptr;
        }
        
        return 0;
    }

    /**
     * Sets the dispel type for the [SpellEntry].
     *
     * @param uint32 dispel : the new dispel type value
     */
    int SetDispel(lua_State* L, SpellEntry* entry)
    {
        uint32 dispel = ALE::CHECKVAL<uint32>(L, 2);
        entry->Dispel = dispel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Dispel = dispel;
        }
        
        return 0;
    }

    /**
     * Sets the mechanic for the [SpellEntry].
     *
     * @param uint32 mechanic : the new mechanic value
     */
    int SetMechanic(lua_State* L, SpellEntry* entry)
    {
        uint32 mechanic = ALE::CHECKVAL<uint32>(L, 2);
        entry->Mechanic = mechanic;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Mechanic = mechanic;
        }
        
        return 0;
    }

    /**
     * Sets the attributes for the [SpellEntry].
     *
     * @param uint32 attributes : the new attributes bitmask
     */
    int SetAttributes(lua_State* L, SpellEntry* entry)
    {
        uint32 attributes = ALE::CHECKVAL<uint32>(L, 2);
        entry->Attributes = attributes;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Attributes = attributes;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx for the [SpellEntry].
     *
     * @param uint32 attributesEx : the new attributesEx bitmask
     */
    int SetAttributesEx(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx = attributesEx;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx = attributesEx;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx2 for the [SpellEntry].
     *
     * @param uint32 attributesEx2 : the new attributesEx2 bitmask
     */
    int SetAttributesEx2(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx2 = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx2 = attributesEx2;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx2 = attributesEx2;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx3 for the [SpellEntry].
     *
     * @param uint32 attributesEx3 : the new attributesEx3 bitmask
     */
    int SetAttributesEx3(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx3 = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx3 = attributesEx3;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx3 = attributesEx3;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx4 for the [SpellEntry].
     *
     * @param uint32 attributesEx4 : the new attributesEx4 bitmask
     */
    int SetAttributesEx4(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx4 = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx4 = attributesEx4;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx4 = attributesEx4;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx5 for the [SpellEntry].
     *
     * @param uint32 attributesEx5 : the new attributesEx5 bitmask
     */
    int SetAttributesEx5(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx5 = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx5 = attributesEx5;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx5 = attributesEx5;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx6 for the [SpellEntry].
     *
     * @param uint32 attributesEx6 : the new attributesEx6 bitmask
     */
    int SetAttributesEx6(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx6 = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx6 = attributesEx6;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx6 = attributesEx6;
        }
        
        return 0;
    }

    /**
     * Sets the attributesEx7 for the [SpellEntry].
     *
     * @param uint32 attributesEx7 : the new attributesEx7 bitmask
     */
    int SetAttributesEx7(lua_State* L, SpellEntry* entry)
    {
        uint32 attributesEx7 = ALE::CHECKVAL<uint32>(L, 2);
        entry->AttributesEx7 = attributesEx7;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AttributesEx7 = attributesEx7;
        }
        
        return 0;
    }

    /**
     * Sets the stances for the [SpellEntry].
     *
     * @param uint32 stances : the new stances bitmask
     */
    int SetStances(lua_State* L, SpellEntry* entry)
    {
        uint32 stances = ALE::CHECKVAL<uint32>(L, 2);
        entry->Stances = stances;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Stances = stances;
        }

        return 0;
    }

    /**
     * Sets the stancesNot for the [SpellEntry].
     *
     * @param uint32 stancesNot : the new stancesNot bitmask
     */
    int SetStancesNot(lua_State* L, SpellEntry* entry)
    {
        uint32 stancesNot = ALE::CHECKVAL<uint32>(L, 2);
        entry->StancesNot = stancesNot;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StancesNot = stancesNot;
        }

        return 0;
    }

    /**
     * Sets the targets for the [SpellEntry].
     *
     * @param uint32 targets : the new targets bitmask
     */
    int SetTargets(lua_State* L, SpellEntry* entry)
    {
        uint32 targets = ALE::CHECKVAL<uint32>(L, 2);
        entry->Targets = targets;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Targets = targets;
        }

        return 0;
    }

    /**
     * Sets the target creature type for the [SpellEntry].
     *
     * @param uint32 targetCreatureType : the new target creature type bitmask
     */
    int SetTargetCreatureType(lua_State* L, SpellEntry* entry)
    {
        uint32 targetCreatureType = ALE::CHECKVAL<uint32>(L, 2);
        entry->TargetCreatureType = targetCreatureType;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetCreatureType = targetCreatureType;
        }

        return 0;
    }

    /**
     * Sets the requires spell focus for the [SpellEntry].
     *
     * @param uint32 requiresSpellFocus : the new requires spell focus value
     */
    int SetRequiresSpellFocus(lua_State* L, SpellEntry* entry)
    {
        uint32 requiresSpellFocus = ALE::CHECKVAL<uint32>(L, 2);
        entry->RequiresSpellFocus = requiresSpellFocus;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->RequiresSpellFocus = requiresSpellFocus;
        }

        return 0;
    }

    /**
     * Sets the facing caster flags for the [SpellEntry].
     *
     * @param uint32 facingCasterFlags : the new facing caster flags value
     */
    int SetFacingCasterFlags(lua_State* L, SpellEntry* entry)
    {
        uint32 facingCasterFlags = ALE::CHECKVAL<uint32>(L, 2);
        entry->FacingCasterFlags = facingCasterFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->FacingCasterFlags = facingCasterFlags;
        }

        return 0;
    }

    /**
     * Sets the caster aura state for the [SpellEntry].
     *
     * @param uint32 casterAuraState : the new caster aura state value
     */
    int SetCasterAuraState(lua_State* L, SpellEntry* entry)
    {
        uint32 casterAuraState = ALE::CHECKVAL<uint32>(L, 2);
        entry->CasterAuraState = casterAuraState;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CasterAuraState = casterAuraState;
        }

        return 0;
    }

    /**
     * Sets the target aura state for the [SpellEntry].
     *
     * @param uint32 targetAuraState : the new target aura state value
     */
    int SetTargetAuraState(lua_State* L, SpellEntry* entry)
    {
        uint32 targetAuraState = ALE::CHECKVAL<uint32>(L, 2);
        entry->TargetAuraState = targetAuraState;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetAuraState = targetAuraState;
        }

        return 0;
    }

    /**
     * Sets the caster aura state not for the [SpellEntry].
     *
     * @param uint32 casterAuraStateNot : the new caster aura state not value
     */
    int SetCasterAuraStateNot(lua_State* L, SpellEntry* entry)
    {
        uint32 casterAuraStateNot = ALE::CHECKVAL<uint32>(L, 2);
        entry->CasterAuraStateNot = casterAuraStateNot;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CasterAuraStateNot = casterAuraStateNot;
        }

        return 0;
    }

    /**
     * Sets the target aura state not for the [SpellEntry].
     *
     * @param uint32 targetAuraStateNot : the new target aura state not value
     */
    int SetTargetAuraStateNot(lua_State* L, SpellEntry* entry)
    {
        uint32 targetAuraStateNot = ALE::CHECKVAL<uint32>(L, 2);
        entry->TargetAuraStateNot = targetAuraStateNot;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetAuraStateNot = targetAuraStateNot;
        }

        return 0;
    }

    /**
     * Sets the caster aura spell for the [SpellEntry].
     *
     * @param uint32 casterAuraSpell : the new caster aura spell ID
     */
    int SetCasterAuraSpell(lua_State* L, SpellEntry* entry)
    {
        uint32 casterAuraSpell = ALE::CHECKVAL<uint32>(L, 2);
        entry->CasterAuraSpell = casterAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CasterAuraSpell = casterAuraSpell;
        }

        return 0;
    }

    /**
     * Sets the target aura spell for the [SpellEntry].
     *
     * @param uint32 targetAuraSpell : the new target aura spell ID
     */
    int SetTargetAuraSpell(lua_State* L, SpellEntry* entry)
    {
        uint32 targetAuraSpell = ALE::CHECKVAL<uint32>(L, 2);
        entry->TargetAuraSpell = targetAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->TargetAuraSpell = targetAuraSpell;
        }

        return 0;
    }

    /**
     * Sets the exclude caster aura spell for the [SpellEntry].
     *
     * @param uint32 excludeCasterAuraSpell : the new exclude caster aura spell ID
     */
    int SetExcludeCasterAuraSpell(lua_State* L, SpellEntry* entry)
    {
        uint32 excludeCasterAuraSpell = ALE::CHECKVAL<uint32>(L, 2);
        entry->ExcludeCasterAuraSpell = excludeCasterAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ExcludeCasterAuraSpell = excludeCasterAuraSpell;
        }

        return 0;
    }

    /**
     * Sets the exclude target aura spell for the [SpellEntry].
     *
     * @param uint32 excludeTargetAuraSpell : the new exclude target aura spell ID
     */
    int SetExcludeTargetAuraSpell(lua_State* L, SpellEntry* entry)
    {
        uint32 excludeTargetAuraSpell = ALE::CHECKVAL<uint32>(L, 2);
        entry->ExcludeTargetAuraSpell = excludeTargetAuraSpell;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ExcludeTargetAuraSpell = excludeTargetAuraSpell;
        }

        return 0;
    }

    /**
     * Sets the recovery time for the [SpellEntry].
     *
     * @param uint32 recoveryTime : the new recovery time value
     */
    int SetRecoveryTime(lua_State* L, SpellEntry* entry)
    {
        uint32 recoveryTime = ALE::CHECKVAL<uint32>(L, 2);
        entry->RecoveryTime = recoveryTime;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->RecoveryTime = recoveryTime;
        }

        return 0;
    }

    /**
     * Sets the category recovery time for the [SpellEntry].
     *
     * @param uint32 categoryRecoveryTime : the new category recovery time value in milliseconds
     */
    int SetCategoryRecoveryTime(lua_State* L, SpellEntry* entry)
    {
        uint32 categoryRecoveryTime = ALE::CHECKVAL<uint32>(L, 2);
        entry->CategoryRecoveryTime = categoryRecoveryTime;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->CategoryRecoveryTime = categoryRecoveryTime;
        }

        return 0;
    }

    /**
     * Sets the interrupt flags for the [SpellEntry].
     *
     * @param uint32 interruptFlags : the new interrupt flags bitmask
     */
    int SetInterruptFlags(lua_State* L, SpellEntry* entry)
    {
        uint32 interruptFlags = ALE::CHECKVAL<uint32>(L, 2);
        entry->InterruptFlags = interruptFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->InterruptFlags = interruptFlags;
        }

        return 0;
    }

    /**
     * Sets the aura interrupt flags for the [SpellEntry].
     *
     * @param uint32 auraInterruptFlags : the new aura interrupt flags bitmask
     */
    int SetAuraInterruptFlags(lua_State* L, SpellEntry* entry)
    {
        uint32 auraInterruptFlags = ALE::CHECKVAL<uint32>(L, 2);
        entry->AuraInterruptFlags = auraInterruptFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->AuraInterruptFlags = auraInterruptFlags;
        }

        return 0;
    }

    /**
     * Sets the channel interrupt flags for the [SpellEntry].
     *
     * @param uint32 channelInterruptFlags : the new channel interrupt flags bitmask
     */
    int SetChannelInterruptFlags(lua_State* L, SpellEntry* entry)
    {
        uint32 channelInterruptFlags = ALE::CHECKVAL<uint32>(L, 2);
        entry->ChannelInterruptFlags = channelInterruptFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ChannelInterruptFlags = channelInterruptFlags;
        }

        return 0;
    }

    /**
     * Sets the proc flags for the [SpellEntry].
     *
     * @param uint32 procFlags : the new proc flags bitmask
     */
    int SetProcFlags(lua_State* L, SpellEntry* entry)
    {
        uint32 procFlags = ALE::CHECKVAL<uint32>(L, 2);
        entry->ProcFlags = procFlags;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ProcFlags = procFlags;
        }

        return 0;
    }

    /**
     * Sets the proc chance for the [SpellEntry].
     *
     * @param uint32 procChance : the new proc chance value
     */
    int SetProcChance(lua_State* L, SpellEntry* entry)
    {
        uint32 procChance = ALE::CHECKVAL<uint32>(L, 2);
        entry->ProcChance = procChance;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ProcChance = procChance;
        }

        return 0;
    }

    /**
     * Sets the proc charges for the [SpellEntry].
     *
     * @param uint32 procCharges : the new proc charges value
     */
    int SetProcCharges(lua_State* L, SpellEntry* entry)
    {
        uint32 procCharges = ALE::CHECKVAL<uint32>(L, 2);
        entry->ProcCharges = procCharges;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ProcCharges = procCharges;
        }

        return 0;
    }

    /**
     * Sets the max level for the [SpellEntry].
     *
     * @param uint32 maxLevel : the new max level value
     */
    int SetMaxLevel(lua_State* L, SpellEntry* entry)
    {
        uint32 maxLevel = ALE::CHECKVAL<uint32>(L, 2);
        entry->MaxLevel = maxLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->MaxLevel = maxLevel;
        }

        return 0;
    }

    /**
     * Sets the base level for the [SpellEntry].
     *
     * @param uint32 baseLevel : the new base level value
     */
    int SetBaseLevel(lua_State* L, SpellEntry* entry)
    {
        uint32 baseLevel = ALE::CHECKVAL<uint32>(L, 2);
        entry->BaseLevel = baseLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->BaseLevel = baseLevel;
        }

        return 0;
    }

    /**
     * Sets the spell level for the [SpellEntry].
     *
     * @param uint32 spellLevel : the new spell level value
     */
    int SetSpellLevel(lua_State* L, SpellEntry* entry)
    {
        uint32 spellLevel = ALE::CHECKVAL<uint32>(L, 2);
        entry->SpellLevel = spellLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellLevel = spellLevel;
        }

        return 0;
    }

    /**
     * Sets the mana cost for the [SpellEntry].
     *
     * @param uint32 manaCost : the new mana cost value
     */
    int SetManaCost(lua_State* L, SpellEntry* entry)
    {
        uint32 manaCost = ALE::CHECKVAL<uint32>(L, 2);
        entry->ManaCost = manaCost;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaCost = manaCost;
        }
        
        return 0;
    }

    /**
     * Sets the power type for the [SpellEntry].
     *
     * @param uint32 powerType : the new power type ID
     */
    int SetPowerType(lua_State* L, SpellEntry* entry)
    {
        uint32 powerType = ALE::CHECKVAL<uint32>(L, 2);
        entry->PowerType = powerType;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->PowerType = powerType;
        }
        
        return 0;
    }

    /**
     * Sets the mana cost per level for the [SpellEntry].
     *
     * @param uint32 manaCostPerlevel : the new mana cost per level value
     */
    int SetManaCostPerlevel(lua_State* L, SpellEntry* entry)
    {
        uint32 manaCostPerlevel = ALE::CHECKVAL<uint32>(L, 2);
        entry->ManaCostPerlevel = manaCostPerlevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaCostPerlevel = manaCostPerlevel;
        }
        
        return 0;
    }

    /**
     * Sets the mana per second for the [SpellEntry].
     *
     * @param uint32 manaPerSecond : the new mana per second value
     */
    int SetManaPerSecond(lua_State* L, SpellEntry* entry)
    {
        uint32 manaPerSecond = ALE::CHECKVAL<uint32>(L, 2);
        entry->ManaPerSecond = manaPerSecond;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaPerSecond = manaPerSecond;
        }
        
        return 0;
    }

    /**
     * Sets the mana per second per level for the [SpellEntry].
     *
     * @param uint32 manaPerSecondPerLevel : the new mana per second per level value
     */
    int SetManaPerSecondPerLevel(lua_State* L, SpellEntry* entry)
    {
        uint32 manaPerSecondPerLevel = ALE::CHECKVAL<uint32>(L, 2);
        entry->ManaPerSecondPerLevel = manaPerSecondPerLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaPerSecondPerLevel = manaPerSecondPerLevel;
        }
        
        return 0;
    }

    /**
     * Sets the speed for the [SpellEntry].
     *
     * @param float speed : the new speed value
     */
    int SetSpeed(lua_State* L, SpellEntry* entry)
    {
        float speed = ALE::CHECKVAL<float>(L, 2);
        entry->Speed = speed;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->Speed = speed;
        }
        
        return 0;
    }

    /**
     * Sets the stack amount for the [SpellEntry].
     *
     * @param uint32 stackAmount : the new stack amount value
     */
    int SetStackAmount(lua_State* L, SpellEntry* entry)
    {
        uint32 stackAmount = ALE::CHECKVAL<uint32>(L, 2);
        entry->StackAmount = stackAmount;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StackAmount = stackAmount;
        }

        return 0;
    }

    /**
     * Sets the equipped item class for the [SpellEntry].
     *
     * @param int32 equippedItemClass : the new equipped item class value
     */
    int SetEquippedItemClass(lua_State* L, SpellEntry* entry)
    {
        int32 equippedItemClass = ALE::CHECKVAL<int32>(L, 2);
        entry->EquippedItemClass = equippedItemClass;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->EquippedItemClass = equippedItemClass;
        }

        return 0;
    }

    /**
     * Sets the equipped item sub class mask for the [SpellEntry].
     *
     * @param int32 equippedItemSubClassMask : the new equipped item sub class mask bitmasks
     */
    int SetEquippedItemSubClassMask(lua_State* L, SpellEntry* entry)
    {
        int32 equippedItemSubClassMask = ALE::CHECKVAL<int32>(L, 2);
        entry->EquippedItemSubClassMask = equippedItemSubClassMask;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->EquippedItemSubClassMask = equippedItemSubClassMask;
        }

        return 0;
    }

    /**
     * Sets the equipped item inventory type mask for the [SpellEntry].
     *
     * @param int32 equippedItemInventoryTypeMask : the new equipped item inventory type mask bitmasks
     */
    int SetEquippedItemInventoryTypeMask(lua_State* L, SpellEntry* entry)
    {
        int32 equippedItemInventoryTypeMask = ALE::CHECKVAL<int32>(L, 2);
        entry->EquippedItemInventoryTypeMask = equippedItemInventoryTypeMask;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->EquippedItemInventoryTypeMask = equippedItemInventoryTypeMask;
        }

        return 0;
    }

    /**
     * Sets the spell icon ID for the [SpellEntry].
     *
     * @param uint32 spellIconID : the new spell icon ID value
     */
    int SetSpellIconID(lua_State* L, SpellEntry* entry)
    {
        uint32 spellIconID = ALE::CHECKVAL<uint32>(L, 2);
        entry->SpellIconID = spellIconID;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellIconID = spellIconID;
        }

        return 0;
    }

    /**
     * Sets the active icon ID for the [SpellEntry].
     *
     * @param uint32 activeIconID : the new active icon ID value
     */
    int SetActiveIconID(lua_State* L, SpellEntry* entry)
    {
        uint32 activeIconID = ALE::CHECKVAL<uint32>(L, 2);
        entry->ActiveIconID = activeIconID;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ActiveIconID = activeIconID;
        }

        return 0;
    }

    /**
     * Sets the spell priority for the [SpellEntry].
     *
     * @param uint32 spellPriority : the new spell priority value
     */
    int SetSpellPriority(lua_State* L, SpellEntry* entry)
    {
        uint32 spellPriority = ALE::CHECKVAL<uint32>(L, 2);
        entry->SpellPriority = spellPriority;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellPriority = spellPriority;
        }

        return 0;
    }

    /**
     * Sets the mana cost percentage for the [SpellEntry].
     *
     * @param uint32 manaCostPercentage : the new mana cost percentage value
     */
    int SetManaCostPercentage(lua_State* L, SpellEntry* entry)
    {
        uint32 manaCostPercentage = ALE::CHECKVAL<uint32>(L, 2);
        entry->ManaCostPercentage = manaCostPercentage;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->ManaCostPercentage = manaCostPercentage;
        }
        
        return 0;
    }

    /**
     * Sets the start recovery category for the [SpellEntry].
     *
     * @param uint32 startRecoveryCategory : the new start recovery category value
     */
    int SetStartRecoveryCategory(lua_State* L, SpellEntry* entry)
    {
        uint32 startRecoveryCategory = ALE::CHECKVAL<uint32>(L, 2);
        entry->StartRecoveryCategory = startRecoveryCategory;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StartRecoveryCategory = startRecoveryCategory;
        }

        return 0;
    }

    /**
     * Sets the start recovery time for the [SpellEntry].
     *
     * @param uint32 startRecoveryTime : the new start recovery time value
     */
    int SetStartRecoveryTime(lua_State* L, SpellEntry* entry)
    {
        uint32 startRecoveryTime = ALE::CHECKVAL<uint32>(L, 2);
        entry->StartRecoveryTime = startRecoveryTime;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->StartRecoveryTime = startRecoveryTime;
        }

        return 0;
    }

    /**
     * Sets the max target level for the [SpellEntry].
     *
     * @param uint32 maxTargetLevel : the new max target level value
     */
    int SetMaxTargetLevel(lua_State* L, SpellEntry* entry)
    {
        uint32 maxTargetLevel = ALE::CHECKVAL<uint32>(L, 2);
        entry->MaxTargetLevel = maxTargetLevel;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->MaxTargetLevel = maxTargetLevel;
        }

        return 0;
    }

    /**
     * Sets the spell family name for the [SpellEntry].
     *
     * @param uint32 spellFamilyName : the new spell family name value
     */
    int SetSpellFamilyName(lua_State* L, SpellEntry* entry)
    {
        uint32 spellFamilyName = ALE::CHECKVAL<uint32>(L, 2);
        entry->SpellFamilyName = spellFamilyName;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SpellFamilyName = spellFamilyName;
        }

        return 0;
    }

    /**
     * Sets the max affected targets for the [SpellEntry].
     *
     * @param uint32 maxAffectedTargets : the new max affected targets value
     */
    int SetMaxAffectedTargets(lua_State* L, SpellEntry* entry)
    {
        uint32 maxAffectedTargets = ALE::CHECKVAL<uint32>(L, 2);
        entry->MaxAffectedTargets = maxAffectedTargets;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->MaxAffectedTargets = maxAffectedTargets;
        }

        return 0;
    }

    /**
     * Sets the damage class for the [SpellEntry].
     *
     * @param uint32 dmgClass : the new damage class ID value
     */
    int SetDmgClass(lua_State* L, SpellEntry* entry)
    {
        uint32 dmgClass = ALE::CHECKVAL<uint32>(L, 2);
        entry->DmgClass = dmgClass;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->DmgClass = dmgClass;
        }

        return 0;
    }

    /**
     * Sets the prevention type for the [SpellEntry].
     *
     * @param uint32 preventionType : the new prevention type ID value
     */
    int SetPreventionType(lua_State* L, SpellEntry* entry)
    {
        uint32 preventionType = ALE::CHECKVAL<uint32>(L, 2);
        entry->PreventionType = preventionType;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->PreventionType = preventionType;
        }

        return 0;
    }

    /**
     * Sets the school mask for the [SpellEntry].
     *
     * @param uint32 schoolMask : the new school mask bitmask value
     */
    int SetSchoolMask(lua_State* L, SpellEntry* entry)
    {
        uint32 schoolMask = ALE::CHECKVAL<uint32>(L, 2);
        entry->SchoolMask = schoolMask;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->SchoolMask = schoolMask;
        }

        return 0;
    }

    /**
     * Sets the rune cost ID for the [SpellEntry].
     *
     * @param uint32 runeCostID : the new rune cost ID value
     */
    int SetRuneCostID(lua_State* L, SpellEntry* entry)
    {
        uint32 runeCostID = ALE::CHECKVAL<uint32>(L, 2);
        entry->RuneCostID = runeCostID;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->Id))
        {
            const_cast<SpellInfo*>(spellInfo)->RuneCostID = runeCostID;
        }

        return 0;
    }
}

#endif
