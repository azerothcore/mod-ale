/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"
#include "ALEUtility.h"

#include "CellImpl.h"
#include "Chat.h"
#include "DBCStores.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "MotionMaster.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "WorldPacket.h"
#include "WorldSession.h"

/***
 * Represents a non-[Player] controlled [Unit] (i.e. NPCs).
 *
 * Inherits all methods from: [Object], [WorldObject]
 */
namespace LuaUnit
{
    /**
    * Sets a mechanic immunity for the [Unit].
    *
    * <pre>
    *   MECHANIC_NONE             = 0,
    *   MECHANIC_CHARM            = 1,
    *   MECHANIC_DISORIENTED      = 2,
    *   MECHANIC_DISARM           = 3,
    *   MECHANIC_DISTRACT         = 4,
    *   MECHANIC_FEAR             = 5,
    *   MECHANIC_GRIP             = 6,
    *   MECHANIC_ROOT             = 7,
    *   MECHANIC_SLOW_ATTACK      = 8,
    *   MECHANIC_SILENCE          = 9,
    *   MECHANIC_SLEEP            = 10,
    *   MECHANIC_SNARE            = 11,
    *   MECHANIC_STUN             = 12,
    *   MECHANIC_FREEZE           = 13,
    *   MECHANIC_KNOCKOUT         = 14,
    *   MECHANIC_BLEED            = 15,
    *   MECHANIC_BANDAGE          = 16,
    *   MECHANIC_POLYMORPH        = 17,
    *   MECHANIC_BANISH           = 18,
    *   MECHANIC_SHIELD           = 19,
    *   MECHANIC_SHACKLE          = 20,
    *   MECHANIC_MOUNT            = 21,
    *   MECHANIC_INFECTED         = 22,
    *   MECHANIC_TURN             = 23,
    *   MECHANIC_HORROR           = 24,
    *   MECHANIC_INVULNERABILITY  = 25,
    *   MECHANIC_INTERRUPT        = 26,
    *   MECHANIC_DAZE             = 27,
    *   MECHANIC_DISCOVERY        = 28,
    *   MECHANIC_IMMUNE_SHIELD    = 29,     // Divine (Blessing) Shield/Protection and Ice Block
    *   MECHANIC_SAPPED           = 30,
    *   MECHANIC_ENRAGED          = 31
    * </pre>
    *
    * @param int32 immunity : new value for the immunity mask
    * @param bool apply = true : if true, the immunity is applied, otherwise it is removed
    */
    void SetImmuneTo(Unit* unit, int32 immunity, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);

