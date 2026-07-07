/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Item.h"
#include "Pet.h"
#include "SpellMgr.h"

/***
 * Non-[Player] controlled companions that fight alongside their owners.
 *
 * Includes hunter pets, warlock demons, death knight ghouls, and other summoned creatures.
 * [Pet]s can be temporary or permanent and have various AI behaviors and spell abilities.
 *
 * Inherits all methods from: [Object], [WorldObject], [Unit], [Guardian]
 */
namespace LuaPet
{
    /**
     * Returns the [Pet]'s type.
     *
     * <pre>
     * enum PetType
     * {
     *      SUMMON_PET                  = 0,
     *      HUNTER_PET                  = 1,
     *      MAX_PET_TYPE                = 4
     * };
     * </pre>
     *
     * @return [PetType] petType
     */
    PetType GetPetType(Pet* pet)
    {
        return pet->getPetType();
    }

    /**
     * Sets the [Pet]'s type.
     *
     * <pre>
     * enum PetType
     * {
     *      SUMMON_PET                  = 0,
     *      HUNTER_PET                  = 1,
     *      MAX_PET_TYPE                = 4
     * };
     * </pre>
     *
     * @param [PetType] petType : the pet type to set
     */
    void SetPetType(Pet* pet, uint32 petType)
    {
        pet->setPetType(static_cast<PetType>(petType));
    }

    /**
     * Returns `true` if the [Pet] is controlled by a player, returns `false` otherwise.
     *
     * @return bool isControlled
     */
    bool IsControlled(Pet* pet)
    {
        return pet->isControlled();
    }

    /**
     * Returns `true` if the [Pet] is temporarily summoned, returns `false` otherwise.
     *
     * @return bool isTemporary
     */
    bool IsTemporarySummoned(Pet* pet)
    {
        return pet->isTemporarySummoned();
    }

    /**
     * Returns `true` if the [Pet] is a permanent pet for the specified [Player], returns `false` otherwise.
     *
     * @param [Player] owner : the player to check ownership for
     * @return bool isPermanent
     */
    bool IsPermanentPetFor(Pet* pet, Player* owner)
    {
        return pet->IsPermanentPetFor(owner);
    }

    /**
     * Creates the [Pet]'s base stats and properties from an existing [Creature].
     *
     * @param [Creature] creature : the creature to base the pet on
     * @return bool success : `true` if successful, `false` otherwise
     */
    bool CreateBaseAtCreature(Pet* pet, Creature* creature)
    {
        return pet->CreateBaseAtCreature(creature);
    }

    /**
     * Returns the [Pet]'s remaining duration in milliseconds.
     *
     * @return uint32 duration : remaining time in milliseconds, 0 if permanent
     */
    int64 GetDuration(Pet* pet)
    {
        return pet->GetDuration().count();
    }

    /**
     * Sets the [Pet]'s duration in milliseconds.
     *
     * @param uint32 duration : duration in milliseconds, 0 for permanent
     */
    void SetDuration(Pet* pet, uint32 duration)
    {
        pet->SetDuration(Milliseconds(duration));
    }

    /**
     * Returns the [Pet]'s current happiness state.
     *
     * <pre>
     * enum HappinessState
     * {
     *      UNHAPPY                     = 1,
     *      CONTENT                     = 2,
     *      HAPPY                       = 3
     * };
     * </pre>
     *
     * @return [HappinessState] happinessState
     */
    HappinessState GetHappinessState(Pet* pet)
    {
        return pet->GetHappinessState();
    }

    /**
     * Gives experience points to the [Pet].
     *
     * @param uint32 xp : amount of experience to give
     */
    void GivePetXP(Pet* pet, uint32 xp)
    {
        pet->GivePetXP(xp);
    }

    /**
     * Sets the [Pet]'s level directly.
     *
     * @param uint8 level : the level to set
     */
    void GivePetLevel(Pet* pet, uint8 level)
    {
        pet->GivePetLevel(level);
    }

    /**
     * Synchronizes the [Pet]'s level with its owner's level.
     *
     * The pet's level will be adjusted based on the owner's level and pet scaling rules.
     */
    void SynchronizeLevelWithOwner(Pet* pet)
    {
        pet->SynchronizeLevelWithOwner();
    }

    /**
     * Returns `true` if the [Pet] can eat the specified [Item], returns `false` otherwise.
     *
     * @param [Item] item : the item to check
     * @return bool canEat
     */
    bool HaveInDiet(Pet* pet, Item* item)
    {
        return pet->HaveInDiet(item->GetTemplate());
    }

