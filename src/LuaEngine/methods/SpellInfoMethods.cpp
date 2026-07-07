/*
* Copyright (C) 2010 - 2024 ALE Lua Engine <https://forgeluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Common.h"
#include "SharedDefines.h"
#include "SpellInfo.h"

/***
 * Represents spell metadata used for behavior, targeting, attributes, mechanics, auras, and conditions.
 *
 * Unlike [SpellEntry], this class includes helper functions and logic used to determine spell behavior in-game.
 * Used for checking if a spell is passive, area-targeted, profession-related, or has specific effects or auras.
 *
 * Inherits all methods from: none
 */
namespace LuaSpellInfo
{

    /**
     * Returns the name of the [SpellInfo]
     *
     * <pre>
     * enum LocaleConstant
     * {
     *     LOCALE_enUS = 0,
     *     LOCALE_koKR = 1,
     *     LOCALE_frFR = 2,
     *     LOCALE_deDE = 3,
     *     LOCALE_zhCN = 4,
     *     LOCALE_zhTW = 5,
     *     LOCALE_esES = 6,
     *     LOCALE_esMX = 7,
     *     LOCALE_ruRU = 8
     * };
     * </pre>
     *
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [SpellInfo]'s name
     * @return [string] name
     */
    char const* GetName(SpellInfo* spell_info, sol::optional<uint8> locale)
    {
        return spell_info->SpellName[static_cast<LocaleConstant>(locale.value_or(DEFAULT_LOCALE))];
    }