        unit->ApplySpellImmune(0, 5, immunity, apply);
    }

    /**
     * The [Unit] modifies a specific stat
     *
     * @param int32 stat : The stat to modify
     * @param int8 type : The type of modifier to apply
     * @param float value : The value to apply to the stat
     * @param bool apply = false : Whether the modifier should be applied or removed
     * @return bool : Whether the stat modification was successful
     */
    bool HandleStatFlatModifier(Unit* unit, int32 stat, int8 type, float value, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(false);

        return unit->HandleStatFlatModifier(UnitMods(UNIT_MOD_STAT_START + stat), (UnitModifierFlatType)type, value, apply);
    }

    /**
     * The [Unit] tries to attack a given target
     *
     * @param [Unit] who : [Unit] to attack
     * @param bool meleeAttack = false: attack with melee or not
     * @return didAttack : if the [Unit] did not attack
     */
    bool Attack(Unit* unit, Unit* who, sol::optional<bool> meleeAttackArg)
    {
        bool meleeAttack = meleeAttackArg.value_or(false);

        return unit->Attack(who, meleeAttack);
    }

    /**
     * The [Unit] stops attacking its target
     *
     * @return bool isAttacking : if the [Unit] wasn't attacking already
     */
    bool AttackStop(Unit* unit)
    {
        return unit->AttackStop();
    }

    /**
     * Returns true if the [Unit] is standing.
     *
     * @return bool isStanding
     */
    bool IsStandState(Unit* unit)
    {
        return unit->IsStandState();
    }

    /**
     * Returns true if the [Unit] is mounted.
     *
     * @return bool isMounted
     */
    bool IsMounted(Unit* unit)
    {
        return unit->IsMounted();
    }

    /**
     * Returns true if the [Unit] is rooted.
     *
     * @return bool isRooted
     */
    bool IsRooted(Unit* unit)
    {
        return unit->HasRootAura() || unit->HasUnitMovementFlag(MOVEMENTFLAG_ROOT);
    }

    /**
     * Returns true if the [Unit] has full health.
     *
     * @return bool hasFullHealth
     */
    bool IsFullHealth(Unit* unit)
    {
        return unit->IsFullHealth();
    }

    /**
     * Returns true if the [Unit] is in an accessible place for the given [Creature].
     *
     * @param [WorldObject] obj
     * @param float radius
     * @return bool isAccessible
     */
    bool IsInAccessiblePlaceFor(Unit* unit, Creature* creature)
    {
        return unit->isInAccessiblePlaceFor(creature);
    }

    /**
     * Returns true if the [Unit] an auctioneer.
     *
     * @return bool isAuctioneer
     */
    bool IsAuctioneer(Unit* unit)
    {
        return unit->IsAuctioner();
    }

    /**
     * Returns true if the [Unit] a guild master.
     *
     * @return bool isGuildMaster
     */
    bool IsGuildMaster(Unit* unit)
    {
        return unit->IsGuildMaster();
    }

    /**
     * Returns true if the [Unit] an innkeeper.
     *
     * @return bool isInnkeeper
     */
    bool IsInnkeeper(Unit* unit)
    {
        return unit->IsInnkeeper();
    }

    /**
     * Returns true if the [Unit] a trainer.
     *
     * @return bool isTrainer
     */
    bool IsTrainer(Unit* unit)
    {
        return unit->IsTrainer();
    }

    /**
     * Returns true if the [Unit] is able to show a gossip window.
     *
     * @return bool hasGossip
     */
    bool IsGossip(Unit* unit)
    {
        return unit->IsGossip();
    }

    /**
     * Returns true if the [Unit] is a taxi master.
     *
     * @return bool isTaxi
     */
    bool IsTaxi(Unit* unit)
    {
        return unit->IsTaxi();
    }

    /**
     * Returns true if the [Unit] is a spirit healer.
     *
     * @return bool isSpiritHealer
     */
    bool IsSpiritHealer(Unit* unit)
    {
        return unit->IsSpiritHealer();
    }

    /**
     * Returns true if the [Unit] is a spirit guide.
     *
     * @return bool isSpiritGuide
     */
    bool IsSpiritGuide(Unit* unit)
    {
        return unit->IsSpiritGuide();
    }

    /**
     * Returns true if the [Unit] is a tabard designer.
     *
     * @return bool isTabardDesigner
     */
    bool IsTabardDesigner(Unit* unit)
    {
        return unit->IsTabardDesigner();
    }

    /**
     * Returns true if the [Unit] provides services like vendor, training and auction.
     *
     * @return bool isTabardDesigner
     */
    bool IsServiceProvider(Unit* unit)
    {
        return unit->IsServiceProvider();
    }

    /**
     * Returns true if the [Unit] is a spirit guide or spirit healer.
     *
     * @return bool isSpiritService
     */
    bool IsSpiritService(Unit* unit)
    {
        return unit->IsSpiritService();
    }

    /**
     * Returns true if the [Unit] is alive.
     *
     * @return bool isAlive
     */
    bool IsAlive(Unit* unit)
    {
        return unit->IsAlive();
    }

    /**
     * Returns true if the [Unit] is dead.
     *
     * @return bool isDead
     */
    bool IsDead(Unit* unit)
    {
        return unit->isDead();
    }

    /**
     * Returns true if the [Unit] is dying.
     *
     * @return bool isDying
     */
    bool IsDying(Unit* unit)
    {
        return unit->isDying();
    }

    /**
     * Returns true if the [Unit] is a banker.
     *
     * @return bool isBanker
     */
    bool IsBanker(Unit* unit)
    {
        return unit->IsBanker();
    }

    /**
     * Returns true if the [Unit] is a vendor.
     *
     * @return bool isVendor
     */
    bool IsVendor(Unit* unit)
    {
        return unit->IsVendor();
    }

    /**
     * Returns true if the [Unit] is a battle master.
     *
     * @return bool isBattleMaster
     */
    bool IsBattleMaster(Unit* unit)
    {
        return unit->IsBattleMaster();
    }

    /**
     * Returns true if the [Unit] is a charmed.
     *
     * @return bool isCharmed
     */
    bool IsCharmed(Unit* unit)
    {
        return unit->IsCharmed();
    }

    /**
     * Returns true if the [Unit] is an armorer and can repair equipment.
     *
     * @return bool isArmorer
     */
    bool IsArmorer(Unit* unit)
    {
        return unit->IsArmorer();
    }

    /**
     * Returns true if the [Unit] is attacking a player.
     *
     * @return bool isAttackingPlayer
     */
    bool IsAttackingPlayer(Unit* unit)
    {
        return unit->isAttackingPlayer();
    }

    /**
     * Returns true if the [Unit] flagged for PvP.
     *
     * @return bool isPvP
     */
    bool IsPvPFlagged(Unit* unit)
    {
        return unit->IsPvP();
    }

    /**
     * Returns true if the [Unit] is on a [Vehicle].
     *
     * @return bool isOnVehicle
     */
    Vehicle* IsOnVehicle(Unit* unit)
    {
        return unit->GetVehicle();
    }

    /**
     * Returns true if the [Unit] is in combat.
     *
     * @return bool inCombat
     */
    bool IsInCombat(Unit* unit)
    {
        return unit->IsInCombat();
    }

    /**
     * Returns true if the [Unit] is under water.
     *
     * @return bool underWater
     */
    bool IsUnderWater(Unit* unit)
    {
        return unit->IsUnderWater();
    }

    /**
     * Returns true if the [Unit] is in water.
     *
     * @return bool inWater
     */
    bool IsInWater(Unit* unit)
    {
        return unit->IsInWater();
    }

    /**
     * Returns true if the [Unit] is not moving.
     *
     * @return bool notMoving
     */
    bool IsStopped(Unit* unit)
    {
        return unit->IsStopped();
    }

    /**
     * Returns true if the [Unit] is a quest giver.
     *
     * @return bool questGiver
     */
    bool IsQuestGiver(Unit* unit)
    {
        return unit->IsQuestGiver();
    }

    /**
     * Returns true if the [Unit]'s health is below the given percentage.
     *
     * @param int32 healthpct : percentage in integer from
     * @return bool isBelow
     */
    bool HealthBelowPct(Unit* unit, int32 pct)
    {
        return unit->HealthBelowPct(pct);
    }

    /**
     * Returns true if the [Unit]'s health is above the given percentage.
     *
     * @param int32 healthpct : percentage in integer from
     * @return bool isAbove
     */
    bool HealthAbovePct(Unit* unit, int32 pct)
    {
        return unit->HealthAbovePct(pct);
    }

    /**
     * Returns true if the [Unit] has an aura from the given spell entry.
     *
     * @param uint32 spell : entry of the aura spell
     * @return bool hasAura
     */
    bool HasAura(Unit* unit, uint32 spell)
    {
        return unit->HasAura(spell);
    }

    /**
     * Returns true if the [Unit] is casting a spell
     *
     * @return bool isCasting
     */
    bool IsCasting(Unit* unit)
    {
        return unit->HasUnitState(UNIT_STATE_CASTING);
    }

    /**
     * Returns true if the [Unit] has the given unit state.
     *
     * @param [UnitState] state : an unit state
     * @return bool hasState
     */
    bool HasUnitState(Unit* unit, uint32 state)
    {
        return unit->HasUnitState(state);
    }

    /**
     * Returns the [Unit]'s owner.
     *
     * @return [Unit] owner
     */
    Unit* GetOwner(Unit* unit)
    {
        return unit->GetOwner();
    }

    /**
     * Returns the [Unit]'s owner's GUID.
     *
     * @return ObjectGuid ownerGUID
     */
    ObjectGuid GetOwnerGUID(Unit* unit)
    {
        return unit->GetOwnerGUID();
    }

    /**
     * Returns the [Unit]'s mount's modelID.
     *
     * @return uint32 mountId : displayId of the mount
     */
    uint32 GetMountId(Unit* unit)
    {
        return unit->GetMountID();
    }

    /**
     * Returns the [Unit]'s creator's GUID.
     *
     * @return ObjectGuid creatorGUID
     */
    ObjectGuid GetCreatorGUID(Unit* unit)
    {
        return unit->GetCreatorGUID();
    }

    /**
     * Returns the [Unit]'s charmer's GUID.
     *
     * @return ObjectGuid charmerGUID
     */
    ObjectGuid GetCharmerGUID(Unit* unit)
    {
        return unit->GetCharmerGUID();
    }

    /**
     * Returns the GUID of the [Unit]'s charmed entity.
     *
     * @return ObjectGuid charmedGUID
     */
    ObjectGuid GetCharmGUID(Unit* unit)
    {
        return unit->GetCharmGUID();
    }

    /**
     * Returns the GUID of the [Unit]'s pet.
     *
     * @return ObjectGuid petGUID
     */
    ObjectGuid GetPetGUID(Unit* unit)
    {
        return unit->GetPetGUID();
    }

    /**
     * Returns the GUID of the [Unit]'s charmer or owner.
     *
     * @return ObjectGuid controllerGUID
     */
    ObjectGuid GetControllerGUID(Unit* unit)
    {
        return unit->GetCharmerOrOwnerGUID();
    }

    /**
     * Returns the GUID of the [Unit]'s charmer or owner or its own GUID.
     *
     * @return ObjectGuid controllerGUID
     */
    ObjectGuid GetControllerGUIDS(Unit* unit)
    {
        return unit->GetCharmerOrOwnerOrOwnGUID();
    }

    /**
     * Returns [Unit]'s specified stat
     *
     * @param uint32 statType
     * @return float stat
     */
    sol::optional<float> GetStat(Unit* unit, uint32 stat)
    {
        if (stat >= MAX_STATS)
            return sol::nullopt;

        return unit->GetStat((Stats)stat);
    }

    /**
     * Returns the [Unit]'s base spell power
     *
     * @param uint32 spellSchool
     * @return uint32 spellPower
     */
    sol::optional<uint32> GetBaseSpellPower(Unit* unit, uint32 spellschool)
    {
        if (spellschool >= MAX_SPELL_SCHOOL)
            return sol::nullopt;

        return unit->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + spellschool);
    }

    /**
     * Returns the [Unit]'s current victim target or nil.
     *
     * @return [Unit] victim
     */
    Unit* GetVictim(Unit* unit)
    {
        return unit->GetVictim();
    }

    /**
     * Returns the currently casted [Spell] of given type or nil.
     *
     * <pre>
     * enum CurrentSpellTypes
     * {
     *     CURRENT_MELEE_SPELL             = 0,
     *     CURRENT_GENERIC_SPELL           = 1,
     *     CURRENT_CHANNELED_SPELL         = 2,
     *     CURRENT_AUTOREPEAT_SPELL        = 3
     * };
     * </pre>
     *
     * @param [CurrentSpellTypes] spellType
     * @return [Spell] castedSpell
     */
    Spell* GetCurrentSpell(Unit* unit, uint32 type)
    {
        if (type >= CURRENT_MAX_SPELL)
            throw std::invalid_argument("valid CurrentSpellTypes expected");

        return unit->GetCurrentSpell(type);
    }

    /**
     * Returns the [Unit]'s current stand state.
     *
     * @return uint8 standState
     */
    uint8 GetStandState(Unit* unit)
    {
        return unit->getStandState();
    }

    /**
     * Returns the [Unit]'s current display ID.
     *
     * @return uint32 displayId
     */
    uint32 GetDisplayId(Unit* unit)
    {
        return unit->GetDisplayId();
    }

    /**
     * Returns the [Unit]'s native/original display ID.
     *
     * @return uint32 displayId
     */
    uint32 GetNativeDisplayId(Unit* unit)
    {
        return unit->GetNativeDisplayId();
    }

    /**
     * Returns the [Unit]'s level.
     *
     * @return uint8 level
     */
    uint8 GetLevel(Unit* unit)
    {
        return unit->GetLevel();
    }

    /**
     * Returns the [Unit]'s health amount.
     *
     * @return uint32 healthAmount
     */
    uint32 GetHealth(Unit* unit)
    {
        return unit->GetHealth();
    }

    Powers PowerSelectorHelper(Unit* unit, int powerType = -1)
    {
        if (powerType == -1)
            return unit->getPowerType();

        if (powerType < 0 || powerType >= int(MAX_POWERS))
            throw std::invalid_argument("valid Powers expected");

        return (Powers)powerType;
    }

    /**
     * Returns the [Unit]'s power amount for given power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param int type = -1 : a valid power type from [Powers] or -1 for the [Unit]'s current power type
     * @return uint32 powerAmount
     */
    uint32 GetPower(Unit* unit, sol::optional<int> typeArg)
    {
        int type = typeArg.value_or(-1);
        Powers power = PowerSelectorHelper(unit, type);

        return unit->GetPower(power);
    }

    /**
     * Returns the [Unit]'s max power amount for given power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param int type = -1 : a valid power type from [Powers] or -1 for the [Unit]'s current power type
     * @return uint32 maxPowerAmount
     */
    uint32 GetMaxPower(Unit* unit, sol::optional<int> typeArg)
    {
        int type = typeArg.value_or(-1);
        Powers power = PowerSelectorHelper(unit, type);

        return unit->GetMaxPower(power);
    }

    /**
     * Returns the [Unit]'s power percent for given power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param int type = -1 : a valid power type from [Powers] or -1 for the [Unit]'s current power type
     * @return float powerPct
     */
    float GetPowerPct(Unit* unit, sol::optional<int> typeArg)
    {
        int type = typeArg.value_or(-1);
        Powers power = PowerSelectorHelper(unit, type);

        float percent = ((float)unit->GetPower(power) / (float)unit->GetMaxPower(power)) * 100.0f;

        return percent;
    }

    /**
     * Returns the [Unit]'s current power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @return [Powers] powerType
     */
    Powers GetPowerType(Unit* unit)
    {
        return unit->getPowerType();
    }

    /**
     * Returns the [Unit]'s max health.
     *
     * @return uint32 maxHealth
     */
    uint32 GetMaxHealth(Unit* unit)
    {
        return unit->GetMaxHealth();
    }

    /**
     * Returns the [Unit]'s health percent.
     *
     * @return float healthPct
     */
    float GetHealthPct(Unit* unit)
    {
        return unit->GetHealthPct();
    }

    /**
     * Returns the [Unit]'s gender.
     *
     * @return uint8 gender : 0 for male, 1 for female and 2 for none
     */
    uint8 GetGender(Unit* unit)
    {
        return unit->getGender();
    }

    /**
     * Returns the [Unit]'s race ID.
     *
     * @return [Races] race
     */
    uint8 GetRace(Unit* unit)
    {
        return unit->getRace();
    }

    /**
     * Returns the [Unit]'s class ID.
     *
     * @return [Classes] class
     */
    uint8 GetClass(Unit* unit)
    {
        return unit->getClass();
    }

    /**
    * Returns the race mask
    *
    * @return uint32 racemask
    */
    uint32 GetRaceMask(Unit* unit)
    {
        return unit->getRaceMask();
    }

    /**
    * Returns the class mask
    *
    * @return uint32 classmask
    */
    uint32 GetClassMask(Unit* unit)
    {
        return unit->getClassMask();
    }

    /**
     * Returns the [Unit]'s creature type ID (enumerated in CreatureType.dbc).
     *
     * <pre>
     * enum CreatureType
     * {
     *     CREATURE_TYPE_BEAST            = 1,
     *     CREATURE_TYPE_DRAGONKIN        = 2,
     *     CREATURE_TYPE_DEMON            = 3,
     *     CREATURE_TYPE_ELEMENTAL        = 4,
     *     CREATURE_TYPE_GIANT            = 5,
     *     CREATURE_TYPE_UNDEAD           = 6,
     *     CREATURE_TYPE_HUMANOID         = 7,
     *     CREATURE_TYPE_CRITTER          = 8,
     *     CREATURE_TYPE_MECHANICAL       = 9,
     *     CREATURE_TYPE_NOT_SPECIFIED    = 10,
     *     CREATURE_TYPE_TOTEM            = 11,
     *     CREATURE_TYPE_NON_COMBAT_PET   = 12,     // This and below is TBC+
     *     CREATURE_TYPE_GAS_CLOUD        = 13
     * };
     * </pre>
     *
     * @return [CreatureType] creatureType
     */
    uint32 GetCreatureType(Unit* unit)
    {
        return unit->GetCreatureType();
    }

    /**
     * Returns the [Unit]'s class' name in given or default locale or nil.
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
     * @param [LocaleConstant] locale = DEFAULT_LOCALE
     * @return string className : class name or nil
     */
    sol::optional<std::string> GetClassAsString(Unit* unit, sol::optional<uint8> localeArg)
    {
        uint8 locale = localeArg.value_or(DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            throw std::invalid_argument("valid LocaleConstant expected");

        ChrClassesEntry const* entry = sChrClassesStore.LookupEntry(unit->getClass());
        if (!entry)
            return sol::nullopt;

        return std::string(entry->name[locale]);
    }

    /**
     * Returns the [Unit]'s race's name in given or default locale or nil.
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
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the race name in
     * @return string raceName : race name or nil
     */
    sol::optional<std::string> GetRaceAsString(Unit* unit, sol::optional<uint8> localeArg)
    {
        uint8 locale = localeArg.value_or(DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
            throw std::invalid_argument("valid LocaleConstant expected");

        ChrRacesEntry const* entry = sChrRacesStore.LookupEntry(unit->getRace());
        if (!entry)
            return sol::nullopt;

        return std::string(entry->name[locale]);
    }

    /**
     * Returns the [Unit]'s faction ID.
     *
     * @return uint32 faction
     */
    uint32 GetFaction(Unit* unit)
    {
        return unit->GetFaction();
    }

    /**
     * Returns the [Aura] of the given spell entry on the [Unit] or nil.
     *
     * @param uint32 spellID : entry of the aura spell
     * @return [Aura] aura : aura object or nil
     */
    Aura* GetAura(Unit* unit, uint32 spellID)
    {
        return unit->GetAura(spellID);
    }

    /**
     * Returns a table containing friendly [Unit]'s within given range of the [Unit].
     *
     * @param float range = 533.333 : search radius
     * @return table friendyUnits : table filled with friendly units
     */
    sol::table GetFriendlyUnitsInRange(Unit* unit, sol::optional<float> rangeArg, sol::this_state s)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);

        std::list<Unit*> list;

        Acore::AnyFriendlyUnitInObjectRangeCheck checker(unit, unit, range);
        Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck> searcher(unit, list, checker);
        Cell::VisitObjects(unit, searcher, range);

        ALEUtil::ObjectGUIDCheck guidCheck(unit->GetGUID());
        list.remove_if(guidCheck);

        sol::state_view lua(s);
        sol::table tbl = lua.create_table();
        uint32 i = 0;

        for (std::list<Unit*>::const_iterator it = list.begin(); it != list.end(); ++it)
            tbl[++i] = ALEBind::ToLuaDynamic(lua, *it);

        return tbl;
    }

    /**
     * Returns a table containing unfriendly [Unit]'s within given range of the [Unit].
     *
     * @param float range = 533.333 : search radius
     * @return table unfriendyUnits : table filled with unfriendly units
     */
    sol::table GetUnfriendlyUnitsInRange(Unit* unit, sol::optional<float> rangeArg, sol::this_state s)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);

        std::list<Unit*> list;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck checker(unit, unit, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(unit, list, checker);
        Cell::VisitObjects(unit, searcher, range);
        ALEUtil::ObjectGUIDCheck guidCheck(unit->GetGUID());
        list.remove_if(guidCheck);

        sol::state_view lua(s);
        sol::table tbl = lua.create_table();
        uint32 i = 0;

        for (std::list<Unit*>::const_iterator it = list.begin(); it != list.end(); ++it)
            tbl[++i] = ALEBind::ToLuaDynamic(lua, *it);

        return tbl;
    }

    /**
     * Returns [Unit]'s [Vehicle] methods
     *
     * @return [Vehicle] vehicle
     */
    Vehicle* GetVehicleKit(Unit* unit)
    {
        return unit->GetVehicleKit();
    }

    /**
     * Returns the Critter Guid
     *
     * @return ObjectGuid critterGuid
     */
    ObjectGuid GetCritterGUID(Unit* unit)
    {
        return unit->GetCritterGUID();
    }

    /**
     * Returns the [Unit]'s speed of given [UnitMoveType].
     *
     * <pre>
     * enum UnitMoveType
     * {
     *     MOVE_WALK           = 0,
     *     MOVE_RUN            = 1,
     *     MOVE_RUN_BACK       = 2,
     *     MOVE_SWIM           = 3,
     *     MOVE_SWIM_BACK      = 4,
     *     MOVE_TURN_RATE      = 5,
     *     MOVE_FLIGHT         = 6,
     *     MOVE_FLIGHT_BACK    = 7,
     *     MOVE_PITCH_RATE     = 8
     * };
     * </pre>
     *
     * @param [UnitMoveType] type
     * @return float speed
     */
    float GetSpeed(Unit* unit, uint32 type)
    {
        if (type >= MAX_MOVE_TYPE)
            throw std::invalid_argument("valid UnitMoveType expected");

        return unit->GetSpeed((UnitMoveType)type);
    }

    /**
    * Returns the [Unit]'s speed rate of given [UnitMoveType].
    *
    * <pre>
    * enum UnitMoveType
    * {
    *     MOVE_WALK           = 0,
    *     MOVE_RUN            = 1,
    *     MOVE_RUN_BACK       = 2,
    *     MOVE_SWIM           = 3,
    *     MOVE_SWIM_BACK      = 4,
    *     MOVE_TURN_RATE      = 5,
    *     MOVE_FLIGHT         = 6,
    *     MOVE_FLIGHT_BACK    = 7,
    *     MOVE_PITCH_RATE     = 8
    * };
    * </pre>
    *
    * @param [UnitMoveType] type
    * @return float speed
    */
    float GetSpeedRate(Unit* unit, uint32 type)
    {
        if (type >= MAX_MOVE_TYPE)
            throw std::invalid_argument("valid UnitMoveType expected");

        return unit->GetSpeedRate((UnitMoveType)type);
    }

    /**
     * Returns the current movement type for this [Unit].
     *
     * <pre>
     * enum MovementGeneratorType
     * {
     *     IDLE_MOTION_TYPE                = 0,
     *     RANDOM_MOTION_TYPE              = 1,
     *     WAYPOINT_MOTION_TYPE            = 2,
     *     MAX_DB_MOTION_TYPE              = 3,
     *     ANIMAL_RANDOM_MOTION_TYPE       = 3, // TC
     *
     *     CONFUSED_MOTION_TYPE            = 4,
     *     CHASE_MOTION_TYPE               = 5,
     *     HOME_MOTION_TYPE                = 6,
     *     FLIGHT_MOTION_TYPE              = 7,
     *     POINT_MOTION_TYPE               = 8,
     *     FLEEING_MOTION_TYPE             = 9,
     *     DISTRACT_MOTION_TYPE            = 10,
     *     ASSISTANCE_MOTION_TYPE          = 11,
     *     ASSISTANCE_DISTRACT_MOTION_TYPE = 12,
     *     TIMED_FLEEING_MOTION_TYPE       = 13,
     *     FOLLOW_MOTION_TYPE              = 14,
     *     EFFECT_MOTION_TYPE              = 15, // mangos
     *     ROTATE_MOTION_TYPE              = 15, // TC
     *     EFFECT_MOTION_TYPE              = 16, // TC
     *     NULL_MOTION_TYPE                = 17, // TC
     * };
     * </pre>
     *
     * @return [MovementGeneratorType] movementType
     */
    MovementGeneratorType GetMovementType(Unit* unit)
    {
        return unit->GetMotionMaster()->GetCurrentMovementGeneratorType();
    }

    /**
     * Returns the [Unit]'s attackers.
     *
     * @return table attackers : table of [Unit]s attacking the unit
     */
    sol::table GetAttackers(Unit* unit, sol::this_state s)
    {
        Unit::AttackerSet const& attackers = unit->getAttackers();

        sol::state_view lua(s);
        sol::table tbl = lua.create_table();
        uint32 i = 0;

        for (Unit* attacker : attackers)
        {
            if (!attacker)
                continue;

            tbl[++i] = ALEBind::ToLuaDynamic(lua, attacker);
        }

        return tbl;
    }

    /**
     * Sets the [Unit]'s owner GUID to given GUID.
     *
     * @param ObjectGuid guid : new owner guid
     */
    void SetOwnerGUID(Unit* unit, ObjectGuid guid)
    {
        unit->SetOwnerGUID(guid);
    }

    /**
     * Sets the [Unit]'s PvP on or off.
     *
     * @param bool apply = true : true if set on, false if off
     */
    void SetPvP(Unit* unit, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);

        unit->SetPvP(apply);
    }

    /**
     * Sets the [Unit]'s sheath state.
     *
     *     enum SheathState
     *     {
     *         SHEATH_STATE_UNARMED  = 0, // non prepared weapon
     *         SHEATH_STATE_MELEE    = 1, // prepared melee weapon
     *         SHEATH_STATE_RANGED   = 2  // prepared ranged weapon
     *     };
     *
     * @param [SheathState] sheathState : valid SheathState
     */
    void SetSheath(Unit* unit, uint32 sheathed)
    {
        if (sheathed >= MAX_SHEATH_STATE)
            throw std::invalid_argument("valid SheathState expected");

        unit->SetSheath((SheathState)sheathed);
    }

    /**
     * Sets the [Unit]'s name internally.
     *
     * @param string name : new name
     */
    void SetName(Unit* unit, std::string const& name)
    {
        if (name.length() > 0)
            unit->SetName(name);
    }

    /**
     * Sets the [Unit]'s speed of given [UnitMoveType] to given speed.
     * If forced, packets sent to clients forcing the visual change.
     *
     * <pre>
     * enum UnitMoveType
     * {
     *     MOVE_WALK           = 0,
     *     MOVE_RUN            = 1,
     *     MOVE_RUN_BACK       = 2,
     *     MOVE_SWIM           = 3,
     *     MOVE_SWIM_BACK      = 4,
     *     MOVE_TURN_RATE      = 5,
     *     MOVE_FLIGHT         = 6,
     *     MOVE_FLIGHT_BACK    = 7,
     *     MOVE_PITCH_RATE     = 8
     * };
     * </pre>
     *
     * @param [UnitMoveType] type
     * @param float rate
     * @param bool forced = false
     */
    void SetSpeed(Unit* unit, uint32 type, float rate, sol::optional<bool> forcedArg)
    {
        bool forced = forcedArg.value_or(false);
        if (type >= MAX_MOVE_TYPE)
            throw std::invalid_argument("valid UnitMoveType expected");

        unit->SetSpeed((UnitMoveType)type, rate, forced);
    }

    /**
     * Sets the [Unit]'s speed rate of given [UnitMoveType] to given rate.
     * If forced, packets sent to clients forcing the visual change.
     *
     * <pre>
     * enum UnitMoveType
     * {
     *     MOVE_WALK           = 0,
     *     MOVE_RUN            = 1,
     *     MOVE_RUN_BACK       = 2,
     *     MOVE_SWIM           = 3,
     *     MOVE_SWIM_BACK      = 4,
     *     MOVE_TURN_RATE      = 5,
     *     MOVE_FLIGHT         = 6,
     *     MOVE_FLIGHT_BACK    = 7,
     *     MOVE_PITCH_RATE     = 8
     * };
     * </pre>
     *
     * @param [UnitMoveType] type
     * @param float rate
     * @param bool forced = false
     */
    void SetSpeedRate(Unit* unit, uint32 type, float rate)
    {
        if (type >= MAX_MOVE_TYPE)
            throw std::invalid_argument("valid UnitMoveType expected");

        unit->SetSpeedRate((UnitMoveType)type, rate);
    }

    /**
     * Sets the [Unit]'s faction.
     *
     * @param uint32 faction : new faction ID
     */
    void SetFaction(Unit* unit, uint32 factionId)
    {
        unit->SetFaction(factionId);
    }

    /**
     * Sets the [Unit]'s level.
     *
     * @param uint8 level : new level
     */
    void SetLevel(Unit* unit, uint8 newlevel)
    {
        if (newlevel < 1)
            throw std::invalid_argument("level cannot be below 1");

        if (Player* player = unit->ToPlayer())
        {
            player->GiveLevel(newlevel);
            player->InitTalentForLevel();
            player->SetUInt32Value(PLAYER_XP, 0);
        }
        else
            unit->SetLevel(newlevel);
    }

    /**
     * Sets the [Unit]'s health.
     *
     * @param uint32 health : new health
     */
    void SetHealth(Unit* unit, uint32 amt)
    {
        unit->SetHealth(amt);
    }

    /**
     * Sets the [Unit]'s max health.
     *
     * @param uint32 maxHealth : new max health
     */
    void SetMaxHealth(Unit* unit, uint32 amt)
    {
        unit->SetMaxHealth(amt);
    }

    /**
     * Sets the [Unit]'s power amount for the given power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param uint32 amount : new power amount
     * @param int type = -1 : a valid power type from [Powers] or -1 for the [Unit]'s current power type
     */
    void SetPower(Unit* unit, uint32 amt, sol::optional<int> typeArg)
    {
        int type = typeArg.value_or(-1);
        Powers power = PowerSelectorHelper(unit, type);

        unit->SetPower(power, amt);
    }

    /**
     * modifies the [Unit]'s power amount for the given power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param int32 amount : amount to modify
     * @param int type = -1 : a valid power type from [Powers] or -1 for the [Unit]'s current power type
     */
    void ModifyPower(Unit* unit, int32 amt, sol::optional<int> typeArg)
    {
        int type = typeArg.value_or(-1);
        Powers power = PowerSelectorHelper(unit, type);

        unit->ModifyPower(power, amt);
    }

    /**
     * Sets the [Unit]'s max power amount for the given power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param int type = -1 : a valid power type from [Powers] or -1 for the [Unit]'s current power type
     * @param uint32 maxPower : new max power amount
     */
    void SetMaxPower(Unit* unit, sol::optional<int> typeArg, uint32 amt)
    {
        int type = typeArg.value_or(-1);
        Powers power = PowerSelectorHelper(unit, type);

        unit->SetMaxPower(power, amt);
    }

    /**
     * Sets the [Unit]'s power type.
     *
     *     enum Powers
     *     {
     *         POWER_MANA        = 0,
     *         POWER_RAGE        = 1,
     *         POWER_FOCUS       = 2,
     *         POWER_ENERGY      = 3,
     *         POWER_HAPPINESS   = 4,
     *         POWER_RUNE        = 5,
     *         POWER_RUNIC_POWER = 6,
     *         MAX_POWERS        = 7,
     *         POWER_ALL         = 127,         // default for class?
     *         POWER_HEALTH      = 0xFFFFFFFE   // (-2 as signed value)
     *     };
     *
     * @param [Powers] type : a valid power type
     */
    void SetPowerType(Unit* unit, uint32 type)
    {
        if (type >= int(MAX_POWERS))
            throw std::invalid_argument("valid Powers expected");

        unit->setPowerType((Powers)type);
    }

    /**
     * Sets the [Unit]'s modelID.
     *
     * @param uint32 displayId
     */
    void SetDisplayId(Unit* unit, uint32 model)
    {
        unit->SetDisplayId(model);
    }

    /**
     * Sets the [Unit]'s native/default modelID.
     *
     * @param uint32 displayId
     */
    void SetNativeDisplayId(Unit* unit, uint32 model)
    {
        unit->SetNativeDisplayId(model);
    }

    /**
     * Sets the [Unit]'s facing/orientation.
     *
     * @param uint32 orientation
     */
    void SetFacing(Unit* unit, float o)
    {
        unit->SetFacingTo(o);
    }

    /**
     * Sets the [Unit] to face the given [WorldObject]'s direction.
     *
     * @param [WorldObject] target
     */
    void SetFacingToObject(Unit* unit, WorldObject* obj)
    {
        unit->SetFacingToObject(obj);
    }

    /**
     * Sets creator GUID
     *
     * @param ObjectGuid guid
     */
    void SetCreatorGUID(Unit* unit, ObjectGuid guid)
    {
        unit->SetCreatorGUID(guid);
    }

    /**
     * Sets pet GUID
     *
     * @param ObjectGuid guid
     */
    void SetPetGUID(Unit* unit, ObjectGuid guid)
    {
        unit->SetPetGUID(guid);
    }

    /**
     * Toggles (Sets) [Unit]'s water walking
     *
     * @param bool enable = true
     */
    void SetWaterWalk(Unit* unit, sol::optional<bool> enableArg)
    {
        bool enable = enableArg.value_or(true);
        unit->SetWaterWalking(enable);
    }

    /**
     * Sets the [Unit]'s stand state
     *
     * @param uint8 state : stand state
     */
    void SetStandState(Unit* unit, uint8 state)
    {
        unit->SetStandState(state);
    }

    /**
     * Sets the [Unit] in combat with the `enemy` [Unit].
     *
     * @param [Unit] enemy : the [Unit] to start combat with
     */
    void SetInCombatWith(Unit* unit, Unit* enemy)
    {
        unit->SetInCombatWith(enemy);
    }

    /**
     * Sets the [Unit]'s FFA flag on or off.
     *
     * @param bool apply = true
     */
    void SetFFA(Unit* unit, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);

        if (apply)
        {
            unit->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
            for (Unit::ControlSet::iterator itr = unit->m_Controlled.begin(); itr != unit->m_Controlled.end(); ++itr)
                (*itr)->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        }
        else
        {
            unit->RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
            for (Unit::ControlSet::iterator itr = unit->m_Controlled.begin(); itr != unit->m_Controlled.end(); ++itr)
                (*itr)->RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_FFA_PVP);
        }
    }

    /**
     * Sets the [Unit]'s sanctuary flag on or off.
     *
     * @param bool apply = true
     */
    void SetSanctuary(Unit* unit, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);

        if (apply)
        {
            unit->SetByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY);
            unit->CombatStop();
            unit->CombatStopWithPets();
        }
        else
            unit->RemoveByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY);
    }

    /**
     * Sets the [Unit]'s critter companion by GUID.
     *
     * This method assigns the specified [ObjectGuid] as the critter (non-combat pet) companion of the [Unit].
     *
     * @param [ObjectGuid] guid : The GUID of the critter to set
     */
    void SetCritterGUID(Unit* unit, ObjectGuid guid)
    {
        unit->SetCritterGUID(guid);
    }

    /**
     * Roots the [Unit] to the ground, if 'false' specified, unroots the [Unit].
     *
     * @param bool apply = true
     */
    void SetRooted(Unit* unit, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);
        unit->SetControlled(apply, UNIT_STATE_ROOT);
    }

    /**
     * Confuses the [Unit], if 'false' specified, the [Unit] is no longer confused.
     *
     * @param bool apply = true
     */
    void SetConfused(Unit* unit, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);
        unit->SetControlled(apply, UNIT_STATE_CONFUSED);
    }

    /**
     * Fears the [Unit], if 'false' specified, the [Unit] is no longer feared.
     *
     * @param bool apply = true
     */
    void SetFeared(Unit* unit, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);
        unit->SetControlled(apply, UNIT_STATE_FLEEING);
    }

    /**
     * Clears the [Unit]'s threat list.
     */
    void ClearThreatList(Unit* unit)
    {
        unit->GetThreatMgr().ClearAllThreat();
    }

    /**
     * Returns the [Unit]'s threat list.
     *
     * @return table threatList : table of [Unit]s in the threat list
     */
    sol::object GetThreatList(Unit* unit, sol::this_state s)
    {
        sol::state_view lua(s);

        if (!unit->CanHaveThreatList())
            return sol::make_object(lua, sol::lua_nil);

        sol::table tbl = lua.create_table();
        uint32 i = 0;

        for (ThreatReference const* item : unit->GetThreatMgr().GetSortedThreatList())
        {
            Unit* victim = item->GetVictim();
            if (!victim)
                continue;

            tbl[++i] = ALEBind::ToLuaDynamic(lua, victim);
        }

        return sol::make_object(lua, tbl);
    }

    /**
     * Mounts the [Unit] on the given displayID/modelID.
     *
     * @param uint32 displayId
     */
    void Mount(Unit* unit, uint32 displayId)
    {
        unit->Mount(displayId);
    }

    /**
     * Dismounts the [Unit].
     */
    void Dismount(Unit* unit)
    {
        if (unit->IsMounted())
        {
            unit->Dismount();
            unit->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }

    /**
     * Makes the [Unit] perform the given emote.
     *
     * @param uint32 emoteId
     */
    void PerformEmote(Unit* unit, uint32 emoteId)
    {
        unit->HandleEmoteCommand(emoteId);
    }

    /**
     * Makes the [Unit] perform the given emote continuously.
     *
     * @param uint32 emoteId
     */
    void EmoteState(Unit* unit, uint32 emoteId)
    {
        unit->SetUInt32Value(UNIT_NPC_EMOTESTATE, emoteId);
    }

    /**
     * Returns calculated percentage from Health
     *
     * @return int32 percentage
     */
    uint32 CountPctFromCurHealth(Unit* unit, int32 pct)
    {
        return unit->CountPctFromCurHealth(pct);
    }

    /**
     * Returns calculated percentage from Max Health
     *
     * @return int32 percentage
     */
    uint32 CountPctFromMaxHealth(Unit* unit, int32 pct)
    {
        return unit->CountPctFromMaxHealth(pct);
    }

    /**
     * Sends chat message to [Player]
     *
     * @param uint8 type : chat, whisper, etc
     * @param uint32 lang : language to speak
     * @param string msg
     * @param [Player] target
     */
    void SendChatMessageToPlayer(Unit* unit, uint8 type, uint32 lang, std::string const& msg, Player* target)
    {
        if (type >= MAX_CHAT_MSG_TYPE)
            throw std::invalid_argument("valid ChatMsg expected");
        if (lang >= LANGUAGES_COUNT)
            throw std::invalid_argument("valid Language expected");

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, ChatMsg(type), Language(lang), unit, target, msg);
        target->GetSession()->SendPacket(&data);
    }

    /**
     * Stops the [Unit]'s movement
     */
    void MoveStop(Unit* unit)
    {
        unit->StopMoving();
    }

    /**
     * The [Unit]'s movement expires and clears movement
     *
     * @param bool reset = true : cleans movement
     */
    void MoveExpire(Unit* unit, sol::optional<bool> resetArg)
    {
        bool reset = resetArg.value_or(true);
        unit->GetMotionMaster()->MovementExpired(reset);
    }

    /**
     * Clears the [Unit]'s movement
     *
     * @param bool reset = true : clean movement
     */
    void MoveClear(Unit* unit, sol::optional<bool> resetArg)
    {
        bool reset = resetArg.value_or(true);
        unit->GetMotionMaster()->Clear(reset);
    }

    /**
     * The [Unit] will be idle
     */
    void MoveIdle(Unit* unit)
    {
        unit->GetMotionMaster()->MoveIdle();
    }

    /**
     * The [Unit] will move at random
     *
     * @param float radius : limit on how far the [Unit] will move at random
     */
    void MoveRandom(Unit* unit, float radius)
    {
        float x, y, z;
        unit->GetPosition(x, y, z);
        unit->GetMotionMaster()->MoveRandom(radius);
    }

    /**
     * The [Unit] will move to its set home location
     */
    void MoveHome(Unit* unit)
    {
        unit->GetMotionMaster()->MoveTargetedHome();
    }

    /**
     * The [Unit] will follow the target
     *
     * @param [Unit] target : target to follow
     * @param float dist = 0 : distance to start following
     * @param float angle = 0
     */
    void MoveFollow(Unit* unit, Unit* target, sol::optional<float> distArg, sol::optional<float> angleArg)
    {
        float dist = distArg.value_or(0.0f);
        float angle = angleArg.value_or(0.0f);
        unit->GetMotionMaster()->MoveFollow(target, dist, angle);
    }

    /**
     * The [Unit] will chase the target
     *
     * @param [Unit] target : target to chase
     * @param float dist = 0 : distance start chasing
     * @param float angle = 0
     */
    void MoveChase(Unit* unit, Unit* target, sol::optional<float> distArg, sol::optional<float> angleArg)
    {
        float dist = distArg.value_or(0.0f);
        float angle = angleArg.value_or(0.0f);
        unit->GetMotionMaster()->MoveChase(target, dist, angle);
    }

    /**
     * The [Unit] will move confused
     */
    void MoveConfused(Unit* unit)
    {
        unit->GetMotionMaster()->MoveConfused();
    }

    /**
     * The [Unit] will flee
     *
     * @param [Unit] target
     * @param uint32 time = 0 : flee delay
     */
    void MoveFleeing(Unit* unit, Unit* target, sol::optional<uint32> timeArg)
    {
        uint32 time = timeArg.value_or(0);
        unit->GetMotionMaster()->MoveFleeing(target, time);
    }

    /**
     * The [Unit] will move to the coordinates
     *
     * @param uint32 id : unique waypoint Id
     * @param float x
     * @param float y
     * @param float z
     * @param bool genPath = true : if true, generates path
     */
    void MoveTo(Unit* unit, uint32 id, float x, float y, float z, sol::optional<bool> genPathArg)
    {
        bool genPath = genPathArg.value_or(true);
        unit->GetMotionMaster()->MovePoint(id, x, y, z, FORCED_MOVEMENT_NONE, 0.f, 0.f, genPath);
    }

    /**
     * Makes the [Unit] jump to the coordinates
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float zSpeed : start velocity
     * @param float maxHeight : maximum height
     * @param uint32 id = 0 : unique movement Id
     */
    void MoveJump(Unit* unit, float x, float y, float z, float zSpeed, float maxHeight, sol::optional<uint32> idArg)
    {
        uint32 id = idArg.value_or(0);
        Position pos(x, y, z);
        unit->GetMotionMaster()->MoveJump(pos, zSpeed, maxHeight, id);
    }

    /**
     * The [Unit] will whisper the message to a [Player]
     *
     * @param string msg : message for the [Unit] to emote
     * @param uint32 lang : language for the [Unit] to speak
     * @param [Player] receiver : specific [Unit] to receive the message
     * @param bool bossWhisper = false : is a boss whisper
     */
    void SendUnitWhisper(Unit* unit, std::string const& msg, uint32 lang, Player* receiver, sol::optional<bool> bossWhisperArg)
    {
        bool bossWhisper = bossWhisperArg.value_or(false);
        if (msg.length() > 0)
            unit->Whisper(msg, (Language)lang, receiver, bossWhisper);
    }

    /**
     * The [Unit] will emote the message
     *
     * @param string msg : message for the [Unit] to emote
     * @param [Unit] receiver = nil : specific [Unit] to receive the message
     * @param bool bossEmote = false : is a boss emote
     */
    void SendUnitEmote(Unit* unit, std::string const& msg, sol::optional<UnitRef> receiverArg, sol::optional<bool> bossEmoteArg)
    {
        Unit* receiver = receiverArg ? receiverArg->Resolve() : nullptr;
        bool bossEmote = bossEmoteArg.value_or(false);
        if (msg.length() > 0)
            unit->TextEmote(msg, receiver, bossEmote);
    }

    /**
     * The [Unit] will say the message
     *
     * @param string msg : message for the [Unit] to say
     * @param uint32 language : language for the [Unit] to speak
     */
    void SendUnitSay(Unit* unit, std::string const& msg, uint32 language)
    {
        if (msg.length() > 0)
            unit->Say(msg, (Language)language, unit);
    }

    /**
     * The [Unit] will yell the message
     *
     * @param string msg : message for the [Unit] to yell
     * @param uint32 language : language for the [Unit] to speak
     */
    void SendUnitYell(Unit* unit, std::string const& msg, uint32 language)
    {
        if (msg.length() > 0)
            unit->Yell(msg, (Language)language, unit);
    }

    /**
     * Unmorphs the [Unit] setting it's display ID back to the native display ID.
     */
    void DeMorph(Unit* unit)
    {
        unit->DeMorph();
    }

    /**
     * Makes the [Unit] cast the spell on the target.
     *
     * @param [Unit] target = nil : can be self or another unit
     * @param uint32 spell : entry of a spell
     * @param bool triggered = false : if true the spell is instant and has no cost
     */
    void CastSpell(Unit* unit, sol::optional<UnitRef> targetArg, uint32 spell, sol::optional<bool> triggeredArg)
    {
        Unit* target = targetArg ? targetArg->Resolve() : nullptr;
        bool triggered = triggeredArg.value_or(false);
        SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(spell);
        if (!spellEntry)
            return;

        unit->CastSpell(target, spell, triggered);
    }

    /**
     * Casts the [Spell] at target [Unit] with custom basepoints or casters.
     * See also [Unit:CastSpell].
     *
     * @param [Unit] target = nil
     * @param uint32 spell
     * @param bool triggered = false
     * @param int32 bp0 = nil : custom basepoints for [Spell] effect 1. If nil, no change is made
     * @param int32 bp1 = nil : custom basepoints for [Spell] effect 2. If nil, no change is made
     * @param int32 bp2 = nil : custom basepoints for [Spell] effect 3. If nil, no change is made
     * @param [Item] castItem = nil
     * @param ObjectGuid originalCaster = ObjectGuid()
     */
    void CastCustomSpell(Unit* unit, sol::optional<UnitRef> targetArg, uint32 spell, sol::optional<bool> triggeredArg, sol::optional<int32> bp0Arg, sol::optional<int32> bp1Arg, sol::optional<int32> bp2Arg, sol::optional<ItemRef> castItemArg, sol::optional<ObjectGuid> originalCasterArg)
    {
        Unit* target = targetArg ? targetArg->Resolve() : nullptr;
        bool triggered = triggeredArg.value_or(false);
        bool has_bp0 = bp0Arg.has_value();
        int32 bp0 = bp0Arg.value_or(0);
        bool has_bp1 = bp1Arg.has_value();
        int32 bp1 = bp1Arg.value_or(0);
        bool has_bp2 = bp2Arg.has_value();
        int32 bp2 = bp2Arg.value_or(0);
        Item* castItem = castItemArg ? castItemArg->Resolve() : nullptr;
        ObjectGuid originalCaster = originalCasterArg.value_or(ObjectGuid());

        unit->CastCustomSpell(target, spell, has_bp0 ? &bp0 : nullptr, has_bp1 ? &bp1 : nullptr, has_bp2 ? &bp2 : nullptr, triggered, castItem, nullptr, ObjectGuid(originalCaster));
    }

    /**
     * Makes the [Unit] cast the spell to the given coordinates, used for area effect spells.
     *
     * @param float x
     * @param float y
     * @param float z
     * @param uint32 spell : entry of a spell
     * @param bool triggered = false : if true the spell is instant and has no cost
     */
    void CastSpellAoF(Unit* unit, float _x, float _y, float _z, uint32 spell, sol::optional<bool> triggeredArg)
    {
        bool triggered = triggeredArg.value_or(true);
        unit->CastSpell(_x, _y, _z, spell, triggered);
    }

    /**
     * Clears the [Unit]'s combat
     */
    void ClearInCombat(Unit* unit)
    {
        unit->ClearInCombat();
    }

    /**
     * Stops the [Unit]'s current spell cast
     *
     * @param uint32 spell = 0 : entry of a spell
     */
    void StopSpellCast(Unit* unit, sol::optional<uint32> spellIdArg)
    {
        uint32 spellId = spellIdArg.value_or(0);
        unit->CastStop(spellId);
    }

    /**
     * Interrupts [Unit]'s spell state, casting, etc.
     *
     * if spell is not interruptible, it will return
     *
     * @param int32 spellType : type of spell to interrupt
     * @param bool delayed = true : skips if the spell is delayed
     */
    void InterruptSpell(Unit* unit, int spellType, sol::optional<bool> delayedArg)
    {
        bool delayed = delayedArg.value_or(true);
        switch (spellType)
        {
        case 0:
            spellType = CURRENT_MELEE_SPELL;
            break;
        case 1:
            spellType = CURRENT_GENERIC_SPELL;
            break;
        case 2:
            spellType = CURRENT_CHANNELED_SPELL;
            break;
        case 3:
            spellType = CURRENT_AUTOREPEAT_SPELL;
            break;
        default:
            throw std::invalid_argument("valid CurrentSpellTypes expected");
        }

        unit->InterruptSpell((CurrentSpellTypes)spellType, delayed);
    }

    /**
     * Adds the [Aura] of the given spell entry on the given target from the [Unit].
     *
     * @param uint32 spell : entry of a spell
     * @param [Unit] target : aura will be applied on the target
     * @return [Aura] aura
     */
    Aura* AddAura(Unit* unit, uint32 spell, Unit* target)
    {
        SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(spell);
        if (!spellEntry)
            return nullptr;

        return unit->AddAura(spell, target);
    }

    /**
     * Removes [Aura] of the given spell entry from the [Unit].
     *
     * @param uint32 spell : entry of a spell
     */
    void RemoveAura(Unit* unit, uint32 spellId)
    {
        unit->RemoveAurasDueToSpell(spellId);
    }

    /**
     * Removes all [Aura]'s from the [Unit].
     *
     *     Note: talents and racials are also auras, use with caution
     */
    void RemoveAllAuras(Unit* unit)
    {
        unit->RemoveAllAuras();
    }

    /**
     * Removes all positive visible [Aura]'s from the [Unit].
     */
    void RemoveArenaAuras(Unit* unit)
    {
        unit->RemoveArenaAuras();
    }

    /**
     * Adds the given unit state for the [Unit].
     *
     * @param [UnitState] state
     */
    void AddUnitState(Unit* unit, uint32 state)
    {
        unit->AddUnitState(state);
    }

    /**
     * Removes the given unit state from the [Unit].
     *
     * @param [UnitState] state
     */
    void ClearUnitState(Unit* unit, uint32 state)
    {
        unit->ClearUnitState(state);
    }

    /**
     * Makes the [Unit] teleport to given coordinates within same map.
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float o : orientation
     */
    void NearTeleport(Unit* unit, float x, float y, float z, float o)
    {
        unit->NearTeleportTo(x, y, z, o);
    }

    /**
     * Makes the [Unit] damage the target [Unit]
     *
     * <pre>
     * enum SpellSchools
     * {
     *     SPELL_SCHOOL_NORMAL  = 0,
     *     SPELL_SCHOOL_HOLY    = 1,
     *     SPELL_SCHOOL_FIRE    = 2,
     *     SPELL_SCHOOL_NATURE  = 3,
     *     SPELL_SCHOOL_FROST   = 4,
     *     SPELL_SCHOOL_SHADOW  = 5,
     *     SPELL_SCHOOL_ARCANE  = 6,
     *     MAX_SPELL_SCHOOL     = 7
     * };
     * </pre>
     *
     * @param [Unit] target : [Unit] to damage
     * @param uint32 damage : amount to damage
     * @param bool durabilityloss = true : if false, the damage does not do durability damage
     * @param [SpellSchools] school = MAX_SPELL_SCHOOL : school the damage is done in or MAX_SPELL_SCHOOL for direct damage
     * @param uint32 spell = 0 : spell that inflicts the damage
     */
    void DealDamage(Unit* unit, Unit* target, uint32 damage, sol::optional<bool> durabilitylossArg, sol::optional<uint32> schoolArg, sol::optional<uint32> spellArg)
    {
        bool durabilityloss = durabilitylossArg.value_or(true);
        uint32 school = schoolArg.value_or(MAX_SPELL_SCHOOL);
        uint32 spell = spellArg.value_or(0);
        if (school > MAX_SPELL_SCHOOL)
            throw std::invalid_argument("valid SpellSchool expected");

        // flat melee damage without resistence/etc reduction
        if (school == MAX_SPELL_SCHOOL)
        {
            Unit::DealDamage(unit, target, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, durabilityloss);
            unit->SendAttackStateUpdate(HITINFO_AFFECTS_VICTIM, target, 1, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_HIT, 0);
            return;
        }

        SpellSchoolMask schoolmask = SpellSchoolMask(1 << school);

        if (Unit::IsDamageReducedByArmor(schoolmask))
            damage = Unit::CalcArmorReducedDamage(unit, target, damage, nullptr, BASE_ATTACK);

        if (!spell)
        {
            DamageInfo dmgInfo(unit, target, damage, nullptr, schoolmask, SPELL_DIRECT_DAMAGE);
            unit->CalcAbsorbResist(dmgInfo);

            if (!dmgInfo.GetDamage())
                damage = 0;
            else
                damage = dmgInfo.GetDamage();

            uint32 absorb = dmgInfo.GetAbsorb();
            uint32 resist = dmgInfo.GetResist();
            unit->DealDamageMods(target, damage, &absorb);
            Unit::DealDamage(unit, target, damage, nullptr, DIRECT_DAMAGE, schoolmask, nullptr, false);
            unit->SendAttackStateUpdate(HITINFO_AFFECTS_VICTIM, target, 0, schoolmask, damage, absorb, resist, VICTIMSTATE_HIT, 0);
            return;
        }

        if (!spell)
            return;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if (!spellInfo)
            return;

        SpellNonMeleeDamage dmgInfo(unit, target, spellInfo, spellInfo->GetSchoolMask());
        Unit::DealDamageMods(dmgInfo.target, dmgInfo.damage, &dmgInfo.absorb);
        unit->SendSpellNonMeleeDamageLog(&dmgInfo);
        unit->DealSpellDamage(&dmgInfo, true);
    }

    /**
     * Makes the [Unit] heal the target [Unit] with given spell
     *
     * @param [Unit] target : [Unit] to heal
     * @param uint32 spell : spell that causes the healing
     * @param uint32 amount : amount to heal
     * @param bool critical = false : if true, heal is logged as critical
     */
    void DealHeal(Unit* unit, Unit* target, uint32 spell, uint32 amount, sol::optional<bool> criticalArg)
    {
        bool critical = criticalArg.value_or(false);

        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spell))
        {
            HealInfo healInfo(unit, target, amount, info, info->GetSchoolMask());
            unit->HealBySpell(healInfo, critical);
        }
    }

    /**
     * Makes the [Unit] kill the target [Unit]
     *
     * @param [Unit] target : [Unit] to kill
     * @param bool durLoss = true : when true, the target's items suffer durability loss
     */
    void Kill(Unit* unit, Unit* target, sol::optional<bool> durLossArg)
    {
        bool durLoss = durLossArg.value_or(true);

        Unit::Kill(unit, target, durLoss);
    }

    /**
     * Adds threat to the [Unit] from the victim.
     *
     * <pre>
     * enum SpellSchoolMask
     * {
     *     SPELL_SCHOOL_MASK_NONE    = 0,
     *     SPELL_SCHOOL_MASK_NORMAL  = 1,
     *     SPELL_SCHOOL_MASK_HOLY    = 2,
     *     SPELL_SCHOOL_MASK_FIRE    = 4,
     *     SPELL_SCHOOL_MASK_NATURE  = 8,
     *     SPELL_SCHOOL_MASK_FROST   = 16,
     *     SPELL_SCHOOL_MASK_SHADOW  = 32,
     *     SPELL_SCHOOL_MASK_ARCANE  = 64,
     * }
     * </pre>
     *
     * @param [Unit] victim : [Unit] that caused the threat
     * @param float threat : threat amount
     * @param [SpellSchoolMask] schoolMask = 0 : [SpellSchoolMask] of the threat causer
     * @param uint32 spell = 0 : spell entry used for threat
     */
    void AddThreat(Unit* unit, Unit* victim, sol::optional<float> threatArg, sol::optional<uint32> spellArg, sol::optional<uint32> schoolMaskArg)
    {
        float threat = threatArg.value_or(1.0f);
        uint32 spell = spellArg.value_or(0);

        uint32 schoolMask = schoolMaskArg.value_or(0);
        if (schoolMask > SPELL_SCHOOL_MASK_ALL)
            throw std::invalid_argument("valid SpellSchoolMask expected");

        unit->AddThreat(victim, threat, (SpellSchoolMask)schoolMask, spell ? sSpellMgr->GetSpellInfo(spell) : nullptr);
    }

    /**
     * Modifies threat in pct to the [Unit] from the victim
     *
     * @param [Unit] victim : [Unit] that caused the threat
     * @param int32 percent : threat amount in pct
     */
    void ModifyThreatPct(Unit* unit, Unit* victim, sol::optional<int32> threatPctArg)
    {
        int32 threatPct = threatPctArg.value_or(1);

        unit->GetThreatMgr().ModifyThreatByPercent(victim, threatPct);
    }

    /**
     * Clear the threat of a [Unit] in the threat list.
     *
     * @param [Unit] target
     */
    void ClearThreat(Unit* unit, Unit* target)
    {
        unit->GetThreatMgr().ClearThreat(target);
    }

    /**
     * Resets the [Unit]'s threat list, setting all threat targets' threat to 0.
     */
    void ResetAllThreat(Unit* unit)
    {
        unit->GetThreatMgr().ResetAllThreat();
    }

    /**
     * Returns the threat of a [Unit].
     *
     * @param [Unit] target
     * @return float threat
     */
    float GetThreat(Unit* unit, Unit* target)
    {
        return unit->GetThreatMgr().GetThreat(target);
    }
}