    /**
     * Returns the food benefit level for an item of the specified level.
     *
     * @param uint32 itemLevel : the level of the food item
     * @return uint32 benefitLevel
     */
    uint32 GetCurrentFoodBenefitLevel(Pet* pet, uint32 itemLevel)
    {
        return pet->GetCurrentFoodBenefitLevel(itemLevel);
    }

    /**
     * Toggles autocast on or off for the specified spell.
     *
     * @param uint32 spellId : the spell ID to toggle autocast for
     * @param bool apply : `true` to enable autocast, `false` to disable
     */
    void ToggleAutocast(Pet* pet, uint32 spellId, bool apply)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo)
            pet->ToggleAutocast(spellInfo, apply);
    }

    /**
     * Makes the [Pet] learn all passive spells it should know.
     *
     * This includes racial passives and pet-specific passive abilities.
     */
    void LearnPetPassives(Pet* pet)
    {
        pet->LearnPetPassives();
    }

    /**
     * Queues a spell to be cast when the [Pet] becomes available.
     *
     * @param uint32 spellId : the spell ID to cast
     * @param [Unit] target : the target for the spell
     * @param bool isPositive = false : whether the spell is beneficial
     */
    void CastWhenWillAvailable(Pet* pet, uint32 spellId, Unit* target, sol::optional<bool> isPositive)
    {
        ObjectGuid oldTarget = ObjectGuid::Empty;
        pet->CastWhenWillAvailable(spellId, target, oldTarget, isPositive.value_or(false));
    }

    /**
     * Clears any queued spell that was set to cast when available.
     */
    void ClearCastWhenWillAvailable(Pet* pet)
    {
        pet->ClearCastWhenWillAvailable();
    }

    /**
     * Adds a spell to the [Pet]'s spellbook.
     *
     * <pre>
     * enum ActiveStates : uint8
     * {
     *     ACT_PASSIVE  = 0x01,                                    // 0x01 - passive
     *     ACT_DISABLED = 0x81,                                    // 0x80 - castable
     *     ACT_ENABLED  = 0xC1,                                    // 0x40 | 0x80 - auto cast + castable
     *     ACT_COMMAND  = 0x07,                                    // 0x01 | 0x02 | 0x04
     *     ACT_REACTION = 0x06,                                    // 0x02 | 0x04
     *     ACT_DECIDE   = 0x00                                     // custom
     * };
     * </pre>
     *
     * <pre>
     * enum PetSpellState
     * {
     *     PETSPELL_UNCHANGED          = 0,
     *     PETSPELL_CHANGED            = 1,
     *     PETSPELL_NEW                = 2,
     *     PETSPELL_REMOVED            = 3
     * };
     * </pre>
     *
     * <pre>
     * enum PetSpellType
     * {
     *     PETSPELL_NORMAL             = 0,
     *     PETSPELL_FAMILY             = 1,
     *     PETSPELL_TALENT             = 2
     * };
     * </pre>
     *
     * @param uint32 spellId : the spell ID to add
     * @param [ActiveStates] active : the spell's active state by default is ACT_DECIDE
     * @param [PetSpellState] state : the spell's state by default is PETSPELL_NEW
     * @param [PetSpellType] type : the spell's type by default is PETSPELL_NORMAL
     * @return bool success : `true` if the spell was added successfully
     */
    bool AddSpell(Pet* pet, uint32 spellId, sol::optional<uint32> active, sol::optional<uint32> state,
        sol::optional<uint32> type)
    {
        return pet->addSpell(spellId,
            static_cast<ActiveStates>(active.value_or(ACT_DECIDE)),
            static_cast<PetSpellState>(state.value_or(PETSPELL_NEW)),
            static_cast<PetSpellType>(type.value_or(PETSPELL_NORMAL)));
    }

    /**
     * Makes the [Pet] learn a spell.
     *
     * @param uint32 spellId : the spell ID to learn
     * @return bool success : `true` if the spell was learned successfully
     */
    bool LearnSpell(Pet* pet, uint32 spellId)
    {
        return pet->learnSpell(spellId);
    }

    /**
     * Makes the [Pet] learn the highest available rank of a spell.
     *
     * @param uint32 spellId : the base spell ID
     */
    void LearnSpellHighRank(Pet* pet, uint32 spellId)
    {
        pet->learnSpellHighRank(spellId);
    }

    /**
     * Initializes level-up spells for the [Pet]'s current level.
     *
     * This teaches the pet all spells it should know at its current level.
     */
    void InitLevelupSpellsForLevel(Pet* pet)
    {
        pet->InitLevelupSpellsForLevel();
    }

    /**
     * Makes the [Pet] unlearn a spell.
     *
     * @param uint32 spellId : the spell ID to unlearn
     * @param bool learnPrev : if `true`, learns the previous rank by default is false
     * @param bool clearAb : if `true`, clears the spell from action bar by default is true
     * @return bool success : `true` if the spell was unlearned successfully
     */
    bool UnlearnSpell(Pet* pet, uint32 spellId, sol::optional<bool> learnPrev, sol::optional<bool> clearAb)
    {
        return pet->unlearnSpell(spellId, learnPrev.value_or(false), clearAb.value_or(true));
    }

    /**
     * Removes a spell from the [Pet]'s spellbook.
     *
     * @param uint32 spellId : the spell ID to remove
     * @param bool learnPrev : if `true`, learns the previous rank by default is false
     * @param bool clearAb : if `true`, clears the spell from action bar by default is true
     * @return bool success : `true` if the spell was removed successfully
     */
    bool RemoveSpell(Pet* pet, uint32 spellId, sol::optional<bool> learnPrev, sol::optional<bool> clearAb)
    {
        return pet->removeSpell(spellId, learnPrev.value_or(false), clearAb.value_or(true));
    }

    /**
     * Cleans up the [Pet]'s action bar, removing invalid spells.
     */
    void CleanupActionBar(Pet* pet)
    {
        pet->CleanupActionBar();
    }

    /**
     * Generates action bar data for the [Pet].
     *
     * @return string actionBarData : the action bar data as a string
     */
    std::string GenerateActionBarData(Pet* pet)
    {
        return pet->GenerateActionBarData();
    }

    /**
     * Initializes the [Pet]'s creation spells.
     *
     * This sets up the basic spells the pet should have when first created.
     */
    void InitPetCreateSpells(Pet* pet)
    {
        pet->InitPetCreateSpells();
    }

    /**
     * Resets all of the [Pet]'s talents.
     *
     * @return bool success : `true` if talents were reset successfully
     */
    bool ResetTalents(Pet* pet)
    {
        return pet->resetTalents();
    }

    /**
     * Initializes talents for the [Pet]'s current level.
     *
     * This assigns talent points based on the pet's level.
     */
    void InitTalentForLevel(Pet* pet)
    {
        pet->InitTalentForLevel();
    }

    /**
     * Returns the maximum number of talent points available at the specified level.
     *
     * @param uint8 level : the level to check
     * @return uint8 maxTalentPoints
     */
    uint8 GetMaxTalentPointsForLevel(Pet* pet, uint8 level)
    {
        return pet->GetMaxTalentPointsForLevel(level);
    }

    /**
     * Returns the number of unspent talent points the [Pet] has.
     *
     * @return uint8 freeTalentPoints
     */
    uint8 GetFreeTalentPoints(Pet* pet)
    {
        return pet->GetFreeTalentPoints();
    }

    /**
     * Sets the number of unspent talent points for the [Pet].
     *
     * @param uint8 points : the number of free talent points to set
     */
    void SetFreeTalentPoints(Pet* pet, uint8 points)
    {
        pet->SetFreeTalentPoints(points);
    }

    /**
     * Returns the number of talents the [Pet] has used.
     *
     * @return uint32 usedTalentCount
     */
    uint32 GetUsedTalentCount(Pet* pet)
    {
        return pet->m_usedTalentCount;
    }

    /**
     * Sets the number of talents the [Pet] has used.
     *
     * @param uint32 count : the number of used talents to set
     */
    void SetUsedTalentCount(Pet* pet, uint32 count)
    {
        pet->m_usedTalentCount = count;
    }

    /**
     * Returns the aura update mask for raid members.
     *
     * @return uint64 auraUpdateMask
     */
    uint64 GetAuraUpdateMaskForRaid(Pet* pet)
    {
        return pet->GetAuraUpdateMaskForRaid();
    }

    /**
     * Sets an aura slot in the raid update mask.
     *
     * @param uint8 slot : the aura slot to set
     */
    void SetAuraUpdateMaskForRaid(Pet* pet, uint8 slot)
    {
        pet->SetAuraUpdateMaskForRaid(slot);
    }

    /**
     * Resets the aura update mask for raid members.
     */
    void ResetAuraUpdateMaskForRaid(Pet* pet)
    {
        pet->ResetAuraUpdateMaskForRaid();
    }

    /**
     * Returns the [Player] who owns this [Pet].
     *
     * @return [Player] owner : the pet's owner
     */
    Player* GetOwner(Pet* pet)
    {
        return pet->GetOwner();
    }

    /**
     * Returns `true` if the [Pet] has a temporary spell queued, returns `false` otherwise.
     *
     * @return bool hasTempSpell
     */
    bool HasTempSpell(Pet* pet)
    {
        return pet->HasTempSpell();
    }

    /**
     * Returns `true` if the [Pet] is marked as removed, returns `false` otherwise.
     *
     * @return bool isRemoved
     */
    bool IsRemoved(Pet* pet)
    {
        return pet->m_removed;
    }

    /**
     * Sets whether the [Pet] is marked as removed.
     *
     * @param bool removed : `true` to mark as removed, `false` otherwise
     */
    void SetRemoved(Pet* pet, bool removed)
    {
        pet->m_removed = removed;
    }

    /**
     * Returns the number of auto-spells the [Pet] has.
     *
     * @return uint8 autoSpellCount
     */
    uint8 GetPetAutoSpellSize(Pet* pet)
    {
        return pet->GetPetAutoSpellSize();
    }

    /**
     * Returns the auto-spell at the specified position.
     *
     * @param uint8 pos : the position in the auto-spell list
     * @return uint32 spellId : the spell ID, or 0 if invalid position
     */
    uint32 GetPetAutoSpellOnPos(Pet* pet, uint8 pos)
    {
        return pet->GetPetAutoSpellOnPos(pos);
    }

    /**
     * Saves the [Pet] to the database.
     *
     * <pre>
     * enum PetSaveMode
     * {
     *     PET_SAVE_AS_DELETED         = -1,                        // not saved in fact
     *     PET_SAVE_AS_CURRENT         =  0,                        // in current slot (with player)
     *     PET_SAVE_FIRST_STABLE_SLOT  =  1,
     *     PET_SAVE_LAST_STABLE_SLOT   =  MAX_PET_STABLES,          // last in DB stable slot index (including), all higher have same meaning as PET_SAVE_NOT_IN_SLOT
     *     PET_SAVE_NOT_IN_SLOT        =  100                       // for avoid conflict with stable size grow will use 100
     * };
     * </pre>
     *
     * @param [PetSaveMode] mode : the save mode to use
     */
    void SavePetToDB(Pet* pet, uint32 mode)
    {
        pet->SavePetToDB(static_cast<PetSaveMode>(mode));
    }

    /**
     * Removes the [Pet] from the world.
     *
     * <pre>
     * enum PetSaveMode
     * {
     *     PET_SAVE_AS_DELETED         = -1,                        // not saved in fact
     *     PET_SAVE_AS_CURRENT         =  0,                        // in current slot (with player)
     *     PET_SAVE_FIRST_STABLE_SLOT  =  1,
     *     PET_SAVE_LAST_STABLE_SLOT   =  MAX_PET_STABLES,          // last in DB stable slot index (including), all higher have same meaning as PET_SAVE_NOT_IN_SLOT
     *     PET_SAVE_NOT_IN_SLOT        =  100                       // for avoid conflict with stable size grow will use 100
     * };
     * </pre>
     *
     * @param [PetSaveMode] mode : how to handle the removal
     * @param bool returnReagent = false : if `true`, returns reagents used to summon
     */
    void Remove(Pet* pet, uint32 mode, sol::optional<bool> returnReagent)
    {
        pet->Remove(static_cast<PetSaveMode>(mode), returnReagent.value_or(false));
    }

    /**
     * Returns `true` if the [Pet] is currently being loaded from the database, returns `false` otherwise.
     *
     * @return bool isBeingLoaded
     */
    bool IsBeingLoaded(Pet* pet)
    {
        return pet->isBeingLoaded();
    }
}