    /**
     * Checks if the [SpellInfo] has a specific attribute.
     *
     * Attributes are characteristics or properties that spells can possess.
     * Attributes are divided into different categories (from 0 to 8 in this context).
     *
     * Here is how each attribute is inspected:
     *
     * <pre>
     * 0 : SpellAttr0
     * 1 : SpellAttr1
     * 2 : SpellAttr2
     * 3 : SpellAttr3
     * 4 : SpellAttr4
     * 5 : SpellAttr5
     * 6 : SpellAttr6
     * 7 : SpellAttr7
     * -1 : SpellCustomAttributes
     * </pre>
     *
     * @param [int8] attributeType : the type of the attribute.
     * @param [uint32] attribute : the specific attribute to check.
     * @return [bool] has_attribute
     */
    bool HasAttribute(SpellInfo* spell_info, int8 attributeType, uint32 attribute)
    {
        bool hasAttribute = false;
        if (attributeType == -1)
            hasAttribute = spell_info->HasAttribute(static_cast<SpellCustomAttributes>(attribute));
        else
        {
            switch (attributeType)
            {
                case 0:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr0>(attribute));
                    break;
                case 1:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr1>(attribute));
                    break;
                case 2:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr2>(attribute));
                    break;
                case 3:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr3>(attribute));
                    break;
                case 4:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr4>(attribute));
                    break;
                case 5:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr5>(attribute));
                    break;
                case 6:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr6>(attribute));
                    break;
                case 7:
                    hasAttribute = spell_info->HasAttribute(static_cast<SpellAttr7>(attribute));
                    break;
                case -1:
                    break;
            }
        }

        return hasAttribute;
    }

    /**
     * Retrieves the attributes of the [SpellInfo] based on the attribute type.
     *
     * Attributes are properties or traits of a spell. There are different categories (0 to 8 in this case) of attributes.
     *
     * How each type of attribute is extracted:
     *
     * <pre>
     * 0 : Attributes
     * 1 : AttributesEx
     * 2 : AttributesEx2
     * 3 : AttributesEx3
     * 4 : AttributesEx4
     * 5 : AttributesEx5
     * 6 : AttributesEx6
     * 7 : AttributesEx7
     * -1 : AttributesCu
     * </pre>
     *
     * @param [int8] attributeType : The type of the attribute.
     * @return [uint32] attributes
     */
    uint32 GetAttributes(SpellInfo* spell_info, int8 attributeType)
    {
        uint32 attributes = 0;

        if (attributeType == -1)
            attributes = spell_info->AttributesCu;
        else
        {
            switch (attributeType)
            {
                case 0:
                    attributes = spell_info->Attributes;
                    break;
                case 1:
                    attributes = spell_info->AttributesEx;
                    break;
                case 2:
                    attributes = spell_info->AttributesEx2;
                    break;
                case 3:
                    attributes = spell_info->AttributesEx3;
                    break;
                case 4:
                    attributes = spell_info->AttributesEx4;
                    break;
                case 5:
                    attributes = spell_info->AttributesEx5;
                    break;
                case 6:
                    attributes = spell_info->AttributesEx6;
                    break;
                case 7:
                    attributes = spell_info->AttributesEx7;
                    break;
            }
        }

        return attributes;
    }

    /**
     * Determines whether the [SpellInfo] affects an area (AOE - Area of Effect)
     *
     * The affected area will depend upon the specifics of the spell.
     * A target can be an individual unit, player, or an area, and the spellInfo stores these details.
     *
     * The function checks the spell's attributes to determine if the spell is designed to affect an area or not.
     * The outcome relies on spell's attributes field.
     *
     * @return [bool] is_affecting_area
     */
    bool IsAffectingArea(SpellInfo* spell_info)
    {
        return spell_info->IsAffectingArea();
    }

    /**
     * Retrieves the category of the [SpellInfo].
     *
     * A spell's category is a way of grouping similar spells together.
     * It might define the spell's nature or its effect.
     * For instance, damage spells, heal spells, and crowd-control spells might each have a different category.
     *
     * @return [uint32] category
     */
    uint32 GetCategory(SpellInfo* spell_info)
    {
        return spell_info->GetCategory();
    }

    /**
     * Checks if the [SpellInfo] has a specific effect.
     *
     * A spell can have various effects such as damage, healing, or status changes.
     * These effects are identified by a predefined set of constants represented by the 'SpellEffects' enumeration.
     *
     * @param [uint8] effect : The specific effect to check.
     * @return [bool] has_effect
     */
    bool HasEffect(SpellInfo* spell_info, uint8 effect)
    {
        return spell_info->HasEffect(static_cast<SpellEffects>(effect));
    }

    /**
     * Checks if the [SpellInfo] has a specific aura.
     *
     * An aura represents a status change or modification due to a spell or ability.
     * These auras are identified by a predefined set of constants represented by the 'AuraType' enumeration.
     *
     * @param [uint32] aura : The specific aura to check.
     * @return [bool] has_aura
     */
    bool HasAura(SpellInfo* spell_info, uint32 aura)
    {
        return spell_info->HasAura(static_cast<AuraType>(aura));
    }

    /**
     * Checks if the [SpellInfo] has an area aura effect.
     *
     * Area aura is a type of spell effect that affects multiple targets within a certain area.
     *
     * @return [bool] has_area_aura_effect
     */
    bool HasAreaAuraEffect(SpellInfo* spell_info)
    {
        return spell_info->HasAreaAuraEffect();
    }

    /**
     * Checks if the [SpellInfo] is an explicit discovery.
     *
     * An "explicit discovery" may refer to a spell that is not intuitive or is hidden and must be specifically
     * discovered by the player through some sort of action or event.
     *
     * @return [bool] is_explicit_discovery
     */
    bool IsExplicitDiscovery(SpellInfo* spell_info)
    {
        return spell_info->IsExplicitDiscovery();
    }

    /**
     * Checks if the [SpellInfo] is related to loot crafting.
     *
     * Loot crafting can refer to the process wherein a player uses collected in-game items (loot)
     * to craft or create new items, abilities, or spells.
     *
     * @return [bool] is_loot_crafting
     */
    bool IsLootCrafting(SpellInfo* spell_info)
    {
        return spell_info->IsLootCrafting();
    }

    /**
     * Checks if the [SpellInfo] is related to a Profession skill or Riding skill.
     *
     * Profession skills may refer to a set of abilities related to a particular trade or activity, such as blacksmithing or alchemy.
     * Riding skills are those related to the ability to ride mounts.
     *
     * @return [bool] is_profression_or_riding
     */
    bool IsProfessionOrRiding(SpellInfo* spell_info)
    {
        return spell_info->IsProfessionOrRiding();
    }

    /**
     * Checks if the [SpellInfo] is related to a profession skill.
     *
     * Profession skills may refer to abilities related to a specific occupation or trade,
     * such as blacksmithing, alchemy, fishing, etc.
     *
     * @return [bool] is_profession
     */
    bool IsProfession(SpellInfo* spell_info)
    {
        return spell_info->IsProfession();
    }

    /**
     * Checks if the [SpellInfo] is related to a primary profession skill.
     *
     * Primary profession skills usually refer to main occupations or trades of the player character,
     * such as blacksmithing, alchemy, mining, etc.
     *
     * @return [bool] is_primary_profession
     */
    bool IsPrimaryProfession(SpellInfo* spell_info)
    {
        return spell_info->IsPrimaryProfession();
    }

    /**
     * Checks if the [SpellInfo] represents the first rank of a primary profession skill.
     *
     * Primary profession skills usually refer to main occupations or trades of the player character.
     * The first rank typically indicates the introductory level of the profession.
     *
     * @return [bool] is_primary_profession_first_rank
     */
    bool IsPrimaryProfessionFirstRank(SpellInfo* spell_info)
    {
        return spell_info->IsPrimaryProfessionFirstRank();
    }

    /**
     * Checks if the [SpellInfo] represents an ability learned with a profession skill.
     *
     * Certain abilities or skills (like crafting item or gathering materials)
     * can be learned as part of a profession.
     *
     * @return [bool] is_ability_learned_with_profession
     */
    bool IsAbilityLearnedWithProfession(SpellInfo* spell_info)
    {
        return spell_info->IsAbilityLearnedWithProfession();
    }

    /**
     * Checks if the [SpellInfo] represents an ability of a specific skill type.
     *
     * This function allows checking if a spell or ability belongs to a specific skill type.
     * The skill type is often represented as an integral value (in this case, uint32),
     * where each value may correspond to a different skill category such as crafting, combat, magic, etc.
     *
     * @param [uint32] skillType: The skill type to check against. Should be an integral value representing the skill type.
     * @return [bool] is_ability_of_skill_type
     */
    bool IsAbilityOfSkillType(SpellInfo* spell_info, uint32 skillType)
    {
        return spell_info->IsAbilityOfSkillType(skillType);
    }

    /**
     * Determines if the [SpellInfo] represents a spell or ability that targets an area.
     *
     * Spells or abilities that target an area are typically designed to affect multiple targets within a specified range.
     *
     * @return [bool] is_targeting_area
     */
    bool IsTargetingArea(SpellInfo* spell_info)
    {
        return spell_info->IsTargetingArea();
    }

    /**
     * Checks if the [SpellInfo] requires an explicit unit target.
     *
     * Certain spells or abilities can only be cast or used when a specific unit (like a player character, NPC, or enemy) is targeted.
     * This function checks if the spell or ability represented by [SpellInfo] has this requirement.
     *
     * @return [bool] needs_explicit_unit_target
     */
    bool NeedsExplicitUnitTarget(SpellInfo* spell_info)
    {
        return spell_info->NeedsExplicitUnitTarget();
    }

    /**
     * Checks if the [SpellInfo] requires to be triggered by the caster of another specified [SpellInfo].
     *
     * Certain spells or abilities can only be activated or become effective when they are triggered by the caster
     * of another specific spell (the `triggeringSpell`). This function examines if the spell or ability represented
     * by [SpellInfo] has such requirement.
     *
     * @param [SpellInfo] triggeringSpell : the spell by the casting of which the ability or spell represented by [SpellInfo] is triggered
     * @return [bool] needs_to_be_triggered_by_caster
     */
    bool NeedsToBeTriggeredByCaster(SpellInfo* spell_info, SpellInfo const* triggeringSpell)
    {
        return spell_info->NeedsToBeTriggeredByCaster(triggeringSpell);
    }

    /**
     * Checks if the [SpellInfo] represents a self-casting spell or ability.
     *
     * Self-casting spells or abilities are those that the casters use on themselves. This can include
     * defensive spells, healing spells, buffs, or any other type of effect that a player character or
     * NPC applies on themselves.
     *
     * @return [bool] is_self_cast
     */
    bool IsSelfCast(SpellInfo* spell_info)
    {
        return spell_info->IsSelfCast();
    }

    /**
     * Checks if the [SpellInfo] represents a passive spell or ability.
     *
     * Passive spells or abilities are those that are always in effect, without the need for the player or
     * NPC to manually activate them. They usually provide their bonus or effect as long as certain conditions are met.
     *
     * @return [bool] is_passive
     */
    bool IsPassive(SpellInfo* spell_info)
    {
        return spell_info->IsPassive();
    }

    /**
     * Checks if the [SpellInfo] represents a spell or ability that can be set to autocast.
     *
     * Autocasting is a feature that allows certain abilities or spells to be cast automatically by the game's
     * AI when certain conditions are met. This function checks if the spell or ability represented by [SpellInfo]
     * can be set to autocast.
     *
     * @return [bool] is_autocastable
     */
    bool IsAutocastable(SpellInfo* spell_info)
    {
        return spell_info->IsAutocastable();
    }

    /**
     * Determines if the [SpellInfo] represents a spell or ability that stack with different ranks.
     *
     * Some spells or abilities can accumulate or "stack" their effects with multiple activations
     * and these effects can sometimes vary based on the rank or level of the spell. This function checks
     * if the spell represented by [SpellInfo] has this capacity.
     *
     * @return [bool] is_stackable_with_ranks
     */
    bool IsStackableWithRanks(SpellInfo* spell_info)
    {
        return spell_info->IsStackableWithRanks();
    }

    /**
     * Checks if the [SpellInfo] represents a passive spell or ability that is stackable with different ranks.
     *
     * Some passive spells or abilities are designed to stack their effects with multiple activations, and these effects
     * can also vary depending on the rank of the spell. This function assesses whether the spell or ability represented
     * by [SpellInfo] has this property.
     *
     * @return [bool] is_passive_stackable_with_ranks
     */
    bool IsPassiveStackableWithRanks(SpellInfo* spell_info)
    {
        return spell_info->IsPassiveStackableWithRanks();
    }

    /**
     * Checks if the [SpellInfo] represents a multi-slot aura spell or effect.
     *
     * A multi-slot aura is one that takes up more than one slot or position in the game's effect array or system.
     * This function checks if the spell or ability represented by [SpellInfo] has this property.
     *
     * @return [bool] is_multi_slot_aura
     */
    bool IsMultiSlotAura(SpellInfo* spell_info)
    {
        return spell_info->IsMultiSlotAura();
    }

    /**
     * Returns a boolean indicating whether the cooldown has started on the event associated with the [SpellInfo]
     *
     * @return [bool] is_cooldown_started_on_event
     */
    bool IsCooldownStartedOnEvent(SpellInfo* spell_info)
    {
        return spell_info->IsCooldownStartedOnEvent();
    }

    /**
     * Returns a boolean indicating whether the death is persistent for the given [SpellInfo]
     *
     * @return [bool] is_death_persistant
     */
    bool IsDeathPersistent(SpellInfo* spell_info)
    {
        return spell_info->IsDeathPersistent();
    }

    /**
     * Returns a boolean indicating whether the [SpellInfo] requires a dead target
     *
     * @return [bool] : true if the [SpellInfo] requires a dead target; false otherwise
     */
    bool IsRequiringDeadTarget(SpellInfo* spell_info)
    {
        return spell_info->IsRequiringDeadTarget();
    }

    /**
     * Returns `true` if the [SpellInfo] allows casting on dead targets, `false` otherwise.
     *
     * @return bool allowsDeadTarget
     */
    bool IsAllowingDeadTarget(SpellInfo* spell_info)
    {
        return spell_info->IsAllowingDeadTarget();
    }

    /**
     * Returns `true` if the [SpellInfo] can be cast while in combat, `false` otherwise.
     *
     * @return bool usableInCombat
     */
    bool CanBeUsedInCombat(SpellInfo* spell_info)
    {
        return spell_info->CanBeUsedInCombat();
    }

    /**
     * Returns `true` if the [SpellInfo] is considered a positive (beneficial) spell, `false` otherwise.
     *
     * @return bool isPositive
     */
    bool IsPositive(SpellInfo* spell_info)
    {
        return spell_info->IsPositive();
    }

    /**
     * Returns `true` if the specified effect index of the [SpellInfo] is positive, `false` otherwise.
     *
     * @param uint8 effIndex
     * @return bool isPositiveEffect
     */
    bool IsPositiveEffect(SpellInfo* spell_info, uint8 effIndex)
    {
        return spell_info->IsPositiveEffect(effIndex);
    }

    /**
     * Returns `true` if the [SpellInfo] is a channeled spell, `false` otherwise.
     *
     * @return bool isChanneled
     */
    bool IsChanneled(SpellInfo* spell_info)
    {
        return spell_info->IsChanneled();
    }

    /**
     * Returns `true` if the [SpellInfo] requires combo points to cast, `false` otherwise.
     *
     * @return bool needsComboPoints
     */
    bool NeedsComboPoints(SpellInfo* spell_info)
    {
        return spell_info->NeedsComboPoints();
    }

    /**
     * Returns `true` if the [SpellInfo] breaks stealth when cast, `false` otherwise.
     *
     * @return bool breaksStealth
     */
    bool IsBreakingStealth(SpellInfo* spell_info)
    {
        return spell_info->IsBreakingStealth();
    }

    /**
     * Returns `true` if the [SpellInfo] is a ranged weapon attack (e.g., shoot, throw), `false` otherwise.
     *
     * @return bool isRangedWeaponSpell
     */
    bool IsRangedWeaponSpell(SpellInfo* spell_info)
    {
        return spell_info->IsRangedWeaponSpell();
    }

    /**
     * Returns `true` if the [SpellInfo] is an auto-repeat ranged spell (e.g., auto-shot), `false` otherwise.
     *
     * @return bool isAutoRepeat
     */
    bool IsAutoRepeatRangedSpell(SpellInfo* spell_info)
    {
        return spell_info->IsAutoRepeatRangedSpell();
    }

    /**
     * Returns `true` if the [SpellInfo] is affected by spell modifiers (e.g., talents, auras), `false` otherwise.
     *
     * @return bool isAffectedByMods
     */
    bool IsAffectedBySpellMods(SpellInfo* spell_info)
    {
        return spell_info->IsAffectedBySpellMods();
    }

    /*  bool IsAffectedBySpellMod(SpellInfo* spell_info, SpellInfo const* auraSpellInfo)
        {
            return spell_info->IsAffectedBySpellMod(auraSpellInfo);
        }
    */

    /**
     * Returns `true` if the [SpellInfo] can pierce through an immunity aura defined by the given [SpellInfo], `false` otherwise.
     *
     * @param [SpellInfo] auraSpellInfo : the spell representing the immunity aura
     * @return bool canPierce
     */
    bool CanPierceImmuneAura(SpellInfo* spell_info, SpellInfo const* auraSpellInfo)
    {
        return spell_info->CanPierceImmuneAura(auraSpellInfo);
    }

    /**
     * Returns `true` if the [SpellInfo] can dispel the specified aura [SpellInfo], `false` otherwise.
     *
     * @param [SpellInfo] auraSpellInfo : the aura spell to check
     * @return bool canDispel
     */
    bool CanDispelAura(SpellInfo* spell_info, SpellInfo const* auraSpellInfo)
    {
        return spell_info->CanDispelAura(auraSpellInfo);
    }

    /**
     * Returns `true` if the [SpellInfo] only affects a single target, `false` if it affects multiple or area targets.
     *
     * @return bool isSingleTarget
     */
    bool IsSingleTarget(SpellInfo* spell_info)
    {
        return spell_info->IsSingleTarget();
    }

    /**
     * Returns `true` if the [SpellInfo] is mutually exclusive with the specified [SpellInfo] due to specific aura exclusivity rules.
     *
     * @param [SpellInfo] otherSpellInfo : the spell to compare exclusivity with
     * @return bool isExclusive
     */
    bool IsAuraExclusiveBySpecificWith(SpellInfo* spell_info, SpellInfo const* spellInfo)
    {
        return spell_info->IsAuraExclusiveBySpecificWith(spellInfo);
    }

    /**
     * Returns `true` if the [SpellInfo] is exclusive with the specified [SpellInfo] per caster, based on aura exclusivity rules.
     *
     * @param [SpellInfo] otherSpellInfo : the spell to compare exclusivity with
     * @return bool isExclusivePerCaster
     */
    bool IsAuraExclusiveBySpecificPerCasterWith(SpellInfo* spell_info, SpellInfo const* spellInfo)
    {
        return spell_info->IsAuraExclusiveBySpecificPerCasterWith(spellInfo);
    }

    /**
     * Returns `true` if the [SpellInfo] can be cast while in the specified shapeshift form.
     *
     * @param uint32 form : the shapeshift form to check
     * @return bool isAllowed
     */
    SpellCastResult CheckShapeshift(SpellInfo* spell_info, uint32 form)
    {
        return spell_info->CheckShapeshift(form);
    }

    /**
     * Returns `true` if the [SpellInfo] can be cast in the specified location.
     *
     * @param uint32 map_id : required map ID
     * @param uint32 zone_id : required zone ID
     * @param uint32 area_id : required area ID
     * @param [Player] player : the [Player] casting the spell
     * @param bool strict = false : whether all conditions must strictly match
     * @return bool isAllowed
     */
    SpellCastResult CheckLocation(SpellInfo* spell_info, uint32 map_id, uint32 zone_id, uint32 area_id, Player* player, sol::optional<bool> strict)
    {
        return spell_info->CheckLocation(map_id, zone_id, area_id, player, strict.value_or(false));
    }

    /**
     * Returns `true` if the target is valid for the [SpellInfo].
     *
     * @param [Unit] caster : the [Unit] casting the spell
     * @param [WorldObject] target : the intended target
     * @param bool implicit = true : whether implicit target checks should apply
     * @return bool isValid
     */
    SpellCastResult CheckTarget(SpellInfo* spell_info, Unit const* caster, WorldObject const* target, sol::optional<bool> implicit)
    {
        return spell_info->CheckTarget(caster, target, implicit.value_or(true));
    }

    /**
     * Returns `true` if the [SpellInfo] can be explicitly cast on the given [target] with the optional [Item].
     *
     * @param [Unit] caster : the [Unit] attempting to cast the spell
     * @param [WorldObject] target : the intended target of the spell
     * @param [Item] item : optional item used in the cast
     * @return bool isValid
     */
    SpellCastResult CheckExplicitTarget(SpellInfo* spell_info, Unit const* caster, WorldObject const* target, Item const* item)
    {
        return spell_info->CheckExplicitTarget(caster, target, item);
    }

    /**
     * Returns `true` if the [SpellInfo] can affect the [Unit] based on its creature type.
     *
     * @param [Unit] target : the [Unit] whose creature type is evaluated
     * @return bool isValid
     */
    bool CheckTargetCreatureType(SpellInfo* spell_info, Unit const* target)
    {
        return spell_info->CheckTargetCreatureType(target);
    }

    /**
     * Returns the school mask of the [SpellInfo].
     *
     * The school mask is a bitmask representing the spell's school(s), such as arcane, fire, frost, etc.
     *
     * @return uint32 schoolMask
     */
    SpellSchoolMask GetSchoolMask(SpellInfo* spell_info)
    {
        return spell_info->GetSchoolMask();
    }

    /**
     * Returns a combined mechanic mask of all effects for the [SpellInfo].
     *
     * The mechanic mask is a bitmask representing all mechanics applied by the spell’s effects.
     *
     * @return uint32 mechanicMask
     */
    uint64 GetAllEffectsMechanicMask(SpellInfo* spell_info)
    {
        return spell_info->GetAllEffectsMechanicMask();
    }

    /**
     * Returns the mechanic mask of a specific effect of the [SpellInfo].
     *
     * @param uint32 effIndex
     * @return uint32 mechanicMask
     */
    uint64 GetEffectMechanicMask(SpellInfo* spell_info, uint32 effIndex)
    {
        return spell_info->GetEffectMechanicMask(static_cast<SpellEffIndex>(effIndex));
    }

    /**
     * Returns the mechanic mask for the [SpellInfo] based on an effect bitmask.
     *
     * @param uint32 effectmask : bitmask of effects to include
     * @return uint32 mechanicMask
     */
    uint64 GetSpellMechanicMaskByEffectMask(SpellInfo* spell_info, uint32 effectmask)
    {
        return spell_info->GetSpellMechanicMaskByEffectMask(effectmask);
    }

    /**
     * Returns the mechanic of the specified effect index in the [SpellInfo].
     *
     * @param uint32 effIndex
     * @return uint32 mechanic
     */
    Mechanics GetEffectMechanic(SpellInfo* spell_info, uint32 effIndex)
    {
        return spell_info->GetEffectMechanic(static_cast<SpellEffIndex>(effIndex));
    }

    /**
     * Returns the dispel mask for the [SpellInfo].
     *
     * The dispel mask is a bitmask representing the types of dispels that can remove the spell's effects.
     *
     * @param uint32 type : optional type of dispel to check. If not provided, uses the spell's own dispel type.
     * @return uint32 dispelMask
     */
    uint32 GetDispelMask(SpellInfo* spell_info, sol::optional<uint32> type)
    {
        uint32 dispelType = type.value_or(0);
        return dispelType != 0 ? spell_info->GetDispelMask(static_cast<DispelType>(dispelType)) : spell_info->GetDispelMask();
    }

    /**
     * Returns the explicit target mask of the [SpellInfo].
     *
     * This mask defines what types of targets the spell can explicitly target.
     *
     * @return uint32 targetMask
     */
    uint32 GetExplicitTargetMask(SpellInfo* spell_info)
    {
        return spell_info->GetExplicitTargetMask();
    }

    /**
     * Returns the aura state requirement for the [SpellInfo].
     *
     * Used to check whether a specific aura state must be active to cast the spell.
     *
     * @return uint32 auraState
     */
    AuraStateType GetAuraState(SpellInfo* spell_info)
    {
        return spell_info->GetAuraState();
    }

    /**
     * Returns the spell specific type of the [SpellInfo].
     *
     * Useful for identifying special types such as food, bandages, portals, etc.
     *
     * @return uint32 spellSpecific
     */
    SpellSpecificType GetSpellSpecific(SpellInfo* spell_info)
    {
        return spell_info->GetSpellSpecific();
    }

    /**
    * Retrieves the MiscValueA of a spell effect at the specified index.
    *
    * MiscValueA contains additional information about the effect, such as:
    * - Which stat is affected for stat modifiers
    * - School mask for resistance changes
    * - Item class for item creation effects
    * - And more depending on the effect type
    *
    * @param uint8 effectIndex : The index of the effect (0, 1, or 2)
    * @return int32 miscValueA : The MiscValueA value, or 0 if the effect doesn't exist
    */
    int32 GetEffectMiscValueA(SpellInfo* spell_info, uint8 effectIndex)
    {
        if (effectIndex >= MAX_SPELL_EFFECTS)
            return 0;

        if (spell_info->Effects[effectIndex].Effect == 0)
            return 0;

        return spell_info->Effects[effectIndex].MiscValue;
    }

    /**
    * Retrieves the MiscValueB of a spell effect at the specified index.
    *
    * MiscValueB contains secondary information about the effect.
    *
    * @param uint8 effectIndex : The index of the effect (0, 1, or 2)
    * @return int32 miscValueB : The MiscValueB value, or 0 if the effect doesn't exist
    */
    int32 GetEffectMiscValueB(SpellInfo* spell_info, uint8 effectIndex)
    {
        if (effectIndex >= MAX_SPELL_EFFECTS)
            return 0;

        if (spell_info->Effects[effectIndex].Effect == 0)
            return 0;

        return spell_info->Effects[effectIndex].MiscValueB;
    }
}