void RegisterUnitMethods(sol::state& lua)
{
    sol::usertype<UnitRef> type = ALEBind::NewHandleType<UnitRef, WorldObjectRef, ObjectRef>(lua, "Unit");

    type["SetImmuneTo"]              = ALEBind::Method(&LuaUnit::SetImmuneTo);
    type["HandleStatFlatModifier"]   = ALEBind::Method(&LuaUnit::HandleStatFlatModifier);
    type["Attack"]                   = ALEBind::Method(&LuaUnit::Attack);
    type["AttackStop"]               = ALEBind::Method(&LuaUnit::AttackStop);
    type["IsStandState"]             = ALEBind::Method(&LuaUnit::IsStandState);
    type["IsMounted"]                = ALEBind::Method(&LuaUnit::IsMounted);
    type["IsRooted"]                 = ALEBind::Method(&LuaUnit::IsRooted);
    type["IsFullHealth"]             = ALEBind::Method(&LuaUnit::IsFullHealth);
    type["IsInAccessiblePlaceFor"]   = ALEBind::Method(&LuaUnit::IsInAccessiblePlaceFor);
    type["IsAuctioneer"]             = ALEBind::Method(&LuaUnit::IsAuctioneer);
    type["IsGuildMaster"]            = ALEBind::Method(&LuaUnit::IsGuildMaster);
    type["IsInnkeeper"]              = ALEBind::Method(&LuaUnit::IsInnkeeper);
    type["IsTrainer"]                = ALEBind::Method(&LuaUnit::IsTrainer);
    type["IsGossip"]                 = ALEBind::Method(&LuaUnit::IsGossip);
    type["IsTaxi"]                   = ALEBind::Method(&LuaUnit::IsTaxi);
    type["IsSpiritHealer"]           = ALEBind::Method(&LuaUnit::IsSpiritHealer);
    type["IsSpiritGuide"]            = ALEBind::Method(&LuaUnit::IsSpiritGuide);
    type["IsTabardDesigner"]         = ALEBind::Method(&LuaUnit::IsTabardDesigner);
    type["IsServiceProvider"]        = ALEBind::Method(&LuaUnit::IsServiceProvider);
    type["IsSpiritService"]          = ALEBind::Method(&LuaUnit::IsSpiritService);
    type["IsAlive"]                  = ALEBind::Method(&LuaUnit::IsAlive);
    type["IsDead"]                   = ALEBind::Method(&LuaUnit::IsDead);
    type["IsDying"]                  = ALEBind::Method(&LuaUnit::IsDying);
    type["IsBanker"]                 = ALEBind::Method(&LuaUnit::IsBanker);
    type["IsVendor"]                 = ALEBind::Method(&LuaUnit::IsVendor);
    type["IsBattleMaster"]           = ALEBind::Method(&LuaUnit::IsBattleMaster);
    type["IsCharmed"]                = ALEBind::Method(&LuaUnit::IsCharmed);
    type["IsArmorer"]                = ALEBind::Method(&LuaUnit::IsArmorer);
    type["IsAttackingPlayer"]        = ALEBind::Method(&LuaUnit::IsAttackingPlayer);
    type["IsPvPFlagged"]             = ALEBind::Method(&LuaUnit::IsPvPFlagged);
    type["IsOnVehicle"]              = ALEBind::Method(&LuaUnit::IsOnVehicle);
    type["IsInCombat"]               = ALEBind::Method(&LuaUnit::IsInCombat);
    type["IsUnderWater"]             = ALEBind::Method(&LuaUnit::IsUnderWater);
    type["IsInWater"]                = ALEBind::Method(&LuaUnit::IsInWater);
    type["IsStopped"]                = ALEBind::Method(&LuaUnit::IsStopped);
    type["IsQuestGiver"]             = ALEBind::Method(&LuaUnit::IsQuestGiver);
    type["HealthBelowPct"]           = ALEBind::Method(&LuaUnit::HealthBelowPct);
    type["HealthAbovePct"]           = ALEBind::Method(&LuaUnit::HealthAbovePct);
    type["HasAura"]                  = ALEBind::Method(&LuaUnit::HasAura);
    type["IsCasting"]                = ALEBind::Method(&LuaUnit::IsCasting);
    type["HasUnitState"]             = ALEBind::Method(&LuaUnit::HasUnitState);
    type["GetOwner"]                 = ALEBind::Method(&LuaUnit::GetOwner);
    type["GetOwnerGUID"]             = ALEBind::Method(&LuaUnit::GetOwnerGUID);
    type["GetMountId"]               = ALEBind::Method(&LuaUnit::GetMountId);
    type["GetCreatorGUID"]           = ALEBind::Method(&LuaUnit::GetCreatorGUID);
    type["GetCharmerGUID"]           = ALEBind::Method(&LuaUnit::GetCharmerGUID);
    type["GetCharmGUID"]             = ALEBind::Method(&LuaUnit::GetCharmGUID);
    type["GetPetGUID"]               = ALEBind::Method(&LuaUnit::GetPetGUID);
    type["GetControllerGUID"]        = ALEBind::Method(&LuaUnit::GetControllerGUID);
    type["GetControllerGUIDS"]       = ALEBind::Method(&LuaUnit::GetControllerGUIDS);
    type["GetStat"]                  = ALEBind::Method(&LuaUnit::GetStat);
    type["GetBaseSpellPower"]        = ALEBind::Method(&LuaUnit::GetBaseSpellPower);
    type["GetVictim"]                = ALEBind::Method(&LuaUnit::GetVictim);
    type["GetCurrentSpell"]          = ALEBind::Method(&LuaUnit::GetCurrentSpell);
    type["GetStandState"]            = ALEBind::Method(&LuaUnit::GetStandState);
    type["GetDisplayId"]             = ALEBind::Method(&LuaUnit::GetDisplayId);
    type["GetNativeDisplayId"]       = ALEBind::Method(&LuaUnit::GetNativeDisplayId);
    type["GetLevel"]                 = ALEBind::Method(&LuaUnit::GetLevel);
    type["GetHealth"]                = ALEBind::Method(&LuaUnit::GetHealth);
    type["GetPower"]                 = ALEBind::Method(&LuaUnit::GetPower);
    type["GetMaxPower"]              = ALEBind::Method(&LuaUnit::GetMaxPower);
    type["GetPowerPct"]              = ALEBind::Method(&LuaUnit::GetPowerPct);
    type["GetPowerType"]             = ALEBind::Method(&LuaUnit::GetPowerType);
    type["GetMaxHealth"]             = ALEBind::Method(&LuaUnit::GetMaxHealth);
    type["GetHealthPct"]             = ALEBind::Method(&LuaUnit::GetHealthPct);
    type["GetGender"]                = ALEBind::Method(&LuaUnit::GetGender);
    type["GetRace"]                  = ALEBind::Method(&LuaUnit::GetRace);
    type["GetClass"]                 = ALEBind::Method(&LuaUnit::GetClass);
    type["GetRaceMask"]              = ALEBind::Method(&LuaUnit::GetRaceMask);
    type["GetClassMask"]             = ALEBind::Method(&LuaUnit::GetClassMask);
    type["GetCreatureType"]          = ALEBind::Method(&LuaUnit::GetCreatureType);
    type["GetClassAsString"]         = ALEBind::Method(&LuaUnit::GetClassAsString);
    type["GetRaceAsString"]          = ALEBind::Method(&LuaUnit::GetRaceAsString);
    type["GetFaction"]               = ALEBind::Method(&LuaUnit::GetFaction);
    type["GetAura"]                  = ALEBind::Method(&LuaUnit::GetAura);
    type["GetFriendlyUnitsInRange"]  = ALEBind::Method(&LuaUnit::GetFriendlyUnitsInRange);
    type["GetUnfriendlyUnitsInRange"] = ALEBind::Method(&LuaUnit::GetUnfriendlyUnitsInRange);
    type["GetVehicleKit"]            = ALEBind::Method(&LuaUnit::GetVehicleKit);
    type["GetCritterGUID"]           = ALEBind::Method(&LuaUnit::GetCritterGUID);
    type["GetSpeed"]                 = ALEBind::Method(&LuaUnit::GetSpeed);
    type["GetSpeedRate"]             = ALEBind::Method(&LuaUnit::GetSpeedRate);
    type["GetMovementType"]          = ALEBind::Method(&LuaUnit::GetMovementType);
    type["GetAttackers"]             = ALEBind::Method(&LuaUnit::GetAttackers);
    type["SetOwnerGUID"]             = ALEBind::Method(&LuaUnit::SetOwnerGUID);
    type["SetPvP"]                   = ALEBind::Method(&LuaUnit::SetPvP);
    type["SetSheath"]                = ALEBind::Method(&LuaUnit::SetSheath);
    type["SetName"]                  = ALEBind::Method(&LuaUnit::SetName);
    type["SetSpeed"]                 = ALEBind::Method(&LuaUnit::SetSpeed);
    type["SetSpeedRate"]             = ALEBind::Method(&LuaUnit::SetSpeedRate);
    type["SetFaction"]               = ALEBind::Method(&LuaUnit::SetFaction);
    type["SetLevel"]                 = ALEBind::Method(&LuaUnit::SetLevel);
    type["SetHealth"]                = ALEBind::Method(&LuaUnit::SetHealth);
    type["SetMaxHealth"]             = ALEBind::Method(&LuaUnit::SetMaxHealth);
    type["SetPower"]                 = ALEBind::Method(&LuaUnit::SetPower);
    type["ModifyPower"]              = ALEBind::Method(&LuaUnit::ModifyPower);
    type["SetMaxPower"]              = ALEBind::Method(&LuaUnit::SetMaxPower);
    type["SetPowerType"]             = ALEBind::Method(&LuaUnit::SetPowerType);
    type["SetDisplayId"]             = ALEBind::Method(&LuaUnit::SetDisplayId);
    type["SetNativeDisplayId"]       = ALEBind::Method(&LuaUnit::SetNativeDisplayId);
    type["SetFacing"]                = ALEBind::Method(&LuaUnit::SetFacing);
    type["SetFacingToObject"]        = ALEBind::Method(&LuaUnit::SetFacingToObject);
    type["SetCreatorGUID"]           = ALEBind::Method(&LuaUnit::SetCreatorGUID);
    type["SetPetGUID"]               = ALEBind::Method(&LuaUnit::SetPetGUID);
    type["SetWaterWalk"]             = ALEBind::Method(&LuaUnit::SetWaterWalk);
    type["SetStandState"]            = ALEBind::Method(&LuaUnit::SetStandState);
    type["SetInCombatWith"]          = ALEBind::Method(&LuaUnit::SetInCombatWith);
    type["SetFFA"]                   = ALEBind::Method(&LuaUnit::SetFFA);
    type["SetSanctuary"]             = ALEBind::Method(&LuaUnit::SetSanctuary);
    type["SetCritterGUID"]           = ALEBind::Method(&LuaUnit::SetCritterGUID);
    type["SetRooted"]                = ALEBind::Method(&LuaUnit::SetRooted);
    type["SetConfused"]              = ALEBind::Method(&LuaUnit::SetConfused);
    type["SetFeared"]                = ALEBind::Method(&LuaUnit::SetFeared);
    type["ClearThreatList"]          = ALEBind::Method(&LuaUnit::ClearThreatList);
    type["GetThreatList"]            = ALEBind::Method(&LuaUnit::GetThreatList);
    type["Mount"]                    = ALEBind::Method(&LuaUnit::Mount);
    type["Dismount"]                 = ALEBind::Method(&LuaUnit::Dismount);
    type["PerformEmote"]             = ALEBind::Method(&LuaUnit::PerformEmote);
    type["EmoteState"]               = ALEBind::Method(&LuaUnit::EmoteState);
    type["CountPctFromCurHealth"]    = ALEBind::Method(&LuaUnit::CountPctFromCurHealth);
    type["CountPctFromMaxHealth"]    = ALEBind::Method(&LuaUnit::CountPctFromMaxHealth);
    type["SendChatMessageToPlayer"]  = ALEBind::Method(&LuaUnit::SendChatMessageToPlayer);
    type["MoveStop"]                 = ALEBind::Method(&LuaUnit::MoveStop);
    type["MoveExpire"]               = ALEBind::Method(&LuaUnit::MoveExpire);
    type["MoveClear"]                = ALEBind::Method(&LuaUnit::MoveClear);
    type["MoveIdle"]                 = ALEBind::Method(&LuaUnit::MoveIdle);
    type["MoveRandom"]               = ALEBind::Method(&LuaUnit::MoveRandom);
    type["MoveHome"]                 = ALEBind::Method(&LuaUnit::MoveHome);
    type["MoveFollow"]               = ALEBind::Method(&LuaUnit::MoveFollow);
    type["MoveChase"]                = ALEBind::Method(&LuaUnit::MoveChase);
    type["MoveConfused"]             = ALEBind::Method(&LuaUnit::MoveConfused);
    type["MoveFleeing"]              = ALEBind::Method(&LuaUnit::MoveFleeing);
    type["MoveTo"]                   = ALEBind::Method(&LuaUnit::MoveTo);
    type["MoveJump"]                 = ALEBind::Method(&LuaUnit::MoveJump);
    type["SendUnitWhisper"]          = ALEBind::Method(&LuaUnit::SendUnitWhisper);
    type["SendUnitEmote"]            = ALEBind::Method(&LuaUnit::SendUnitEmote);
    type["SendUnitSay"]              = ALEBind::Method(&LuaUnit::SendUnitSay);
    type["SendUnitYell"]             = ALEBind::Method(&LuaUnit::SendUnitYell);
    type["DeMorph"]                  = ALEBind::Method(&LuaUnit::DeMorph);
    type["CastSpell"]                = ALEBind::Method(&LuaUnit::CastSpell);
    type["CastCustomSpell"]          = ALEBind::Method(&LuaUnit::CastCustomSpell);
    type["CastSpellAoF"]             = ALEBind::Method(&LuaUnit::CastSpellAoF);
    type["ClearInCombat"]            = ALEBind::Method(&LuaUnit::ClearInCombat);
    type["StopSpellCast"]            = ALEBind::Method(&LuaUnit::StopSpellCast);
    type["InterruptSpell"]           = ALEBind::Method(&LuaUnit::InterruptSpell);
    type["AddAura"]                  = ALEBind::Method(&LuaUnit::AddAura);
    type["RemoveAura"]               = ALEBind::Method(&LuaUnit::RemoveAura);
    type["RemoveAllAuras"]           = ALEBind::Method(&LuaUnit::RemoveAllAuras);
    type["RemoveArenaAuras"]         = ALEBind::Method(&LuaUnit::RemoveArenaAuras);
    type["AddUnitState"]             = ALEBind::Method(&LuaUnit::AddUnitState);
    type["ClearUnitState"]           = ALEBind::Method(&LuaUnit::ClearUnitState);
    type["NearTeleport"]             = ALEBind::Method(&LuaUnit::NearTeleport);
    type["DealDamage"]               = ALEBind::Method(&LuaUnit::DealDamage);
    type["DealHeal"]                 = ALEBind::Method(&LuaUnit::DealHeal);
    type["Kill"]                     = ALEBind::Method(&LuaUnit::Kill);
    type["AddThreat"]                = ALEBind::Method(&LuaUnit::AddThreat);
    type["ModifyThreatPct"]          = ALEBind::Method(&LuaUnit::ModifyThreatPct);
    type["ClearThreat"]              = ALEBind::Method(&LuaUnit::ClearThreat);
    type["ResetAllThreat"]           = ALEBind::Method(&LuaUnit::ResetAllThreat);
    type["GetThreat"]                = ALEBind::Method(&LuaUnit::GetThreat);
}