void RegisterPetMethods(sol::state& lua)
{
    sol::usertype<PetRef> type = ALEBind::NewHandleType<PetRef, CreatureRef, UnitRef, WorldObjectRef, ObjectRef>(lua, "Pet");

    type["GetPetType"]                 = ALEBind::Method(&LuaPet::GetPetType);
    type["SetPetType"]                 = ALEBind::Method(&LuaPet::SetPetType);
    type["IsControlled"]               = ALEBind::Method(&LuaPet::IsControlled);
    type["IsTemporarySummoned"]        = ALEBind::Method(&LuaPet::IsTemporarySummoned);
    type["IsPermanentPetFor"]          = ALEBind::Method(&LuaPet::IsPermanentPetFor);
    type["CreateBaseAtCreature"]       = ALEBind::Method(&LuaPet::CreateBaseAtCreature);
    type["GetDuration"]                = ALEBind::Method(&LuaPet::GetDuration);
    type["SetDuration"]                = ALEBind::Method(&LuaPet::SetDuration);
    type["GetHappinessState"]          = ALEBind::Method(&LuaPet::GetHappinessState);
    type["GivePetXP"]                  = ALEBind::Method(&LuaPet::GivePetXP);
    type["GivePetLevel"]               = ALEBind::Method(&LuaPet::GivePetLevel);
    type["SynchronizeLevelWithOwner"]  = ALEBind::Method(&LuaPet::SynchronizeLevelWithOwner);
    type["HaveInDiet"]                 = ALEBind::Method(&LuaPet::HaveInDiet);
    type["GetCurrentFoodBenefitLevel"] = ALEBind::Method(&LuaPet::GetCurrentFoodBenefitLevel);
    type["ToggleAutocast"]             = ALEBind::Method(&LuaPet::ToggleAutocast);
    type["LearnPetPassives"]           = ALEBind::Method(&LuaPet::LearnPetPassives);
    type["CastWhenWillAvailable"]      = ALEBind::Method(&LuaPet::CastWhenWillAvailable);
    type["ClearCastWhenWillAvailable"] = ALEBind::Method(&LuaPet::ClearCastWhenWillAvailable);
    type["AddSpell"]                   = ALEBind::Method(&LuaPet::AddSpell);
    type["LearnSpell"]                 = ALEBind::Method(&LuaPet::LearnSpell);
    type["LearnSpellHighRank"]         = ALEBind::Method(&LuaPet::LearnSpellHighRank);
    type["InitLevelupSpellsForLevel"]  = ALEBind::Method(&LuaPet::InitLevelupSpellsForLevel);
    type["UnlearnSpell"]               = ALEBind::Method(&LuaPet::UnlearnSpell);
    type["RemoveSpell"]                = ALEBind::Method(&LuaPet::RemoveSpell);
    type["CleanupActionBar"]           = ALEBind::Method(&LuaPet::CleanupActionBar);
    type["GenerateActionBarData"]      = ALEBind::Method(&LuaPet::GenerateActionBarData);
    type["InitPetCreateSpells"]        = ALEBind::Method(&LuaPet::InitPetCreateSpells);
    type["ResetTalents"]               = ALEBind::Method(&LuaPet::ResetTalents);
    type["InitTalentForLevel"]         = ALEBind::Method(&LuaPet::InitTalentForLevel);
    type["GetMaxTalentPointsForLevel"] = ALEBind::Method(&LuaPet::GetMaxTalentPointsForLevel);
    type["GetFreeTalentPoints"]        = ALEBind::Method(&LuaPet::GetFreeTalentPoints);
    type["SetFreeTalentPoints"]        = ALEBind::Method(&LuaPet::SetFreeTalentPoints);
    type["GetUsedTalentCount"]         = ALEBind::Method(&LuaPet::GetUsedTalentCount);
    type["SetUsedTalentCount"]         = ALEBind::Method(&LuaPet::SetUsedTalentCount);
    type["GetAuraUpdateMaskForRaid"]   = ALEBind::Method(&LuaPet::GetAuraUpdateMaskForRaid);
    type["SetAuraUpdateMaskForRaid"]   = ALEBind::Method(&LuaPet::SetAuraUpdateMaskForRaid);
    type["ResetAuraUpdateMaskForRaid"] = ALEBind::Method(&LuaPet::ResetAuraUpdateMaskForRaid);
    type["GetOwner"]                   = ALEBind::Method(&LuaPet::GetOwner);
    type["HasTempSpell"]               = ALEBind::Method(&LuaPet::HasTempSpell);
    type["IsRemoved"]                  = ALEBind::Method(&LuaPet::IsRemoved);
    type["SetRemoved"]                 = ALEBind::Method(&LuaPet::SetRemoved);
    type["GetPetAutoSpellSize"]        = ALEBind::Method(&LuaPet::GetPetAutoSpellSize);
    type["GetPetAutoSpellOnPos"]       = ALEBind::Method(&LuaPet::GetPetAutoSpellOnPos);
    type["SavePetToDB"]                = ALEBind::Method(&LuaPet::SavePetToDB);
    type["Remove"]                     = ALEBind::Method(&LuaPet::Remove);
    type["IsBeingLoaded"]              = ALEBind::Method(&LuaPet::IsBeingLoaded);
}