void RegisterSpellInfoMethods(sol::state& lua)
{
    sol::usertype<SpellInfo> type = lua.new_usertype<SpellInfo>("SpellInfo", sol::no_constructor);

    type["GetName"]                                = &LuaSpellInfo::GetName;
    type["HasAttribute"]                           = &LuaSpellInfo::HasAttribute;
    type["GetAttributes"]                          = &LuaSpellInfo::GetAttributes;
    type["IsAffectingArea"]                        = &LuaSpellInfo::IsAffectingArea;
    type["GetCategory"]                            = &LuaSpellInfo::GetCategory;
    type["HasEffect"]                              = &LuaSpellInfo::HasEffect;
    type["HasAura"]                                = &LuaSpellInfo::HasAura;
    type["HasAreaAuraEffect"]                      = &LuaSpellInfo::HasAreaAuraEffect;
    type["IsExplicitDiscovery"]                    = &LuaSpellInfo::IsExplicitDiscovery;
    type["IsLootCrafting"]                         = &LuaSpellInfo::IsLootCrafting;
    type["IsProfessionOrRiding"]                   = &LuaSpellInfo::IsProfessionOrRiding;
    type["IsProfession"]                           = &LuaSpellInfo::IsProfession;
    type["IsPrimaryProfession"]                    = &LuaSpellInfo::IsPrimaryProfession;
    type["IsPrimaryProfessionFirstRank"]           = &LuaSpellInfo::IsPrimaryProfessionFirstRank;
    type["IsAbilityLearnedWithProfession"]         = &LuaSpellInfo::IsAbilityLearnedWithProfession;
    type["IsAbilityOfSkillType"]                   = &LuaSpellInfo::IsAbilityOfSkillType;
    type["IsTargetingArea"]                        = &LuaSpellInfo::IsTargetingArea;
    type["NeedsExplicitUnitTarget"]                = &LuaSpellInfo::NeedsExplicitUnitTarget;
    type["NeedsToBeTriggeredByCaster"]             = &LuaSpellInfo::NeedsToBeTriggeredByCaster;
    type["IsSelfCast"]                             = &LuaSpellInfo::IsSelfCast;
    type["IsPassive"]                              = &LuaSpellInfo::IsPassive;
    type["IsAutocastable"]                         = &LuaSpellInfo::IsAutocastable;
    type["IsStackableWithRanks"]                   = &LuaSpellInfo::IsStackableWithRanks;
    type["IsPassiveStackableWithRanks"]            = &LuaSpellInfo::IsPassiveStackableWithRanks;
    type["IsMultiSlotAura"]                        = &LuaSpellInfo::IsMultiSlotAura;
    type["IsCooldownStartedOnEvent"]               = &LuaSpellInfo::IsCooldownStartedOnEvent;
    type["IsDeathPersistent"]                      = &LuaSpellInfo::IsDeathPersistent;
    type["IsRequiringDeadTarget"]                  = &LuaSpellInfo::IsRequiringDeadTarget;
    type["IsAllowingDeadTarget"]                   = &LuaSpellInfo::IsAllowingDeadTarget;
    type["CanBeUsedInCombat"]                      = &LuaSpellInfo::CanBeUsedInCombat;
    type["IsPositive"]                             = &LuaSpellInfo::IsPositive;
    type["IsPositiveEffect"]                       = &LuaSpellInfo::IsPositiveEffect;
    type["IsChanneled"]                            = &LuaSpellInfo::IsChanneled;
    type["NeedsComboPoints"]                       = &LuaSpellInfo::NeedsComboPoints;
    type["IsBreakingStealth"]                      = &LuaSpellInfo::IsBreakingStealth;
    type["IsRangedWeaponSpell"]                    = &LuaSpellInfo::IsRangedWeaponSpell;
    type["IsAutoRepeatRangedSpell"]                = &LuaSpellInfo::IsAutoRepeatRangedSpell;
    type["IsAffectedBySpellMods"]                  = &LuaSpellInfo::IsAffectedBySpellMods;
    type["CanPierceImmuneAura"]                    = &LuaSpellInfo::CanPierceImmuneAura;
    type["CanDispelAura"]                          = &LuaSpellInfo::CanDispelAura;
    type["IsSingleTarget"]                         = &LuaSpellInfo::IsSingleTarget;
    type["IsAuraExclusiveBySpecificWith"]          = &LuaSpellInfo::IsAuraExclusiveBySpecificWith;
    type["IsAuraExclusiveBySpecificPerCasterWith"] = &LuaSpellInfo::IsAuraExclusiveBySpecificPerCasterWith;
    type["CheckShapeshift"]                        = &LuaSpellInfo::CheckShapeshift;
    type["CheckLocation"]                          = ALEBind::Function(&LuaSpellInfo::CheckLocation);
    type["CheckTarget"]                            = ALEBind::Function(&LuaSpellInfo::CheckTarget);
    type["CheckExplicitTarget"]                    = ALEBind::Function(&LuaSpellInfo::CheckExplicitTarget);
    type["CheckTargetCreatureType"]                = ALEBind::Function(&LuaSpellInfo::CheckTargetCreatureType);
    type["GetSchoolMask"]                          = &LuaSpellInfo::GetSchoolMask;
    type["GetAllEffectsMechanicMask"]              = &LuaSpellInfo::GetAllEffectsMechanicMask;
    type["GetEffectMechanicMask"]                  = &LuaSpellInfo::GetEffectMechanicMask;
    type["GetSpellMechanicMaskByEffectMask"]       = &LuaSpellInfo::GetSpellMechanicMaskByEffectMask;
    type["GetEffectMechanic"]                      = &LuaSpellInfo::GetEffectMechanic;
    type["GetDispelMask"]                          = &LuaSpellInfo::GetDispelMask;
    type["GetExplicitTargetMask"]                  = &LuaSpellInfo::GetExplicitTargetMask;
    type["GetAuraState"]                           = &LuaSpellInfo::GetAuraState;
    type["GetSpellSpecific"]                       = &LuaSpellInfo::GetSpellSpecific;
    type["GetEffectMiscValueA"]                    = &LuaSpellInfo::GetEffectMiscValueA;
    type["GetEffectMiscValueB"]                    = &LuaSpellInfo::GetEffectMiscValueB;
}
