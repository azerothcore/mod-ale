/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "AccountMgr.h"
#include "AchievementMgr.h"
#include "AuctionHouseMgr.h"
#include "Bag.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "GossipDef.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "InstanceSaveMgr.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Pet.h"
#include "Player.h"
#include "QuestDef.h"
#include "ReputationMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"

/***
 * Inherits all methods from: [Object], [WorldObject], [Unit]
 */
namespace LuaPlayer
{
    /**
     * Returns `true` if the [Player] can Titan Grip, `false` otherwise.
     *
     * @return bool canTitanGrip
     */
    bool CanTitanGrip(Player* player)
    {
        return player->CanTitanGrip();
    }

    /**
     * Returns `true` if the [Player] has a talent by ID in specified spec, `false` otherwise.
     *
     * @param uint32 spellId : talent spellId to check
     * @param uint8 spec : specified spec. 0 for primary, 1 for secondary.
     * @return bool hasTalent
     */
    sol::optional<bool> HasTalent(Player* player, uint32 spellId, uint8 spec)
    {
        uint8 maxSpecs = MAX_TALENT_SPECS;
        if (spec >= maxSpecs)
            return sol::nullopt;

        return player->HasTalent(spellId, spec);
    }

    /**
     * Returns `true` if the [Player] has completed the specified achievement, `false` otherwise.
     *
     * @param uint32 achievementId
     * @return bool hasAchieved
     */
    bool HasAchieved(Player* player, uint32 achievementId)
    {
        return player->HasAchieved(achievementId);
    }

    /**
     * Returns the progress of the [Player] for the specified achievement criteria.
     *
     * @param uint32 criteriaId
     * @return uint32 progress : progress value or nil
     */
    sol::optional<uint32> GetAchievementCriteriaProgress(Player* player, uint32 criteriaId)
    {
        AchievementCriteriaEntry const* criteria = sAchievementCriteriaStore.LookupEntry(criteriaId);
        CriteriaProgress* progress = player->GetAchievementMgr()->GetCriteriaProgress(criteria);
        if (progress)
            return progress->counter;

        return sol::nullopt;
    }

    /**
     * Returns `true` if the [Player] has an active [Quest] by specific ID, `false` otherwise.
     *
     * @param uint32 questId
     * @return bool hasQuest
     */
    bool HasQuest(Player* player, uint32 quest)
    {
        return player->IsActiveQuest(quest);
    }

    /**
     * Returns `true` if the [Player] has a skill by specific ID, `false` otherwise.
     *
     * @param uint32 skill
     * @return bool hasSkill
     */
    bool HasSkill(Player* player, uint32 skill)
    {
        return player->HasSkill(skill);
    }

    /**
     * Returns `true` if the [Player] has a [Spell] by specific ID, `false` otherwise.
     *
     * @param uint32 spellId
     * @return bool hasSpell
     */
    bool HasSpell(Player* player, uint32 id)
    {
        return player->HasSpell(id);
    }

    /**
     * Returns true if [Player] has specified login flag
     *
     * @param uint32 flag
     * @return bool hasLoginFlag
     */
    bool HasAtLoginFlag(Player* player, uint32 flag)
    {
        return player->HasAtLoginFlag((AtLoginFlags)flag);
    }

    /**
     * Returns true if [Player] has [Quest] for [GameObject]
     *
     * @param int32 entry : entry of a [GameObject]
     * @return bool hasQuest
     */
    bool HasQuestForGO(Player* player, int32 entry)
    {
        return player->HasQuestForGO(entry);
    }

    /**
     * Returns `true` if the [Player] has a title by specific ID, `false` otherwise.
     *
     * @param uint32 titleId
     * @return bool hasTitle
     */
    sol::optional<bool> HasTitle(Player* player, uint32 id)
    {
        CharTitlesEntry const* titleInfo = sCharTitlesStore.LookupEntry(id);
        if (titleInfo)
            return player->HasTitle(titleInfo);

        return sol::nullopt;
    }

    /**
     * Returns `true` if the [Player] has the given amount of item entry specified, `false` otherwise.
     *
     * @param uint32 itemId : entry of the item
     * @param uint32 count = 1 : amount of items the player needs should have
     * @param bool check_bank = false : determines if the item can be in player bank
     * @return bool hasItem
     */
    bool HasItem(Player* player, uint32 itemId, sol::optional<uint32> countArg, sol::optional<bool> checkBankArg)
    {
        uint32 count = countArg.value_or(1);
        bool check_bank = checkBankArg.value_or(false);
        return player->HasItemCount(itemId, count, check_bank);
    }

    /**
     * Returns `true` if the [Player] has a quest for the item entry specified, `false` otherwise.
     *
     * @param uint32 entry : entry of the item
     * @return bool hasQuest
     */
    bool HasQuestForItem(Player* player, uint32 entry)
    {
        return player->HasQuestForItem(entry);
    }

    /**
     * Returns `true` if the [Player] can use the item or item entry specified, `false` otherwise.
     *
     * @proto canUse = (item)
     * @proto canUse = (entry)
     * @param [Item] item : an instance of an item
     * @param uint32 entry : entry of the item
     * @return bool canUse
     */
    bool CanUseItem(Player* player, sol::object itemOrEntry)
    {
        if (itemOrEntry.is<ItemRef>())
        {
            Item* item = itemOrEntry.as<ItemRef>().Require();
            return player->CanUseItem(item) == EQUIP_ERR_OK;
        }

        uint32 entry = itemOrEntry.as<uint32>();
        ItemTemplate const* temp = sObjectMgr->GetItemTemplate(entry);
        if (temp)
            return player->CanUseItem(temp) == EQUIP_ERR_OK;

        return false;
    }

    /**
     * Returns `true` if the [Spell] specified by ID is currently on cooldown for the [Player], `false` otherwise.
     *
     * @param uint32 spellId
     * @return bool hasSpellCooldown
     */
    bool HasSpellCooldown(Player* player, uint32 spellId)
    {
        return player->HasSpellCooldown(spellId);
    }

    /**
     * Returns `true` if the [Player] can share [Quest] specified by ID, `false` otherwise.
     *
     * @param uint32 entryId
     * @return bool hasSpellCooldown
     */
    bool CanShareQuest(Player* player, uint32 entry)
    {
        return player->CanShareQuest(entry);
    }

    /**
     * Returns `true` if the [Player] can currently communicate through chat, `false` otherwise.
     *
     * @return bool canSpeak
     */
    bool CanSpeak(Player* player)
    {
        return player->CanSpeak();
    }

    /**
     * Returns `true` if the [Player] has permission to uninvite others from the current group, `false` otherwise.
     *
     * @return bool canUninviteFromGroup
     */
    bool CanUninviteFromGroup(Player* player)
    {
        return player->CanUninviteFromGroup() == ERR_PARTY_RESULT_OK;
    }

    /**
     * Returns `true` if the [Player] can fly, `false` otherwise.
     *
     * @return bool canFly
     */
    bool CanFly(Player* player)
    {
        return player->CanFly();
    }

    /**
     * Returns `true` if the [Player] is currently in water, `false` otherwise.
     *
     * @return bool isInWater
     */
    bool IsInWater(Player* player)
    {
        return player->IsInWater();
    }

    /**
     * Returns `true` if the [Player] is currently moving, `false` otherwise.
     *
     * @return bool isMoving
     */
    bool IsMoving(Player* player) // enable for unit when mangos support it
    {
        return player->isMoving();
    }

    /**
     * Returns `true` if the [Player] is currently flying, `false` otherwise.
     *
     * @return bool isFlying
     */
    bool IsFlying(Player* player) // enable for unit when mangos support it
    {
        return player->IsFlying();
    }

    /**
     * Returns the number of free slots in the [Player]'s inventory (backpack and equipped bags).
     *
     * @return uint32 freeSlots
     */
    uint32 GetInventoryFreeSlots(Player* player)
    {
        uint32 freeSlots = 0;

        // Backpack slots in INVENTORY_SLOT_BAG_0 (INVENTORY_SLOT_ITEM_START to INVENTORY_SLOT_ITEM_END)
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                ++freeSlots;
        }

        // Check equipped bags slots (INVENTORY_SLOT_BAG_START to INVENTORY_SLOT_BAG_END)
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Bag* bag = player->GetBagByPos(i))
            {
                for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                {
                    if (!player->GetItemByPos(i, j))
                        ++freeSlots;
                }
            }
        }

        return freeSlots;
    }

    /**
     * Returns the number of free slots in the [Player]'s bank (main bank and bank bags).
     *
     * @return uint32 freeSlots
     */
    uint32 GetBankFreeSlots(Player* player)
    {
        uint32 freeSlots = 0;

        // Check main bank slots (BANK_SLOT_ITEM_START to BANK_SLOT_ITEM_END)
        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                ++freeSlots;
        }

        // Check bank bags slots (BANK_SLOT_BAG_START to BANK_SLOT_BAG_END)
        for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (Bag* bag = player->GetBagByPos(i))
            {
                for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                {
                    if (!player->GetItemByPos(i, j))
                        ++freeSlots;
                }
            }
        }

        return freeSlots;
    }

    /**
     * Returns `true` if the [Player] has a Tank Specialization, `false` otherwise.
     *
     * @return bool HasTankSpec
     */
    bool HasTankSpec(Player* player)
    {
        return player->HasTankSpec();
    }

    /**
     * Returns `true` if the [Player] has a Melee Specialization, `false` otherwise.
     *
     * @return bool HasMeleeSpec
     */
    bool HasMeleeSpec(Player* player)
    {
        return player->HasMeleeSpec();
    }

    /**
     * Returns `true` if the [Player] has a Caster Specialization, `false` otherwise.
     *
     * @return bool HasCasterSpec
     */
    bool HasCasterSpec(Player* player)
    {
        return player->HasCasterSpec();
    }

    /**
     * Returns `true` if the [Player] has a Heal Specialization, `false` otherwise.
     *
     * @return bool HasHealSpec
     */
    bool HasHealSpec(Player* player)
    {
        return player->HasHealSpec();
    }

    /**
     * Returns `true` if the [Player] is in a [Group], `false` otherwise.
     *
     * @return bool isInGroup
     */
    bool IsInGroup(Player* player)
    {
        return player->GetGroup() != nullptr;
    }

    /**
     * Returns `true` if the [Player] is in a [Guild], `false` otherwise.
     *
     * @return bool isInGuild
     */
    bool IsInGuild(Player* player)
    {
        return player->GetGuildId() != 0;
    }

    /**
     * Returns `true` if the [Player] is a Game Master, `false` otherwise.
     *
     * Note: This is only true when GM tag is activated! For alternative see [Player:GetGMRank]
     *
     * @return bool isGM
     */
    bool IsGM(Player* player)
    {
        return player->IsGameMaster();
    }

    /**
     * Returns `true` if the [Player] is in an arena team specified by type, `false` otherwise.
     *
     * @param uint32 type
     * @return bool isInArenaTeam
     */
    bool IsInArenaTeam(Player* player, uint32 type)
    {
        if (type < MAX_ARENA_SLOT && player->GetArenaTeamId(type))
            return true;

        return false;
    }

    /**
     * Returns `true` if the [Player] is immune to everything.
     *
     * @return bool isImmune
     */
    bool IsImmuneToDamage(Player* player)
    {
        return player->isTotalImmune();
    }

    /**
     * Returns `true` if the [Player] satisfies all requirements to complete the quest entry.
     *
     * @param uint32 questId
     * @return bool canCompleteRepeatableQuest
     */
    bool CanCompleteRepeatableQuest(Player* player, uint32 questId)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId); // Retrieve the Quest object
        if (!quest)
            return false;

        return player->CanCompleteRepeatableQuest(quest);
    }

    /**
     * Returns `true` if the [Player] satisfies all requirements to reward the quest entry.
     *
     * @param uint32 questId
     * @return bool canRewardQuest
     */
    bool CanRewardQuest(Player* player, uint32 questId)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId); // Retrieve the Quest object
        if (!quest)
            return false;

        return player->CanRewardQuest(quest, true); // Modify the second argument as needed
    }

    /**
     * Returns `true` if the [Player] satisfies all requirements to complete the quest entry.
     *
     * @param uint32 entry
     * @return bool canComplete
     */
    bool CanCompleteQuest(Player* player, uint32 entry)
    {
        return player->CanCompleteQuest(entry);
    }

    /**
     * Returns `true` if the [Player] is a part of the Horde faction, `false` otherwise.
     *
     * @return bool isHorde
     */
    bool IsHorde(Player* player)
    {
        return player->GetTeamId() == TEAM_HORDE;
    }

    /**
     * Returns `true` if the [Player] is a part of the Alliance faction, `false` otherwise.
     *
     * @return bool isAlliance
     */
    bool IsAlliance(Player* player)
    {
        return player->GetTeamId() == TEAM_ALLIANCE;
    }

    /**
     * Returns `true` if the [Player] is 'Do Not Disturb' flagged, `false` otherwise.
     *
     * @return bool isDND
     */
    bool IsDND(Player* player)
    {
        return player->isDND();
    }

    /**
     * Returns `true` if the [Player] is 'Away From Keyboard' flagged, `false` otherwise.
     *
     * @return bool isAFK
     */
    bool IsAFK(Player* player)
    {
        return player->isAFK();
    }

    /**
     * Returns `true` if the [Player] is currently falling, `false` otherwise.
     *
     * @return bool isFalling
     */
    bool IsFalling(Player* player)
    {
        return player->IsFalling();
    }

    /**
     * Returns `true` if the [Player] is in the same group and visible to the specified [Player], `false` otherwise.
     *
     * @param [Player] player : the source player
     * @param [Player] target : the player to check visibility from
     * @return bool isGroupVisible
     */
    bool IsGroupVisibleFor(Player* player, Player* target)
    {
        return player->IsGroupVisibleFor(target);
    }

    /**
     * Returns `true` if the [Player] is currently in the same raid as another [Player] by object, `false` otherwise.
     *
     * @param [Player] player
     * @return bool isInSameRaidWith
     */
    bool IsInSameRaidWith(Player* player, Player* target)
    {
        return player->IsInSameRaidWith(target);
    }

    /**
     * Returns `true` if the [Player] is currently in the same [Group] as another [Player] by object, `false` otherwise.
     *
     * @param [Player] player
     * @return bool isInSameGroupWith
     */
    bool IsInSameGroupWith(Player* player, Player* target)
    {
        return player->IsInSameGroupWith(target);
    }

    /**
     * Returns `true` if the [Player] is eligible for Honor or XP gain by [Unit] specified, `false` otherwise.
     *
     * @param [Unit] unit
     * @return bool isHonorOrXPTarget
     */
    bool IsHonorOrXPTarget(Player* player, Unit* victim)
    {
        return player->isHonorOrXPTarget(victim);
    }

    /**
     * Returns `true` if the [Player] can see anoter [Player] specified by object, `false` otherwise.
     *
     * @param [Player] player
     * @return bool isVisibleForPlayer
     */
    bool IsVisibleForPlayer(Player* player, Player* target)
    {
        return player->IsVisibleGloballyFor(target);
    }

    /**
     * Returns `true` if the [Player] is currently visible to other players, `false` if hidden via GM invisibility.
     *
     * @param [Player] player
     * @return bool isVisible
     */
    bool IsGMVisible(Player* player)
    {
        return player->isGMVisible();
    }

    /**
     * Returns `true` if the [Player] has taxi cheat activated, `false` otherwise.
     *
     * @return bool isTaxiCheater
     */
    bool IsTaxiCheater(Player* player)
    {
        return player->isTaxiCheater();
    }

    /**
     * Returns `true` if the [Player] has GM chat enabled, `false` otherwise.
     *
     * @param [Player] player
     * @return bool isGMChat
     */
    bool IsGMChat(Player* player)
    {
        return player->isGMChat();
    }

    /**
     * Returns `true` if the [Player] is accepting whispers, `false` otherwise.
     *
     * @return bool isAcceptingWhispers
     */
    bool IsAcceptingWhispers(Player* player)
    {
        return player->isAcceptWhispers();
    }

    /**
     * Returns `true` if the [Player] is currently rested, `false` otherwise.
     *
     * @return bool isRested
     */
    bool IsRested(Player* player)
    {
        return player->GetRestBonus() > 0.0f;
    }

    /**
     * Returns `true` if the [Player] is currently in a [BattleGround] queue, `false` otherwise.
     *
     * @return bool inBattlegroundQueue
     */
    bool InBattlegroundQueue(Player* player)
    {
        return player->InBattlegroundQueue();
    }

    /**
     * Returns `true` if the [Player] is currently in an arena, `false` otherwise.
     *
     * @return bool inArena
     */
    bool InArena(Player* player)
    {
        return player->InArena();
    }

    /**
     * Returns `true` if the [Player] is currently in a [BattleGround], `false` otherwise.
     *
     * @return bool inBattleGround
     */
    bool InBattleground(Player* player)
    {
        return player->InBattleground();
    }

    /**
     * Returns `true` if the [Player] can block incomming attacks, `false` otherwise.
     *
     * @return bool canBlock
     */
    bool CanBlock(Player* player)
    {
        return player->CanBlock();
    }

    /**
     * Returns `true` if the [Player] can parry incomming attacks, `false` otherwise.
     *
     * @return bool canParry
     */
    bool CanParry(Player* player)
    {
        return player->CanParry();
    }

    /**
     * Returns the amount of available specs the [Player] currently has
     *
     * @return uint8 specCount
     */
    uint8 GetSpecsCount(Player* player)
    {
        return player->GetSpecsCount();
    }

    /**
     * Returns the [Player]s active spec ID
     *
     * @return uint32 specId
     */
    uint8 GetActiveSpec(Player* player)
    {
        return player->GetActiveSpec();
    }

    /**
     * Returns the normal phase of the player instead of the actual phase possibly containing GM phase
     *
     * @return uint32 phasemask
     */
    uint32 GetPhaseMaskForSpawn(Player* player)
    {
        return player->GetPhaseMaskForSpawn();
    }

    /**
     * Returns the [Player]s current amount of Achievement Points
     *
     * @return uint32 achievementPoints
     */
    uint32 GetAchievementPoints(Player* player)
    {
        uint32 count = 0;
        CompletedAchievementMap const& completedAchievements = player->GetAchievementMgr()->GetCompletedAchievements();
        for (auto& pair : completedAchievements)
        {
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(pair.first);
            if (achievement)
            {
                count += achievement->points;
            }
        }

        return count;
    }

    /**
     * Returns the [Player]s current amount of Achievements Completed
     *
     * @return uint32 achievementsCount
     */
    uint32 GetCompletedAchievementsCount(Player* player, sol::optional<bool> countFeatsOfStrengthArg)
    {
        uint32 count = 0;
        bool countFeatsOfStrength = countFeatsOfStrengthArg.value_or(false);
        CompletedAchievementMap const& completedAchievements = player->GetAchievementMgr()->GetCompletedAchievements();
        for (auto& pair : completedAchievements)
        {
            AchievementEntry const* achievement = sAchievementStore.LookupEntry(pair.first);
            if (achievement && (achievement->categoryId != 81 || countFeatsOfStrength))
            {
                count++;
            }
        }

        return count;
    }

    /**
     * Returns the [Player]s current amount of Arena Points
     *
     * @return uint32 arenaPoints
     */
    uint32 GetArenaPoints(Player* player)
    {
        return player->GetArenaPoints();
    }

    /**
     * Returns the [Player]s current amount of Honor Points
     *
     * @return uint32 honorPoints
     */
    uint32 GetHonorPoints(Player* player)
    {
        return player->GetHonorPoints();
    }

    /**
     * Returns the [Player]s today Honor points.
     *
     * @return uint32 todayHonorPoints
     */
    uint32 GetTodayHonorPoints(Player* player)
    {
        return player->GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION);
    }

    /**
     * Returns the [Player]s yesterday Honor points.
     *
     * @return uint32 yesterdayHonorPoints
     */
    uint32 GetYesterdayHonorPoints(Player* player)
    {
        return player->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION);
    }

    /**
     * Returns the [Player]s current shield block value
     *
     * @return uint32 blockValue
     */
    uint32 GetShieldBlockValue(Player* player)
    {
        return player->GetShieldBlockValue();
    }

    /**
     * Returns the [Player]s cooldown delay by specified [Spell] ID
     *
     * @param uint32 spellId
     * @return uint32 spellCooldownDelay
     */
    uint32 GetSpellCooldownDelay(Player* player, uint32 spellId)
    {
        return uint32(player->GetSpellCooldownDelay(spellId));
    }

    /**
     * Returns the [Player]s current latency in MS
     *
     * @return uint32 latency
     */
    uint32 GetLatency(Player* player)
    {
        return player->GetSession()->GetLatency();
    }

    /**
     * Returns the faction ID the [Player] is currently flagged as champion for
     *
     * @return uint32 championingFaction
     */
    uint32 GetChampioningFaction(Player* player)
    {
        return player->GetChampioningFaction();
    }

    /**
     * Returns [Player]s original sub group
     *
     * @return uint8 subGroup
     */
    uint8 GetOriginalSubGroup(Player* player)
    {
        return player->GetOriginalSubGroup();
    }

    /**
     * Returns [Player]s original [Group] object
     *
     * @return [Group] group
     */
    Group* GetOriginalGroup(Player* player)
    {
        return player->GetOriginalGroup();
    }

    /**
     * Returns a random Raid Member [Player] object within radius specified of [Player]
     *
     * @param float radius
     * @return [Player] player
     */
    Player* GetNextRandomRaidMember(Player* player, float radius)
    {
        return player->GetNextRandomRaidMember(radius);
    }

    /**
     * Returns [Player]s current sub group
     *
     * @return uint8 subGroup
     */
    uint8 GetSubGroup(Player* player)
    {
        return player->GetSubGroup();
    }

    /**
     * Returns [Group] invitation
     *
     * @return [Group] group
     */
    Group* GetGroupInvite(Player* player)
    {
        return player->GetGroupInvite();
    }

    /**
     * Returns the [Player]'s experience points
     *
     * @return uint32 xp
     */
    uint32 GetXP(Player* player)
    {
        return player->GetUInt32Value(PLAYER_XP);
    }

    /**
     * Returns rested experience bonus
     *
     * @param uint32 xp
     * @return uint32 xpBonus
     */
    uint32 GetXPRestBonus(Player* player, uint32 xp)
    {
        return player->GetXPRestBonus(xp);
    }

    /**
     * Returns the [Player]s current [BattleGround] type ID
     *
     * @return [BattleGroundTypeId] typeId
     */
    BattlegroundTypeId GetBattlegroundTypeId(Player* player)
    {
        return player->GetBattlegroundTypeId();
    }

    /**
     * Returns the [Player]s current [BattleGround] ID
     *
     * @return uint32 battleGroundId
     */
    uint32 GetBattlegroundId(Player* player)
    {
        return player->GetBattlegroundId();
    }

    /**
     * Returns the [Player]s reputation rank of faction specified
     *
     * @param uint32 faction
     * @return [ReputationRank] rank
     */
    ReputationRank GetReputationRank(Player* player, uint32 faction)
    {
        return player->GetReputationRank(faction);
    }

    /**
     * Returns the [Player]s current level of intoxication
     *
     * @return uint16 drunkValue
     */
    uint16 GetDrunkValue(Player* player)
    {
        return player->GetDrunkValue();
    }

    /**
     * Returns skill temporary bonus value
     *
     * @param uint32 skill
     * @param int16 bonusVal
     */
    int16 GetSkillTempBonusValue(Player* player, uint32 skill)
    {
        return player->GetSkillTempBonusValue(skill);
    }

    /**
     * Returns skill permanent bonus value
     *
     * @param uint32 skill
     * @param int16 bonusVal
     */
    int16 GetSkillPermBonusValue(Player* player, uint32 skill)
    {
        return player->GetSkillPermBonusValue(skill);
    }

    /**
     * Returns skill value without bonus'
     *
     * @param uint32 skill
     * @return uint16 pureVal
     */
    uint16 GetPureSkillValue(Player* player, uint32 skill)
    {
        return player->GetPureSkillValue(skill);
    }

    /**
     * Returns base skill value
     *
     * @param uint32 skill
     * @return uint16 baseVal
     */
    uint16 GetBaseSkillValue(Player* player, uint32 skill)
    {
        return player->GetBaseSkillValue(skill);
    }

    /**
     * Returns skill value
     *
     * @param uint32 skill
     * @return uint16 val
     */
    uint16 GetSkillValue(Player* player, uint32 skill)
    {
        return player->GetSkillValue(skill);
    }

    /**
     * Returns max value of specified skill without bonus'
     *
     * @param uint32 skill
     * @return uint16 pureVal
     */
    uint16 GetPureMaxSkillValue(Player* player, uint32 skill)
    {
        return player->GetPureMaxSkillValue(skill);
    }

    /**
     * Returns max value of specified skill
     *
     * @param uint32 skill
     * @return uint16 val
     */
    uint16 GetMaxSkillValue(Player* player, uint32 skill)
    {
        return player->GetMaxSkillValue(skill);
    }

    /**
     * Returns mana bonus from amount of intellect
     *
     * @return float bonus
     */
    float GetManaBonusFromIntellect(Player* player)
    {
        return player->GetManaBonusFromIntellect();
    }

    /**
     * Returns health bonus from amount of stamina
     *
     * @return float bonus
     */
    float GetHealthBonusFromStamina(Player* player)
    {
        return player->GetHealthBonusFromStamina();
    }

    /**
     * Returns raid or dungeon difficulty
     *
     * @param bool isRaid = true : argument is TrinityCore only
     * @return int32 difficulty
     */
    Difficulty GetDifficulty(Player* player, sol::optional<bool> isRaidArg)
    {
        bool isRaid = isRaidArg.value_or(true);
        return player->GetDifficulty(isRaid);
    }

    /**
     * Returns the [Player]s current guild rank
     *
     * @return uint32 guildRank
     */
    uint8 GetGuildRank(Player* player) // TODO: Move to Guild Methods
    {
        return player->GetRank();
    }

    /**
     * Returns the [Player]s free talent point amount
     *
     * @return uint32 freeTalentPointAmt
     */
    uint32 GetFreeTalentPoints(Player* player)
    {
        return player->GetFreeTalentPoints();
    }

    /**
     * Returns the name of the [Player]s current [Guild]
     *
     * @return string guildName
     */
    sol::optional<std::string> GetGuildName(Player* player)
    {
        if (!player->GetGuildId())
            return sol::nullopt;

        return sGuildMgr->GetGuildNameById(player->GetGuildId());
    }

    /**
     * Returns the amount of reputation the [Player] has with the faction specified
     *
     * @param uint32 faction
     * @return int32 reputationAmt
     */
    int32 GetReputation(Player* player, uint32 faction)
    {
        return player->GetReputationMgr().GetReputation(faction);
    }

    /**
     * Returns [Unit] target combo points are on
     *
     * @return [Unit] target
     */
    Unit* GetComboTarget(Player* player)
    {
        return player->GetComboTarget();
    }

    /**
     * Returns [Player]'s combo points
     *
     * @return uint8 comboPoints
     */
    uint8 GetComboPoints(Player* player)
    {
        return player->GetComboPoints();
    }

    /**
     * Returns the amount of time the [Player] has spent ingame
     *
     * @return uint32 inGameTime
     */
    uint32 GetInGameTime(Player* player)
    {
        return player->GetInGameTime();
    }

    /**
     * Returns the status of the [Player]s [Quest] specified by entry ID
     *
     * @param uint32 questId
     * @return [QuestStatus] questStatus
     */
    QuestStatus GetQuestStatus(Player* player, uint32 entry)
    {
        return player->GetQuestStatus(entry);
    }

    /**
     * Returns `true` if the [Player]s [Quest] specified by entry ID has been rewarded, `false` otherwise.
     *
     * @param uint32 questId
     * @return bool questRewardStatus
     */
    bool GetQuestRewardStatus(Player* player, uint32 questId)
    {
        return player->GetQuestRewardStatus(questId);
    }

    /**
     * Returns [Quest] required [Creature] or [GameObject] count
     *
     * @param uint32 quest : entry of a quest
     * @param int32 entry : entry of required [Creature]
     * @return uint16 count
     */
    uint16 GetReqKillOrCastCurrentCount(Player* player, uint32 questId, int32 entry)
    {
        return player->GetReqKillOrCastCurrentCount(questId, entry);
    }

    /**
     * Returns the quest level of the [Player]s [Quest] specified by object
     *
     * @param uint32 questId
     * @return [QuestStatus] questRewardStatus
     */
    int32 GetQuestLevel(Player* player, Quest* quest)
    {
        return player->GetQuestLevel(quest);
    }

    /**
     * Returns a [Player]s [Item] object by gear slot specified
     *
     * @param uint8 slot
     * @return [Item] item
     */
    Item* GetEquippedItemBySlot(Player* player, uint8 slot)
    {
        if (slot >= EQUIPMENT_SLOT_END)
            return nullptr;

        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        return item;
    }

    /**
     * Returns the [Player]s current resting bonus
     *
     * @return float restBonus
     */
    float GetRestBonus(Player* player)
    {
        return player->GetRestBonus();
    }

    /**
     * Returns active GM chat tag
     *
     * @return uint8 tag
     */
    uint8 GetChatTag(Player* player)
    {
        return player->GetChatTag();
    }

    /**
     * Returns an item in given bag on given slot.
     *
     * <pre>
     * Possible and most commonly used combinations:
     *
     * bag = 255
     * slots 0-18 equipment
     * slots 19-22 equipped bag slots
     * slots 23-38 backpack
     * slots 39-66 bank main slots
     * slots 67-74 bank bag slots
     * slots 86-117 keyring
     *
     * bag = 19-22
     * slots 0-35 for equipped bags
     *
     * bag = 67-74
     * slots 0-35 for bank bags
     * </pre>
     *
     * @param uint8 bag : the bag the [Item] is in, you can get this with [Item:GetBagSlot]
     * @param uint8 slot : the slot the [Item] is in within the bag, you can get this with [Item:GetSlot]
     * @return [Item] item : [Item] or nil
     */
    Item* GetItemByPos(Player* player, uint8 bag, uint8 slot)
    {
        return player->GetItemByPos(bag, slot);
    }

    /**
     * Returns an [Item] from the player by guid.
     *
     * The item can be equipped, in bags or in bank.
     *
     * @param ObjectGuid guid : an item guid
     * @return [Item] item
     */
    Item* GetItemByGUID(Player* player, ObjectGuid guid)
    {
        return player->GetItemByGuid(guid);
    }

    /**
     * Returns the amount of mails in the player's mailbox.
     *
     * @return uint32 mailCount
     */
    uint32 GetMailCount(Player* player)
    {
        CharacterCacheEntry const* cache = sCharacterCache->GetCharacterCacheByGuid(player->GetGUID());
        if (cache)
            return static_cast<uint32>(cache->MailCount);

        return player->GetMailSize();
    }

    /**
     * Returns a mailed [Item] by guid.
     *
     * @param ObjectGuid guid : an item guid
     * @return [Item] item
     */
    Item* GetMailItem(Player* player, ObjectGuid guid)
    {
        return player->GetMItem(guid.GetCounter());
    }

    /**
     * Returns an [Item] from the player by entry.
     *
     * The item can be equipped, in bags or in bank.
     *
     * @param uint32 entryId
     * @return [Item] item
     */
    Item* GetItemByEntry(Player* player, uint32 entry)
    {
        return player->GetItemByEntry(entry);
    }

    /**
     * Returns the database textID of the [WorldObject]'s gossip header text for the [Player]
     *
     * @param [WorldObject] object
     * @return uint32 textId : key to npc_text database table
     */
    uint32 GetGossipTextId(Player* player, WorldObject* obj)
    {
        return player->GetGossipTextId(obj);
    }

    /**
     * Returns the [Player]s currently selected [Unit] object
     *
     * @return [Unit] unit
     */
    Unit* GetSelection(Player* player)
    {
        return player->GetSelectedUnit();
    }

    /**
     * Returns the [Player]s GM Rank
     *
     * @return [AccountTypes] gmRank
     */
    AccountTypes GetGMRank(Player* player)
    {
        return player->GetSession()->GetSecurity();
    }

    /**
     * Returns the [Player]s amount of money in copper
     *
     * @return uint32 coinage
     */
    uint32 GetCoinage(Player* player)
    {
        return player->GetMoney();
    }

    /**
     * Returns the [Player]s current [Guild] ID
     *
     * @return uint32 guildId
     */
    uint32 GetGuildId(Player* player)
    {
        return player->GetGuildId();
    }

    /**
     * Returns the [Player]s [TeamId]
     *
     * @return [TeamId] teamId
     */
    TeamId GetTeam(Player* player)
    {
        return player->GetTeamId();
    }

    /**
     * Returns amount of the specified [Item] the [Player] has.
     *
     * @param uint32 entry : entry of the item
     * @param bool checkinBank = false : also counts the items in player's bank if true
     * @return uint32 itemamount
     */
    uint32 GetItemCount(Player* player, uint32 entry, sol::optional<bool> checkinBankArg)
    {
        bool checkinBank = checkinBankArg.value_or(false);
        return player->GetItemCount(entry, checkinBank);
    }

    /**
     * Returns the [Player]s lifetime Honorable Kills
     *
     * @return uint32 lifeTimeKils
     */
    uint32 GetLifetimeKills(Player* player)
    {
        return player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
    }

    /**
     * Returns the [Player]s today Honorable Kills
     *
     * @return uint32 todayKills
     */
    uint32 GetTodayKills(Player* player)
    {
        return uint32(player->GetUInt16Value(PLAYER_FIELD_KILLS, 0));
    }

    /**
     * Returns the [Player]s yesterday Honorable Kills
     *
     * @return uint32 yesterdayKills
     */
    uint32 GetYesterdayKills(Player* player)
    {
        return uint32(player->GetUInt16Value(PLAYER_FIELD_KILLS, 1));
    }

    /**
     * Returns the [Player]s IP address
     *
     * @return string ip
     */
    std::string GetPlayerIP(Player* player)
    {
        return player->GetSession()->GetRemoteAddress();
    }

    /**
     * Returns the [Player]s time played at current level
     *
     * @return uint32 currLevelPlayTime
     */
    uint32 GetLevelPlayedTime(Player* player)
    {
        return player->GetLevelPlayedTime();
    }

    /**
     * Returns the [Player]s total time played
     *
     * @return uint32 totalPlayTime
     */
    uint32 GetTotalPlayedTime(Player* player)
    {
        return player->GetTotalPlayedTime();
    }

    /**
     * Returns the [Player]s [Guild] object
     *
     * @return [Guild] guild
     */
    Guild* GetGuild(Player* player)
    {
        return sGuildMgr->GetGuildById(player->GetGuildId());
    }

    /**
     * Returns the [Player]s [Group] object
     *
     * @return [Group] group
     */
    Group* GetGroup(Player* player)
    {
        return player->GetGroup();
    }

    /**
     * Returns the [Player]s account ID
     *
     * @return uint32 accountId
     */
    uint32 GetAccountId(Player* player)
    {
        return player->GetSession()->GetAccountId();
    }

    /**
     * Returns the [Player]s account name
     *
     * @return string accountName
     */
    sol::optional<std::string> GetAccountName(Player* player)
    {
        std::string accName;
        if (AccountMgr::GetName(player->GetSession()->GetAccountId(), accName))
            return accName;

        return sol::nullopt;
    }

    /**
     * Returns the [Player]s completed quest count
     *
     * @return int32 questcount
     */
    uint32 GetCompletedQuestsCount(Player* player)
    {
        uint32 count = player->GetRewardedQuestCount();

        return count;
    }

    /**
     * Returns the [Player]s [Corpse] object
     *
     * @return [Corpse] corpse
     */
    Corpse* GetCorpse(Player* player)
    {
        return player->GetCorpse();
    }

    /**
     * Returns the [Player]s database locale index
     *
     * @return int localeIndex
     */
    LocaleConstant GetDbLocaleIndex(Player* player)
    {
        return player->GetSession()->GetSessionDbLocaleIndex();
    }

    /**
     * Returns the [Player]s game client locale
     *
     * @return [LocaleConstant] locale
     */
    LocaleConstant GetDbcLocale(Player* player)
    {
        return player->GetSession()->GetSessionDbcLocale();
    }

    /**
     * Returns known taxi nodes (flight paths) that the player has unlocked.
     *
     * @return table nodes : A table containing the IDs of the known taxi nodes
     */
    sol::table GetKnownTaxiNodes(Player* player, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();

        ByteBuffer data;
        player->m_taxi.AppendTaximaskTo(data, false);

        uint32 idx = 1;

        for (uint8 i = 0; i < TaxiMaskSize; i++)
        {
            uint32 mask;
            data >> mask;

            for (uint8 bit = 0; bit < 32; bit++)
            {
                if (mask & (1u << bit))
                {
                    uint32 nodeId = (i * 32) + bit + 1;
                    tbl[idx++] = nodeId;
                }
            }
        }

        return tbl;
    }

    /**
     * Locks the player controls and disallows all movement and casting.
     *
     * @param bool apply = true : lock if true and unlock if false
     */
    void SetPlayerLock(Player* player, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(true);

        if (apply)
        {
            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
            player->SetClientControl(player, 0);
        }
        else
        {
            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);
            player->SetClientControl(player, 1);
        }
    }

    /**
     * Sets the [Player]s login flag to the flag specified
     *
     * @param uint32 flag
     */
    void SetAtLoginFlag(Player* player, uint32 flag)
    {
        player->SetAtLoginFlag((AtLoginFlags)flag);
    }

    /**
     * Sets the [Player]s sheathe state to the state specified
     *
     * @param uint32 sheatheState
     */
    void SetSheath(Player* player, uint32 sheathed)
    {
        if (sheathed >= MAX_SHEATH_STATE)
            return;

        player->SetSheath((SheathState)sheathed);
    }

    /**
     * Sets the [Player]s intoxication level to the level specified
     *
     * @param uint8 drunkValue
     */
    void SetDrunkValue(Player* player, uint8 newDrunkValue)
    {
        player->SetDrunkValue(newDrunkValue);
    }

    /**
     * Sets the [Player]s faction standing to that of the race specified
     *
     * @param uint8 raceId
     */
    void SetFactionForRace(Player* player, uint8 race)
    {
        player->SetFactionForRace(race);
    }

    /**
     * Sets (increases) skill of the [Player]
     *
     * @param uint16 id
     * @param uint16 step
     * @param uint16 currVal
     * @param uint16 maxVal
     */
    void SetSkill(Player* player, uint16 id, uint16 step, uint16 currVal, uint16 maxVal)
    {
        player->SetSkill(id, currVal, maxVal, step);
    }

    /**
     * Sets the [Player]s guild rank to the rank specified
     *
     * @param uint8 rank
     */
    void SetGuildRank(Player* player, uint8 rank) // TODO: Move to Guild Methods
    {
        if (!player->GetGuildId())
            return;

        player->SetRank(rank);
    }

    /**
     * Sets the [Player]s free talent points to the amount specified for the current spec
     *
     * @param uint32 talentPointAmt
     */
    void SetFreeTalentPoints(Player* player, uint32 points)
    {
        player->SetFreeTalentPoints(points);
        player->SendTalentsInfoData(false);
    }

    /**
     * Sets the [Player]s reputation amount for the faction specified
     *
     * @param uint32 factionId
     * @param int32 reputationValue
     */
    void SetReputation(Player* player, uint32 faction, int32 value)
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction);
        player->GetReputationMgr().SetReputation(factionEntry, value);
    }

    /**
     * Sets [Quest] state
     *
     * @param uint32 entry : entry of a quest
     * @param uint32 status
     */
    void SetQuestStatus(Player* player, uint32 entry, uint32 status)
    {
        if (status >= MAX_QUEST_STATUS)
            return;

        player->SetQuestStatus(entry, (QuestStatus)status);
    }

    /**
     * Sets the [Player]s rest bonus to the amount specified
     *
     * @param float restBonus
     */
    void SetRestBonus(Player* player, float bonus)
    {
        player->SetRestBonus(bonus);
    }

    /**
     * Toggles whether the [Player] accepts whispers or not
     *
     * @param bool acceptWhispers = true
     */
    void SetAcceptWhispers(Player* player, sol::optional<bool> onArg)
    {
        bool on = onArg.value_or(true);

        player->SetAcceptWhispers(on);
    }

    /**
     * Toggles PvP Death
     *
     * @param bool on = true
     */
    void SetPvPDeath(Player* player, sol::optional<bool> onArg)
    {
        bool on = onArg.value_or(true);

        player->SetPvPDeath(on);
    }

    /**
     * Toggles whether the [Player] has GM visibility on or off
     *
     * @param bool gmVisible = true
     */
    void SetGMVisible(Player* player, sol::optional<bool> onArg)
    {
        bool on = onArg.value_or(true);

        player->SetGMVisible(on);
    }

    /**
     * Sets the player's known taxi nodes (flight paths).
     *
     * @param table nodes : A table containing the taxi node IDs to set as known
     */
    void SetKnownTaxiNodes(Player* player, sol::table nodes)
    {
        for (auto const& kv : nodes)
        {
            uint32 nodeId = kv.second.as<uint32>();

            if (nodeId > 0)
                player->m_taxi.SetTaximaskNode(nodeId);
        }
    }

    /**
     * Toggles whether the [Player] has taxi cheat enabled or not
     *
     * @param bool taxiCheat = true
     */
    void SetTaxiCheat(Player* player, sol::optional<bool> onArg)
    {
        bool on = onArg.value_or(true);

        player->SetTaxiCheater(on);
    }

    /**
     * Toggle Blizz (GM) tag
     *
     * @param bool on = true
     */
    void SetGMChat(Player* player, sol::optional<bool> onArg)
    {
        bool on = onArg.value_or(true);

        player->SetGMChat(on);
    }

    /**
     * Toggles the [Player]s GM mode on or off
     *
     * @param bool setGmMode = true
     */
    void SetGameMaster(Player* player, sol::optional<bool> onArg)
    {
        bool on = onArg.value_or(true);

        player->SetGameMaster(on);
    }

    /**
     * Sets the [Player]s gender to gender specified
     *
     * - GENDER_MALE    = 0
     * - GENDER_FEMALE  = 1
     *
     * @param [Gender] gender
     */
    void SetGender(Player* player, uint32 _gender)
    {
        Gender gender;
        switch (_gender)
        {
            case 0:
                gender = GENDER_MALE;
                break;
            case 1:
                gender = GENDER_FEMALE;
                break;
            default:
                throw std::invalid_argument("valid Gender expected");
        }

        player->SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);
        player->SetByteValue(PLAYER_BYTES_3, 0, gender);
        player->InitDisplayIds();
    }

    /**
     * Sets the [Player]s Arena Points to the amount specified
     *
     * @param uint32 arenaPoints
     */
    void SetArenaPoints(Player* player, uint32 arenaP)
    {
        player->SetArenaPoints(arenaP);
    }

    /**
     * Sets the [Player]s Honor Points to the amount specified
     *
     * @param uint32 honorPoints
     */
    void SetHonorPoints(Player* player, uint32 honorP)
    {
        player->SetHonorPoints(honorP);
    }

    /**
     * Sets the [Player]s amount of Lifetime Honorable Kills to the value specified
     *
     * @param uint32 honorableKills
     */
    void SetLifetimeKills(Player* player, uint32 val)
    {
        player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, val);
    }

    /**
     * Sets the [Player]s amount of money to copper specified
     *
     * @param uint32 copperAmt
     */
    void SetCoinage(Player* player, uint32 amt)
    {
        player->SetMoney(amt);
    }

    /**
     * Sets the [Player]s home location to the location specified
     *
     * @param float x : X Coordinate
     * @param float y : Y Coordinate
     * @param float z : Z Coordinate
     * @param uint32 mapId : Map ID
     * @param uint32 areaId : Area ID
     */
    void SetBindPoint(Player* player, float x, float y, float z, uint32 mapId, uint32 areaId)
    {
        WorldLocation loc(mapId, x, y, z);
        player->SetHomebind(loc, areaId);
    }

    /**
     * Adds the specified title to the [Player]s list of known titles
     *
     * @param uint32 titleId
     */
    void SetKnownTitle(Player* player, uint32 id)
    {
        CharTitlesEntry const* t = sCharTitlesStore.LookupEntry(id);
        if (t)
            player->SetTitle(t, false);
    }

    /**
     * Adds the specified achievement to the [Player]s
     *
     * @param uint32 achievementid
     */
    void SetAchievement(Player* player, uint32 id)
    {
        AchievementEntry const* t = sAchievementStore.LookupEntry(id);
        if (t)
            player->CompletedAchievement(t);
    }

    /**
     * Reset the [Player]s completed achievements
     */
    void ResetAchievements(Player* player)
    {
        player->ResetAchievements();
    }

    /**
     * Shows the mailbox window to the player from specified guid.
     *
     * @param ObjectGuid guid = playerguid : guid of the mailbox window sender
     */
    void SendShowMailBox(Player* player, sol::optional<ObjectGuid> guidArg)
    {
        ObjectGuid guid = guidArg.value_or(player->GetGUID());

        player->GetSession()->SendShowMailBox(guid);
    }

    /**
     * Adds or detracts from the [Player]s current Arena Points
     *
     * @param int32 amount
     */
    void ModifyArenaPoints(Player* player, int32 amount)
    {
        player->ModifyArenaPoints(amount);
    }

    /**
     * Adds or detracts from the [Player]s current Honor Points
     *
     * @param int32 amount
     */
    void ModifyHonorPoints(Player* player, int32 amount)
    {
        player->ModifyHonorPoints(amount);
    }

    /**
     * Saves the [Player] to the database
     */
    void SaveToDB(Player* player)
    {
        player->SaveToDB(false, false);
    }

    /**
     * Sends a summon request to the player from the given summoner
     *
     * @param [Unit] summoner
     */
    void SummonPlayer(Player* player, Unit* summoner)
    {
        float x, y, z;
        summoner->GetPosition(x,y,z);
        player->SetSummonPoint(summoner->GetMapId(), x, y, z);

        WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
        data << summoner->GetGUID();
        data << uint32(summoner->GetZoneId());
        data << uint32(MAX_PLAYER_SUMMON_DELAY * IN_MILLISECONDS);
        player->GetSession()->SendPacket(&data);
    }

    /**
     * Mutes the [Player] for the amount of seconds specified
     *
     * @param uint32 muteTime
     */
    void Mute(Player* player, uint32 muteseconds)
    {
        /*const char* reason = luaL_checkstring(E, 2);*/ // Mangos does not have a reason field in database.

        time_t muteTime = GameTime::GetGameTime().count() + muteseconds;
        player->GetSession()->m_muteTime = muteTime;
        LoginDatabase.Execute("UPDATE account SET mutetime = {} WHERE id = {}", muteTime, player->GetSession()->GetAccountId());
    }

    /**
     * Rewards the given quest entry for the [Player] if he has completed it.
     *
     * @param uint32 entry : quest entry
     */
    void RewardQuest(Player* player, uint32 entry)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) != QUEST_STATUS_COMPLETE)
            return;

        player->RewardQuest(quest, 0, player);
    }

    /**
     * Sends an auction house window to the [Player] from the [Unit] specified
     *
     * @param [Unit] sender
     */
    void SendAuctionMenu(Player* player, Unit* unit)
    {
        AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntryFromFactionTemplate(unit->GetFaction());
        if (!ahEntry)
            return;

        WorldPacket data(MSG_AUCTION_HELLO, 12);
        data << unit->GetGUID();
        data << uint32(ahEntry->houseId);
        data << uint8(1);
        player->GetSession()->SendPacket(&data);
    }

    /**
     * Sends a flightmaster window to the [Player] from the [Creature] specified
     *
     * @param [Creature] sender
     */
    void SendTaxiMenu(Player* player, Creature* creature)
    {
        player->GetSession()->SendTaxiMenu(creature);
    }

    /**
     * Sends a spirit resurrection request to the [Player]
     */
    void SendSpiritResurrect(Player* player)
    {
        player->GetSession()->SendSpiritResurrect();
    }

    /**
     * Sends a tabard vendor window to the [Player] from the [WorldObject] specified
     *
     * @param [WorldObject] sender
     */
    void SendTabardVendorActivate(Player* player, WorldObject* obj)
    {
        player->GetSession()->SendTabardVendorActivate(obj->GetGUID());
    }

    /**
     * Sends a bank window to the [Player] from the [WorldObject] specified.
     *
     * @param [WorldObject] sender
     */
    void SendShowBank(Player* player, WorldObject* obj)
    {
        player->GetSession()->SendShowBank(obj->GetGUID());
    }

    /**
     * Sends a vendor window to the [Player] from the [WorldObject] specified.
     *
     * @param [WorldObject] sender
     * @param uint32 vendorId = 0 : optional entry ID to use for the vendor item list, overriding the sender's default inventory
     */
    void SendListInventory(Player* player, WorldObject* obj, sol::optional<uint32> vendorIdArg)
    {
        uint32 vendorId = vendorIdArg.value_or(0);

        Creature* creature = obj->ToCreature();
        bool addedVendorFlag = false;
        if (vendorId && creature && !creature->HasNpcFlag(UNIT_NPC_FLAG_VENDOR))
        {
            creature->SetNpcFlag(UNIT_NPC_FLAG_VENDOR);
            addedVendorFlag = true;
        }

        player->GetSession()->SendListInventory(obj->GetGUID(), vendorId);

        if (addedVendorFlag)
            creature->RemoveNpcFlag(UNIT_NPC_FLAG_VENDOR);
    }

    /**
     * Sends a trainer window to the [Player] from the [Creature] specified
     *
     * @param [Creature] sender
     */
    void SendTrainerList(Player* player, Creature* obj)
    {
        player->GetSession()->SendTrainerList(obj);
    }

    /**
     * Sends a guild invitation from the [Player]s [Guild] to the [Player] object specified
     *
     * @param [Player] invitee
     */
    void SendGuildInvite(Player* player, Player* plr)
    {
        if (Guild* guild = player->GetGuild())
            guild->HandleInviteMember(player->GetSession(), plr->GetName());
    }

    /**
     * Sends an update for the world state to the [Player]
     *
     * @param uint32 field
     * @param uint32 value
     */
    void SendUpdateWorldState(Player* player, uint32 field, uint32 value)
    {
        player->SendUpdateWorldState(field, value);
    }

    /**
     * Forces the [Player] to log out
     *
     * @param bool saveToDb = true
     */
    void LogoutPlayer(Player* player, sol::optional<bool> saveArg)
    {
        bool save = saveArg.value_or(true);

        player->GetSession()->LogoutPlayer(save);
    }

    /**
     * Forcefully removes the [Player] from a [BattleGround] raid group
     */
    void RemoveFromBattlegroundRaid(Player* player)
    {
        player->RemoveFromBattlegroundOrBattlefieldRaid();
    }

    /**
     * Unbinds the [Player] from his instances except the one he currently is in.
     *
     * Difficulty is not used on classic.
     *
     * @param uint32 map = true
     * @param uint32 difficulty = 0
     */
    void UnbindInstance(Player* player, uint32 map, sol::optional<uint32> difficultyArg)
    {
        uint32 difficulty = difficultyArg.value_or(0);

        if (difficulty < MAX_DIFFICULTY)
            sInstanceSaveMgr->PlayerUnbindInstance(player->GetGUID(), map, Difficulty(difficulty), true, player);
    }

    /**
     * Unbinds the [Player] from his instances except the one he currently is in.
     */
    void UnbindAllInstances(Player* player)
    {
        for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
        {
            BoundInstancesMap const& binds = sInstanceSaveMgr->PlayerGetBoundInstances(player->GetGUID(), Difficulty(i));
            for (BoundInstancesMap::const_iterator itr = binds.begin(); itr != binds.end();)
            {
                if (itr->first != player->GetMapId())
                {
                    sInstanceSaveMgr->PlayerUnbindInstance(player->GetGUID(), itr->first, Difficulty(i), true, player);
                    itr = binds.begin();
                }
                else
                {
                    ++itr;
                }
            }
        }
    }

    /**
     * Forces the [Player] to leave a [BattleGround]
     *
     * @param bool teleToEntry = true
     */
    void LeaveBattleground(Player* player)
    {
        player->LeaveBattleground();
    }

    /**
     * Repairs [Item] at specified position.
     *
     * @param uint16 position
     * @param bool cost = true
     * @param float discountMod = 1.0
     */
    void DurabilityRepair(Player* player, uint16 position, sol::optional<bool> takeCostArg, sol::optional<float> discountModArg)
    {
        bool takeCost = takeCostArg.value_or(true);
        float discountMod = discountModArg.value_or(1.0f);

        player->DurabilityRepair(position, takeCost, discountMod, false);
    }

    /**
     * Repairs all [Item]s.
     *
     * @param bool takeCost = true
     * @param float discountMod = 1.0
     * @param bool guidBank = false
     */
    void DurabilityRepairAll(Player* player, sol::optional<bool> takeCostArg, sol::optional<float> discountModArg, sol::optional<bool> guildBankArg)
    {
        bool takeCost = takeCostArg.value_or(true);
        float discountMod = discountModArg.value_or(1.0f);
        bool guildBank = guildBankArg.value_or(false);

        player->DurabilityRepairAll(takeCost, discountMod, guildBank);
    }

    /**
     * Sets durability loss for an [Item] in the specified slot
     *
     * @param int32 slot
     */
    void DurabilityPointLossForEquipSlot(Player* player, int32 slot)
    {
        if (slot >= EQUIPMENT_SLOT_START && slot < EQUIPMENT_SLOT_END)
            player->DurabilityPointLossForEquipSlot((EquipmentSlots)slot);
    }

    /**
     * Sets durability loss on all [Item]s equipped
     *
     * If inventory is true, sets durability loss for [Item]s in bags
     *
     * @param int32 points
     * @param bool inventory = true
     */
    void DurabilityPointsLossAll(Player* player, int32 points, sol::optional<bool> inventoryArg)
    {
        bool inventory = inventoryArg.value_or(true);

        player->DurabilityPointsLossAll(points, inventory);
    }

    /**
     * Sets durability loss for the specified [Item]
     *
     * @param [Item] item
     * @param int32 points
     */
    void DurabilityPointsLoss(Player* player, Item* item, int32 points)
    {
        player->DurabilityPointsLoss(item, points);
    }

    /**
     * Damages specified [Item]
     *
     * @param [Item] item
     * @param double percent
     */
    void DurabilityLoss(Player* player, Item* item, double percent)
    {
        player->DurabilityLoss(item, percent);
    }

    /**
     * Damages all [Item]s equipped. If inventory is true, damages [Item]s in bags
     *
     * @param double percent
     * @param bool inventory = true
     */
    void DurabilityLossAll(Player* player, double percent, sol::optional<bool> inventoryArg)
    {
        bool inventory = inventoryArg.value_or(true);

        player->DurabilityLossAll(percent, inventory);
    }

    /**
     * Kills the [Player]
     */
    void KillPlayer(Player* player)
    {
        player->KillPlayer();
    }

    /**
     * Forces the [Player] to leave a [Group]
     */
    void RemoveFromGroup(Player* player)
    {
        if (!player->GetGroup())
            return;

        player->RemoveFromGroup();
    }

    /**
     * Returns the [Player]s accumulated talent reset cost
     *
     * @return uint32 resetCost
     */
    uint32 ResetTalentsCost(Player* player)
    {
        return player->resetTalentsCost();
    }

    /**
     * Resets the [Player]s talents
     *
     * @param bool noCost = true
     */
    void ResetTalents(Player* player, sol::optional<bool> noCostArg)
    {
        bool no_cost = noCostArg.value_or(true);

        player->resetTalents(no_cost);
        player->SendTalentsInfoData(false);
    }

    /**
     * Removes the [Spell] from the [Player]
     *
     * @param uint32 entry : entry of a [Spell]
     */
    void RemoveSpell(Player* player, uint32 entry)
    {
        player->removeSpell(entry, SPEC_MASK_ALL, false);
    }

    /**
     * Clears the [Player]s combo points
     */
    void ClearComboPoints(Player* player)
    {
        player->ClearComboPoints();
    }

    /**
     * Adds combo points to the [Player]
     *
     * @param [Unit] target
     * @param int8 count
     */
    void AddComboPoints(Player* player, Unit* target, int8 count)
    {
        player->AddComboPoints(target, count);
    }

    /**
     * Gives [Quest] monster talked to credit
     *
     * @param uint32 entry : entry of a [Creature]
     * @param [Creature] creature
     */
    void TalkedToCreature(Player* player, uint32 entry, Creature* creature)
    {
        player->TalkedToCreature(entry, creature->GetGUID());
    }

    /**
     * Gives [Quest] monster killed credit
     *
     * @param uint32 entry : entry of a [Creature]
     */
    void KilledMonsterCredit(Player* player, uint32 entry)
    {
        player->KilledMonsterCredit(entry, player->GetGUID());
    }

    /**
     * Completes a [Quest] if in a [Group]
     *
     * @param uint32 quest : entry of a quest
     * @param [WorldObject] obj
     */
    void GroupEventHappens(Player* player, uint32 questId, WorldObject* obj)
    {
        player->GroupEventHappens(questId, obj);
    }

    /**
     * Completes the [Quest] if a [Quest] area is explored, or completes the [Quest]
     *
     * @param uint32 quest : entry of a [Quest]
     */
    void AreaExploredOrEventHappens(Player* player, uint32 questId)
    {
        player->AreaExploredOrEventHappens(questId);
    }

    /**
     * Sets the given [Quest] entry failed for the [Player].
     *
     * @param uint32 entry : entry of a [Quest]
     */
    void FailQuest(Player* player, uint32 entry)
    {
        player->FailQuest(entry);
    }

    /**
     * Sets the given quest entry incomplete for the [Player].
     *
     * @param uint32 entry : quest entry
     */
    void IncompleteQuest(Player* player, uint32 entry)
    {
        player->IncompleteQuest(entry);
    }

    /**
     * Completes the given quest entry for the [Player] and tries to satisfy all quest requirements.
     *
     * The player should have the quest to complete it.
     *
     * @param uint32 entry : quest entry
     */
    void CompleteQuest(Player* player, uint32 entry)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        // If player doesn't have the quest
        if (!quest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
            return;

        // Add quest items for quests that require items
        for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
        {
            uint32 id = quest->RequiredItemId[x];
            uint32 count = quest->RequiredItemCount[x];

            if (!id || !count)
                continue;

            uint32 curItemCount = player->GetItemCount(id, true);

            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, id, true);
                player->SendNewItem(item, count - curItemCount, true, false);
            }
        }

        // All creature/GO slain/cast (not required, but otherwise it will display "Creature slain 0/10")
        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32 creature = quest->RequiredNpcOrGo[i];
            uint32 creatureCount = quest->RequiredNpcOrGoCount[i];

            if (creature > 0)
            {
                if (CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creature))
                    for (uint16 z = 0; z < creatureCount; ++z)
                        player->KilledMonster(creatureInfo, ObjectGuid::Empty);
            }
            else if (creature < 0)
                for (uint16 z = 0; z < creatureCount; ++z)
                    player->KillCreditGO(creature);
        }


        // If the quest requires reputation to complete
        if (uint32 repFaction = quest->GetRepObjectiveFaction())
        {
            uint32 repValue = quest->GetRepObjectiveValue();
            uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, repValue);
        }

        // If the quest requires a SECOND reputation to complete
        if (uint32 repFaction = quest->GetRepObjectiveFaction2())
        {
            uint32 repValue2 = quest->GetRepObjectiveValue2();
            uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
            if (curRep < repValue2)
                if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    player->GetReputationMgr().SetReputation(factionEntry, repValue2);
        }

        // If the quest requires money
        int32 ReqOrRewMoney = quest->GetRewOrReqMoney();
        if (ReqOrRewMoney < 0)
            player->ModifyMoney(-ReqOrRewMoney);

        player->CompleteQuest(entry);
    }

    /**
     * Tries to add the given quest entry for the [Player].
     *
     * @param uint32 entry : quest entry
     */
    void AddQuest(Player* player, uint32 entry)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            return;

        // check item starting quest (it can work incorrectly if added without item in inventory)
        ItemTemplateContainer const* itc = sObjectMgr->GetItemTemplateStore();
        for (auto const& pair : *itc)
        {
            if (pair.second.StartQuest == entry)
                return;
        }

        // ok, normal (creature/GO starting) quest
        if (player->CanAddQuest(quest, true))
            player->AddQuestAndCheckCompletion(quest, nullptr);
    }

    /**
     * Removes the given quest entry from the [Player].
     *
     * @param uint32 entry : quest entry
     */
    void RemoveQuest(Player* player, uint32 entry)
    {
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);

        if (!quest)
            return;

        // remove all quest entries for 'entry' from quest log
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuest = player->GetQuestSlotQuestId(slot);
            if (logQuest == entry)
            {
                player->SetQuestSlot(slot, 0);

                // we ignore unequippable quest items in this case, its' still be equipped
                player->TakeQuestSourceItem(logQuest, false);

                if (quest->HasFlag(QUEST_FLAGS_FLAGS_PVP))
                {
                    player->pvpInfo.IsHostile = player->pvpInfo.IsInHostileArea || player->HasPvPForcingQuest();
                    player->UpdatePvPState();
                }
            }
        }

        player->RemoveActiveQuest(entry, false);
        player->RemoveRewardedQuest(entry);
    }

    /**
     * Sends whisper text from the [Player]
     *
     * @param string text
     * @param uint32 lang : language the [Player] will speak
     * @param [Player] receiver : is the [Player] that will receive the whisper, if TrinityCore
     * @param ObjectGuid guid : is the GUID of a [Player] that will receive the whisper, not TrinityCore
     */
    void Whisper(Player* player, std::string const& text, uint32 lang, Player* receiver)
    {
        player->Whisper(text, (Language)lang, receiver);
    }

    /**
     * Sends a text emote from the [Player]
     *
     * @param string emoteText
     */
    void TextEmote(Player* player, std::string const& text)
    {
        player->TextEmote(text);
    }

    /**
     * Sends yell text from the [Player]
     *
     * @param string text : text for the [Player] to yells
     * @param uint32 lang : language the [Player] will speak
     */
    void Yell(Player* player, std::string const& text, uint32 lang)
    {
        player->Yell(text, (Language)lang);
    }

    /**
     * Sends say text from the [Player]
     *
     * @param string text : text for the [Player] to say
     * @param uint32 lang : language the [Player] will speak
     */
    void Say(Player* player, std::string const& text, uint32 lang)
    {
        player->Say(text, (Language)lang);
    }

    /**
     * Gives the [Player] experience
     *
     * @param uint32 xp : experience to give
     * @param [Unit] victim = nil
     */
    void GiveXP(Player* player, uint32 xp, sol::optional<UnitRef> victimArg)
    {
        Unit* victim = victimArg ? victimArg->Resolve() : nullptr;

        player->GiveXP(xp, victim);
    }

    /**
     * Toggle the [Player]s 'Do Not Disturb' flag
     */
    void ToggleDND(Player* player)
    {
        player->ToggleDND();
    }

    /**
     * Toggle the [Player]s 'Away From Keyboard' flag
     */
    void ToggleAFK(Player* player)
    {
        player->ToggleAFK();
    }

    /**
     * Equips the given item or item entry to the given slot. Returns the equipped item or nil.
     *
     *     enum EquipmentSlots // 19 slots
     *     {
     *         EQUIPMENT_SLOT_START        = 0,
     *         EQUIPMENT_SLOT_HEAD         = 0,
     *         EQUIPMENT_SLOT_NECK         = 1,
     *         EQUIPMENT_SLOT_SHOULDERS    = 2,
     *         EQUIPMENT_SLOT_BODY         = 3,
     *         EQUIPMENT_SLOT_CHEST        = 4,
     *         EQUIPMENT_SLOT_WAIST        = 5,
     *         EQUIPMENT_SLOT_LEGS         = 6,
     *         EQUIPMENT_SLOT_FEET         = 7,
     *         EQUIPMENT_SLOT_WRISTS       = 8,
     *         EQUIPMENT_SLOT_HANDS        = 9,
     *         EQUIPMENT_SLOT_FINGER1      = 10,
     *         EQUIPMENT_SLOT_FINGER2      = 11,
     *         EQUIPMENT_SLOT_TRINKET1     = 12,
     *         EQUIPMENT_SLOT_TRINKET2     = 13,
     *         EQUIPMENT_SLOT_BACK         = 14,
     *         EQUIPMENT_SLOT_MAINHAND     = 15,
     *         EQUIPMENT_SLOT_OFFHAND      = 16,
     *         EQUIPMENT_SLOT_RANGED       = 17,
     *         EQUIPMENT_SLOT_TABARD       = 18,
     *         EQUIPMENT_SLOT_END          = 19
     *     };
     *
     *     enum InventorySlots // 4 slots
     *     {
     *         INVENTORY_SLOT_BAG_START    = 19,
     *         INVENTORY_SLOT_BAG_END      = 23
     *     };
     *
     * @proto equippedItem = (item, slot)
     * @proto equippedItem = (entry, slot)
     * @param [Item] item : item to equip
     * @param uint32 entry : entry of the item to equip
     * @param uint32 slot : equipment slot to equip the item to The slot can be [EquipmentSlots] or [InventorySlots]
     * @return [Item] equippedItem : item or nil if equipping failed
     */
    Item* EquipItem(Player* player, sol::object itemOrEntry, uint32 slot)
    {
        uint16 dest = 0;
        Item* item = itemOrEntry.is<ItemRef>() ? itemOrEntry.as<ItemRef>().Require() : nullptr;

        if (slot >= INVENTORY_SLOT_BAG_END)
            return nullptr;

        if (!item)
        {
            uint32 entry = itemOrEntry.as<uint32>();
            item = Item::CreateItem(entry, 1, player);
            if (!item)
                return nullptr;

            InventoryResult result = player->CanEquipItem(slot, dest, item, false);
            if (result != EQUIP_ERR_OK)
            {
                delete item;
                return nullptr;
            }
            player->ItemAddedQuestCheck(entry, 1);
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM, entry, 1);
        }
        else
        {
            InventoryResult result = player->CanEquipItem(slot, dest, item, false);
            if (result != EQUIP_ERR_OK)
                return nullptr;
            player->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
        }

        Item* equippedItem = player->EquipItem(dest, item, true);
        player->AutoUnequipOffhandIfNeed();
        return equippedItem;
    }

    /**
     * Returns true if the player can equip the given [Item] or item entry to the given slot, false otherwise.
     *
     * @proto canEquip = (item, slot)
     * @proto canEquip = (entry, slot)
     * @param [Item] item : item to equip
     * @param uint32 entry : entry of the item to equip
     * @param uint32 slot : equipment slot to test
     * @return bool canEquip
     */
    bool CanEquipItem(Player* player, sol::object itemOrEntry, uint32 slot)
    {
        Item* item = itemOrEntry.is<ItemRef>() ? itemOrEntry.as<ItemRef>().Require() : nullptr;
        if (slot >= EQUIPMENT_SLOT_END)
            return false;

        if (!item)
        {
            uint32 entry = itemOrEntry.as<uint32>();
            uint16 dest;
            InventoryResult msg = player->CanEquipNewItem(slot, dest, entry, false);
            if (msg != EQUIP_ERR_OK)
                return false;
        }
        else
        {
            uint16 dest;
            InventoryResult msg = player->CanEquipItem(slot, dest, item, false);
            if (msg != EQUIP_ERR_OK)
                return false;
        }
        return true;
    }

    /**
     * Removes a title by ID from the [Player]s list of known titles
     *
     * @param uint32 titleId
     */
    void UnsetKnownTitle(Player* player, uint32 id)
    {
        CharTitlesEntry const* t = sCharTitlesStore.LookupEntry(id);
        if (t)
            player->SetTitle(t, true);
    }

    /**
     * Advances all of the [Player]s weapon skills to the maximum amount available
     */
    void AdvanceSkillsToMax(Player* player)
    {
        player->UpdateSkillsToMaxSkillsForLevel();
    }

    /**
     * Advances all of the [Player]s skills to the amount specified
     *
     * @param uint32 skillStep
     */
    void AdvanceAllSkills(Player* player, uint32 step)
    {
        if (!step)
            return;

        for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
        {
            if (SkillLineEntry const* entry = sSkillLineStore.LookupEntry(i))
            {
                if (entry->categoryId == SKILL_CATEGORY_LANGUAGES || entry->categoryId == SKILL_CATEGORY_GENERIC)
                    continue;

                if (player->HasSkill(entry->id))
                    player->UpdateSkill(entry->id, step);
            }
        }
    }

    /**
     * Updates a skill for the [Player] and advances it by the specified step.
     *
     * @param uint32 skillId : the skill to update
     * @param uint32 step : the step to advance the skill by
     * @return bool success : true if the skill was updated successfully
     */
    bool AdvanceSkill(Player* player, uint32 _skillId, uint32 _step)
    {
        bool success = false;
        if (_skillId && _step && player->HasSkill(_skillId))
        {
            success = player->UpdateSkill(_skillId, _step);
        }
        return success;
    }

    /**
     * Teleports a [Player] to the location specified
     *
     * @param uint32 mappId
     * @param float xCoord
     * @param float yCoord
     * @param float zCoord
     * @param float orientation
     */
    bool Teleport(Player* player, uint32 mapId, float x, float y, float z, float o)
    {
        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->m_taxi.ClearTaxiDestinations();
        }

        return player->TeleportTo(mapId, x, y, z, o);
    }

    /**
     * Adds a specified number of lifetime honorable kills to the [Player].
     *
     * @param [Player] player
     * @param uint32 kills
     */
    void AddLifetimeKills(Player* player, uint32 val)
    {
        uint32 currentKills = player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
        player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, currentKills + val);
    }

    /**
     * Adds the given amount of the specified item entry to the player.
     *
     * @param uint32 entry : entry of the item to add
     * @param uint32 itemCount = 1 : amount of the item to add
     * @return [Item] item : the item that was added or nil
     */
    Item* AddItem(Player* player, uint32 itemId, sol::optional<uint32> itemCountArg)
    {
        uint32 itemCount = itemCountArg.value_or(1);

        uint32 noSpaceForCount = 0;
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, itemCount, &noSpaceForCount);
        if (msg != EQUIP_ERR_OK)
            itemCount -= noSpaceForCount;

        if (itemCount == 0 || dest.empty())
            return nullptr;

        Item* item = player->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
        if (item)
            player->SendNewItem(item, itemCount, true, false);

        return item;
    }

    /**
     * Removes the given amount of the specified [Item] from the player.
     *
     * @proto (item, itemCount)
     * @proto (entry, itemCount)
     * @param [Item] item : item to remove
     * @param uint32 entry : entry of the item to remove
     * @param uint32 itemCount = 1 : amount of the item to remove
     */
    void RemoveItem(Player* player, sol::object itemOrEntry, uint32 itemCount)
    {
        Item* item = itemOrEntry.is<ItemRef>() ? itemOrEntry.as<ItemRef>().Require() : nullptr;
        if (!item)
        {
            uint32 itemId = itemOrEntry.as<uint32>();
            player->DestroyItemCount(itemId, itemCount, true);
        }
        else
        {
            player->DestroyItemCount(item, itemCount, true);
        }
    }

    /**
     * Removes specified amount of lifetime kills
     *
     * @param uint32 val : kills to remove
     */
    void RemoveLifetimeKills(Player* player, uint32 val)
    {
        uint32 currentKills = player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
        if (val > currentKills)
            val = currentKills;
        player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, currentKills - val);
    }

    /**
     * Resets cooldown of the specified spell
     *
     * @param uint32 spellId
     * @param bool update = true
     */
    void ResetSpellCooldown(Player* player, uint32 spellId, sol::optional<bool> updateArg)
    {
        bool update = updateArg.value_or(true);
        player->RemoveSpellCooldown(spellId, update);
    }

    /**
     * Resets cooldown of the specified category
     *
     * @param uint32 category
     * @param bool update = true
     */
    void ResetTypeCooldowns(Player* player, uint32 category, sol::optional<bool> updateArg)
    {
        bool update = updateArg.value_or(true);
        (void)update; // ensure that the variable is referenced in order to pass compiler checks

        player->RemoveCategoryCooldown(category);
    }

    /**
     * Resets all of the [Player]'s cooldowns
     */
    void ResetAllCooldowns(Player* player)
    {
        player->RemoveAllSpellCooldown();
    }

    /**
     * Sends a Broadcast Message to the [Player]
     *
     * @param string message
     */
    void SendBroadcastMessage(Player* player, std::string const& message)
    {
        if (message.length() > 0)
            ChatHandler(player->GetSession()).SendSysMessage(message);
    }

    /**
     * Sends an Area Trigger Message to the [Player]
     *
     * @param string message
     */
    void SendAreaTriggerMessage(Player* player, std::string const& msg)
    {
        if (msg.length() > 0)
            player->GetSession()->SendAreaTriggerMessage("{}", msg.c_str());
    }

    /**
     * Sends a Notification to the [Player]
     *
     * @param string message
     */
    void SendNotification(Player* player, std::string const& msg)
    {
        if (msg.length() > 0)
            ChatHandler(player->GetSession()).SendNotification("{}", msg);
    }

    /**
     * Sends a [WorldPacket] to the [Player]
     *
     * @param [WorldPacket] packet
     * @param bool selfOnly = true
     */
    void SendPacket(Player* player, WorldPacket* data, sol::optional<bool> selfOnlyArg)
    {
        bool selfOnly = selfOnlyArg.value_or(true);
        if (selfOnly)
            player->GetSession()->SendPacket(data);
        else
            player->SendMessageToSet(data, true);
    }

    /**
     * Sends addon message to the [Player] receiver
     *
     * @param string prefix
     * @param string message
     * @param [ChatMsg] channel
     * @param [Player] receiver
     *
     */
    void SendAddonMessage(Player* player, std::string const& prefix, std::string const& message, uint8 channel, Player* receiver)
    {
        std::string fullmsg = prefix + "\t" + message;

        WorldPacket data(SMSG_MESSAGECHAT, 100);
        data << uint8(channel);
        data << int32(LANG_ADDON);
        data << player->GetGUID();
        data << uint32(0);
        data << receiver->GetGUID();
        data << uint32(fullmsg.length() + 1);
        data << fullmsg;
        data << uint8(0);
        receiver->GetSession()->SendPacket(&data);
    }

    /**
     * Kicks the [Player] from the server
     */
    void KickPlayer(Player* player)
    {
        player->GetSession()->KickPlayer();
    }

    /**
     * Adds or subtracts from the [Player]s money in copper
     *
     * @param int32 copperAmt : negative to remove, positive to add
     */
    void ModifyMoney(Player* player, int32 amt)
    {
        player->ModifyMoney(amt);
    }

    /**
     * Teaches the [Player] the [Spell] specified by entry ID
     *
     * @param uint32 spellId
     */
    void LearnSpell(Player* player, uint32 id)
    {
        player->learnSpell(id);
    }

    /**
     * Learn the [Player] the talent specified by talent_id and talentRank
     *
     * @param uint32 talent_id
     * @param uint32 talentRank
     */
    void LearnTalent(Player* player, uint32 id, uint32 rank)
    {
        player->LearnTalent(id, rank);
        player->SendTalentsInfoData(false);
    }

    /**
    * Run a chat command as if the player typed it into the chat
    *
    * @param string command: text to display in chat or console
    */
    void RunCommand(Player* player, std::string command)
    {
        // In _ParseCommands which is used below no leading . or ! is allowed for the command string.
        if (command[0] == '.' || command[0] == '!') {
            command = command.substr(1);
        }

        auto handler = ChatHandler(player->GetSession());
        handler._ParseCommands(command);
    }

    /**
    * Adds a glyph specified by `glyphId` to the [Player]'s current talent specialization into the slot with the index `slotIndex`
    *
    * @param uint32 glyphId
    * @param uint32 slotIndex
    */
    void SetGlyph(Player* player, uint32 glyphId, uint32 slotIndex)
    {
        player->SetGlyph(slotIndex, glyphId, true);
        player->SendTalentsInfoData(false); // Also handles GlyphData
    }

    /**
    * Returns the glyph ID in the specified glyph slot of the [Player]'s current talent specialization.
    *
    * @param [uint32] slotIndex
    * @return [uint32] glyphId
    */
    uint32 GetGlyph(Player* player, uint32 slotIndex)
    {
        return player->GetGlyph(slotIndex);
    }

    /**
     * Remove cooldowns on spells that have less than 10 minutes of cooldown from the [Player], similarly to when you enter an arena.
     */
    void RemoveArenaSpellCooldowns(Player* player)
    {
        player->RemoveArenaSpellCooldowns();
    }

    /**
     * Resurrects the [Player].
     *
     * @param float healthPercent = 100.0f
     * @param bool ressSickness = false
     */
    void ResurrectPlayer(Player* player, sol::optional<float> percentArg, sol::optional<bool> sicknessArg)
    {
        float percent = percentArg.value_or(100.0f);
        bool sickness = sicknessArg.value_or(false);
        player->ResurrectPlayer(percent, sickness);
        player->SpawnCorpseBones();
    }

    /**
     * Adds a new item to the gossip menu shown to the [Player] on next call to [Player:GossipSendMenu].
     *
     * sender and intid are numbers which are passed directly to the gossip selection handler. Internally they are partly used for the database gossip handling.
     * code specifies whether to show a box to insert text to. The player inserted text is passed to the gossip selection handler.
     * money specifies an amount of money the player needs to have to click the option. An error message is shown if the player doesn't have enough money.
     * Note that the money amount is only checked client side and is not removed from the player either. You will need to check again in your code before taking action.
     *
     * See also: [Player:GossipSendMenu], [Player:GossipAddQuests], [Player:GossipComplete], [Player:GossipClearMenu]
     *
     * @param uint32 icon : number that specifies used icon
     * @param string msg : label on the gossip item
     * @param uint32 sender : number passed to gossip handlers
     * @param uint32 intid : number passed to gossip handlers
     * @param bool code = false : show text input on click if true
     * @param string popup = nil : if non empty string, a popup with given text shown on click
     * @param uint32 money = 0 : required money in copper
     */
    void GossipMenuAddItem(Player* player, uint32 _icon, std::string const& msg, uint32 _sender, uint32 _intid, sol::optional<bool> codeArg, sol::optional<std::string> promptMsgArg, sol::optional<uint32> moneyArg)
    {
        bool _code = codeArg.value_or(false);
        std::string _promptMsg = promptMsgArg.value_or("");
        uint32 _money = moneyArg.value_or(0);
        if (player->PlayerTalkClass->GetGossipMenu().GetMenuItemCount() < GOSSIP_MAX_MENU_ITEMS)
        {
            player->PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, _icon, msg, _sender, _intid, _promptMsg, _money,
                                                                 _code);
        }
        else
        {
            throw std::runtime_error("GossipMenuItem not added. Reached Max amount of possible GossipMenuItems in this GossipMenu");
        }
    }

    /**
     * Closes the [Player]s currently open Gossip Menu.
     *
     * See also: [Player:GossipMenuAddItem], [Player:GossipAddQuests], [Player:GossipSendMenu], [Player:GossipClearMenu]
     */
    void GossipComplete(Player* player)
    {
        player->PlayerTalkClass->SendCloseGossip();
    }

    /**
     * Sends the current gossip items of the player to him as a gossip menu with header text from the given textId.
     *
     * If sender is a [Player] then menu_id is mandatory, otherwise it is not used for anything.
     * menu_id is the ID used to trigger the OnGossipSelect registered for players. See [Global:RegisterPlayerGossipEvent]
     *
     * See also: [Player:GossipMenuAddItem], [Player:GossipAddQuests], [Player:GossipComplete], [Player:GossipClearMenu]
     *
     * @proto (npc_text, sender)
     * @proto (npc_text, sender, menu_id)
     * @param uint32 npc_text : entry ID of a header text in npc_text database table, common default is 100
     * @param [Object] sender : object acting as the source of the sent gossip menu
     * @param uint32 menu_id : if sender is a [Player] then menu_id is mandatory
     */
    void GossipSendMenu(Player* player, uint32 npc_text, Object* sender, sol::optional<uint32> menuIdArg)
    {
        if (sender->GetTypeId() == TYPEID_PLAYER)
        {
            if (!menuIdArg)
                throw std::invalid_argument("menu_id is mandatory when the sender is a Player");

            uint32 menu_id = *menuIdArg;
            player->PlayerTalkClass->GetGossipMenu().SetMenuId(menu_id);
        }
        player->PlayerTalkClass->SendGossipMenu(npc_text, sender->GetGUID());
    }

    /**
     * Clears the [Player]s current gossip item list.
     *
     * See also: [Player:GossipMenuAddItem], [Player:GossipSendMenu], [Player:GossipAddQuests], [Player:GossipComplete]
     *
     *     Note: This is needed when you show a gossip menu without using gossip hello or select hooks which do this automatically.
     *     Usually this is needed when using [Player] is the sender of a Gossip Menu.
     */
    void GossipClearMenu(Player* player)
    {
        player->PlayerTalkClass->ClearMenus();
    }

    /**
     * Attempts to start the taxi/flying to the given pathID
     *
     * @param uint32 pathId : pathId from DBC or [Global:AddTaxiPath]
     */
    void StartTaxi(Player* player, uint32 pathId)
    {
        player->ActivateTaxiPathTo(pathId);
    }

    /**
     * Sends POI to the location on your map
     *
     * @param float x
     * @param float y
     * @param uint32 icon : map icon to show
     * @param uint32 flags
     * @param uint32 data
     * @param string iconText
     */
    void GossipSendPOI(Player* player, float x, float y, uint32 icon, uint32 flags, uint32 data, std::string const& iconText)
    {
        WorldPacket packet(SMSG_GOSSIP_POI, 4 + 4 + 4 + 4 + 4 + 10);
        packet << flags;
        packet << x;
        packet << y;
        packet << icon;
        packet << data;
        packet << iconText;
        player->GetSession()->SendPacket(&packet);
    }

    /**
     * Adds the gossip items to the [Player]'s gossip for the quests the given [WorldObject] can offer to the player.
     *
     * @param [WorldObject] source : a questgiver with quests
     */
    void GossipAddQuests(Player* player, WorldObject* source)
    {
        if (source->GetTypeId() == TYPEID_UNIT)
        {
            if (source->GetUInt32Value(UNIT_NPC_FLAGS) & UNIT_NPC_FLAG_QUESTGIVER)
                player->PrepareQuestMenu(source->GetGUID());
        }
        else if (source->GetTypeId() == TYPEID_GAMEOBJECT)
        {
            if (source->ToGameObject()->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
                player->PrepareQuestMenu(source->GetGUID());
        }
    }

    /**
     * Shows a quest accepting window to the [Player] for the given quest.
     *
     * @param uint32 questId : entry of a quest
     * @param bool activateAccept = true : auto finish the quest
     */
    void SendQuestTemplate(Player* player, uint32 questId, sol::optional<bool> activateAcceptArg)
    {
        bool activateAccept = activateAcceptArg.value_or(true);

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            return;

        player->PlayerTalkClass->SendQuestGiverQuestDetails(quest, player->GetGUID(), activateAccept);
    }

    /**
     * Converts [Player]'s corpse to bones
     */
    void SpawnBones(Player* player)
    {
        player->SpawnCorpseBones();
    }

    /**
     * Loots [Player]'s bones for insignia
     *
     * @param [Player] looter
     */
    void RemovedInsignia(Player* player, Player* looter)
    {
        player->RemovedInsignia(looter);
    }

    /**
     * Makes the [Player] invite another player to a group.
     *
     * @param [Player] invited : player to invite to group
     * @return bool success : true if the player was invited to a group
     */
    bool GroupInvite(Player* player, Player* invited)
    {
        if (invited->GetGroup() || invited->GetGroupInvite())
            return false;

        // Get correct existing group if any
        Group* group = player->GetGroup();
        if (group && group->isBGGroup())
            group = player->GetOriginalGroup();

        bool success = false;

        // Try invite if group found
        if (group)
            success = !group->IsFull() && group->AddInvite(invited);
        else
        {
            // Create new group if one not found
            group = new Group;
            success = group->AddLeaderInvite(player) && group->AddInvite(invited);
            if (!success)
                delete group;
        }

        if (success)
        {
            WorldPacket data(SMSG_GROUP_INVITE, 10);                // guess size
            data << uint8(1);                                       // invited/already in group flag
            data << player->GetName();                              // max len 48
            data << uint32(0);                                      // unk
            data << uint8(0);                                       // count
            data << uint32(0);                                      // unk
            invited->GetSession()->SendPacket(&data);
        }

        return success;
    }

    /**
     * Creates a new [Group] with the creator [Player] as leader.
     *
     * @param [Player] invited : player to add to group
     * @return [Group] createdGroup : the created group or nil
     */
    Group* GroupCreate(Player* player, Player* invited)
    {
        if (player->GetGroup() || invited->GetGroup())
            return nullptr;

        if (player->GetGroupInvite())
            player->UninviteFromGroup();
        if (invited->GetGroupInvite())
            invited->UninviteFromGroup();

        // Try create new group
        Group* group = new Group;
        if (!group->AddLeaderInvite(player))
        {
            delete group;
            return nullptr;
        }

        // Forming a new group, create it
        if (!group->IsCreated())
        {
            group->RemoveInvite(player);
            group->Create(player);
            sGroupMgr->AddGroup(group);
        }

        if (!group->AddMember(invited))
            return nullptr;
        group->BroadcastGroupUpdate();
        return group;
    }

    /**
     * Starts a cinematic for the [Player]
     *
     * @param uint32 CinematicSequenceId : entry of a cinematic
     */
    void SendCinematicStart(Player* player, uint32 CinematicSequenceId)
    {
        player->SendCinematicStart(CinematicSequenceId);
    }

    /**
     * Starts a movie for the [Player]
     *
     * @param uint32 MovieId : entry of a movie
     */
    void SendMovieStart(Player* player, uint32 MovieId)
    {
        player->SendMovieStart(MovieId);
    }

    /**
     * Sets a setting value for the [Player]
     *
     * @param string source
     * @param uint32 index
     * @param uint32 value
     */
    void UpdatePlayerSetting(Player* player, std::string const& source, uint32 index, uint32 value)
    {
        player->UpdatePlayerSetting(source, index, value);
    }

    /**
     * Gets a setting value for the [Player]
     *
     * @param string source
     * @param uint32 index
     */
    uint32 GetPlayerSettingValue(Player* player, std::string const& source, uint32 index)
    {
        uint32 value = player->GetPlayerSetting(source, index).value;
        return value;
    }

    /**
     * Returns the [Player] that is currently trading with this [Player]
     *
     * @return [Player] trader : the player trading, or nil
     */
    Player* GetTrader(Player* player)
    {
        return player->GetTrader();
    }

    /**
     * The [Player] sets the spell power
     *
     * @param int value : The spell power value to set
     * @param bool apply = false : Whether the spell power should be applied or removed
     */
    void SetSpellPower(Player* player, int value, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(false);

        player->ApplySpellPowerBonus(value, apply);
    }

    /**
     * Set bonus talent count to a specific count for the [Player]
     *
     * @param uint32 value : bonus talent points
     */
    void SetBonusTalentCount(Player* player, uint32 value)
    {
        player->SetBonusTalentCount(value);
    }

    /**
     * Get bonus talents count from the [Player]
     *
     * @return uint32 bonusTalent
     */
    uint32 GetBonusTalentCount(Player* player)
    {
        return player->GetBonusTalentCount();
    }

    /**
     *  Returns the [Player] spells list
     *
     * @return table playerSpells
     */
    sol::table GetSpells(Player* player, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        PlayerSpellMap spellMap = player->GetSpellMap();
        for (PlayerSpellMap::const_iterator itr = spellMap.begin(); itr != spellMap.end(); ++itr)
        {
            SpellInfo const* spellInfo = sSpellMgr->AssertSpellInfo(itr->first);
            tbl[++i] = spellInfo->Id;
        }

        return tbl;
    }

    /**
     * Add bonus talents count to the [Player]
     *
     * @param uint32 count = count of bonus talent
     */
    void AddBonusTalent(Player* player, uint32 count)
    {
        player->AddBonusTalent(count);
    }

    /**
     * Remove bonus talents count to the [Player]
     *
     * @param uint32 count = count of bonus talent
     */
    void RemoveBonusTalent(Player* player, uint32 count)
    {
        player->RemoveBonusTalent(count);
    }

    /**
     *  Returns the [Player] homebind location.
     *
     *  @return table homebind : a table containing the player's homebind information:
     *      - uint32 mapId: The ID of the map where the player is bound.
     *      - float x: The X coordinate of the homebind location.
     *      - float y: The Y coordinate of the homebind location.
     *      - float z: The Z coordinate of the homebind location.
     */
    sol::table GetHomebind(Player* player, sol::this_state s)
    {
        sol::table tbl = sol::state_view(s).create_table();

        tbl["mapId"] = player->m_homebindMapId;
        tbl["x"] = player->m_homebindX;
        tbl["y"] = player->m_homebindY;
        tbl["z"] = player->m_homebindZ;

        return tbl;
    }

    /**
     *  Teleports [Player] to a predefined location based on the teleport name.
     *
     *  @param string tele : The name of the predefined teleport location.
     */
    void TeleportTo(Player* player, std::string const& tele)
    {
        GameTele const* game_tele = sObjectMgr->GetGameTele(tele);

        if (player->IsInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->m_taxi.ClearTaxiDestinations();
        }

        player->TeleportTo(game_tele->mapId, game_tele->position_x, game_tele->position_y, game_tele->position_z, game_tele->orientation);
    }

    /**
     * Returns the [Player]'s current [Pet], if any.
     *
     * @return [Pet] pet : the player's pet, or `nil` if no pet
     */
    Pet* GetPet(Player* player)
    {
        return player->GetPet();
    }

    /**
     * Returns `true` if the [Player] is at maximum level, `false` otherwise.
     *
     * @return bool isMaxLevel
     */
    bool IsMaxLevel(Player* player)
    {
        return player->IsMaxLevel();
    }

    /**
     * Summons a [Pet] at the specified location.
     *
     * @param uint32 entry : the creature entry ID to summon
     * @param float x : X coordinate
     * @param float y : Y coordinate
     * @param float z : Z coordinate
     * @param float ang : orientation angle
     * @param [PetType] petType : the type of pet to summon
     * @param uint32 duration = 0 : duration in milliseconds, 0 for permanent
     * @param uint32 healthPct = 0 : initial health percentage
     * @return [Pet] pet : the summoned pet, or `nil` if failed
     */
    Pet* SummonPet(Player* player, uint32 entry, float x, float y, float z, float ang, uint32 petType, sol::optional<uint32> durationArg, sol::optional<uint32> healthPctArg)
    {
        uint32 duration = durationArg.value_or(0);
        uint32 healthPct = healthPctArg.value_or(0);

        Pet* pet = player->SummonPet(entry, x, y, z, ang, static_cast<PetType>(petType), Milliseconds(duration), healthPct);
        return pet;
    }

    /**
     * Returns the average item level of the [Player]'s equipment.
     *
     * @return float averageItemLevel
     */
    float GetAverageItemLevel(Player* player)
    {
        return player->GetAverageItemLevel();
    }

    /**
     * Creates a tamed [Pet] from a [Creature] or creature entry.
     *
     * Can be called with either:
     * - `player:CreatePet(creatureEntry)` - creates pet from entry ID
     * - `player:CreatePet(creature, spellID)` - tames existing creature
     *
     * @param uint32 creatureEntry : creature entry ID (first form)
     * @param [Creature] creature : target creature to tame (second form)
     * @param uint32 spellID = 0 : spell used for taming (second form)
     * @return [Pet] pet : the created pet, or `nil` if failed
     */
    Pet* CreatePet(Player* player, sol::object creatureOrEntry, sol::optional<uint32> spellIDArg)
    {
        if (creatureOrEntry.is<CreatureRef>())
        {
            Creature* creatureTarget = creatureOrEntry.as<CreatureRef>().Require();
            uint32 spellID = spellIDArg.value_or(0);
            Pet* pet = player->CreatePet(creatureTarget, spellID);
            return pet;
        }

        uint32 creatureEntry = creatureOrEntry.as<uint32>();
        Pet* pet = player->CreatePet(creatureEntry);
        return pet;
    }

    /**
     * Returns `true` if the [Player] has completed the daily quest, `false` otherwise.
     *
     * @param uint32 questId
     * @return bool isDailyQuestDone
     */
    bool IsDailyQuestDone(Player* player, uint32 questId)
    {
        return player->IsDailyQuestDone(questId);
    }

    /**
     * Temporarily unsummons the [Player]'s current [Pet].
     *
     * The pet can be resummoned later. Used during teleportation, mounting, etc.
     */
    void UnsummonPetTemporarily(Player* player)
    {
        player->UnsummonPetTemporaryIfAny();
    }

    /**
     * Sets the specified player flag on the [Player].
     *
     * @param uint32 flag : the player flag to set
     */
    void SetPlayerFlag(Player* player, uint32 flag)
    {
        player->SetPlayerFlag((PlayerFlags)flag);
    }

    /**
     * Removes the specified [Pet] from the [Player].
     *
     * @param [Pet] pet : the pet to remove
     * @param [PetSaveMode] mode : how to handle pet removal
     * @param bool returnReagent = false : if `true`, returns reagents used to summon
     */
    void RemovePet(Player* player, Pet* pet, uint32 mode, sol::optional<bool> returnReagentArg)
    {
        bool returnReagent = returnReagentArg.value_or(false);
        player->RemovePet(pet, static_cast<PetSaveMode>(mode), returnReagent);
    }

    /**
     * Removes the specified player flag from the [Player].
     *
     * @param uint32 flag : the player flag to remove
     */
    void RemovePlayerFlag(Player* player, uint32 flag)
    {
        player->RemovePlayerFlag((PlayerFlags)flag);
    }

    /**
     * Returns `true` if the [Player] can resurrect their [Pet] and returns `false` otherwise.
     *
     * @return bool canResurrect
     */
    bool CanPetResurrect(Player* player)
    {
        return player->CanPetResurrect();
    }

    /**
     * Returns a random number between the specified minimum and maximum values.
     *
     * @param uint32 minimum : the minimum value
     * @param uint32 maximum : the maximum value
     * @return uint32 randomValue : a random number between min and max
     */
    uint32 DoRandomRoll(Player* player, uint32 minimum, uint32 maximum)
    {
        return player->DoRandomRoll(minimum, maximum);
    }

    /**
     * Returns `true` if the [Player] is flagged for PvP, `false` otherwise.
     *
     * @return bool isPvP
     */
    bool IsPvP(Player* player)
    {
        return player->IsPvP();
    }

    /**
     * Returns `true` if the [Player] is flagged for Free-for-all PvP, `false` otherwise.
     *
     * @return bool isFFAPvP
     */
    bool IsFFAPvP(Player* player)
    {
        return player->IsFFAPvP();
    }

    /**
     * Returns `true` if the [Player] is using the Looking for Group system, `false` otherwise.
     *
     * @return bool isUsingLfg
     */
    bool IsUsingLfg(Player* player)
    {
        return player->IsUsingLfg();
    }

    /**
     * Returns `true` if the [Player] is in a random LFG dungeon, `false` otherwise.
     *
     * @return bool inRandomLfgDungeon
     */
    bool InRandomLfgDungeon(Player* player)
    {
        return player->inRandomLfgDungeon();
    }

    /**
     * Returns `true` if the [Player] can interact with the specified quest giver, `false` otherwise.
     *
     * @param [Object] questGiver : the quest giver object
     * @return bool canInteract
     */
    bool CanInteractWithQuestGiver(Player* player, Object* questGiver)
    {
        return player->CanInteractWithQuestGiver(questGiver);
    }

    /**
     * Returns `true` if the [Player] can see the specified quest start, `false` otherwise.
     *
     * @param [Quest] quest : the quest to check
     * @return bool canSeeStartQuest
     */
    bool CanSeeStartQuest(Player* player, Quest const* quest)
    {
        return player->CanSeeStartQuest(quest);
    }

    /**
     * Returns `true` if the [Player] has a [Pet] (active or stored) and returns `false` otherwise.
     *
     * @return bool hasExistingPet
     */
    bool IsExistPet(Player* player)
    {
        return player->IsExistPet();
    }

    /**
     * Returns `true` if the [Player] can take the specified quest, `false` otherwise.
     *
     * @param [Quest] quest : the quest to check
     * @param bool msg : whether to send error messages
     * @return bool canTakeQuest
     */
    bool CanTakeQuest(Player* player, Quest const* quest, sol::optional<bool> msgArg)
    {
        bool msg = msgArg.value_or(true);
        return player->CanTakeQuest(quest, msg);
    }

    /**
     * Resets the [Player]'s pet talents.
     *
     */
    void ResetPetTalents(Player* player)
    {
        player->ResetPetTalents();
    }

    /**
     * Returns `true` if the [Player] can add the specified quest, `false` otherwise.
     *
     * @param [Quest] quest : the quest to check
     * @param bool msg : whether to send error messages
     * @return bool canAddQuest
     */
    bool CanAddQuest(Player* player, Quest const* quest, sol::optional<bool> msgArg)
    {
        bool msg = msgArg.value_or(true);
        return player->CanAddQuest(quest, msg);
    }

    /**
     * Returns the barber shop cost for the specified style changes.
     *
     * @param uint8 newhairstyle : the new hair style
     * @param uint8 newhaircolor : the new hair color
     * @param uint8 newfacialhair : the new facial hair
     * @return uint32 cost : the cost in copper
     */
    uint32 GetBarberShopCost(Player* player, uint8 newhairstyle, uint8 newhaircolor, uint8 newfacialhair)
    {
        return player->GetBarberShopCost(newhairstyle, newhaircolor, newfacialhair);
    }

    /**
     * Returns the sight range of the [Player] for the specified target.
     *
     * @param [WorldObject] target : the target to check sight range for (optional)
     * @return float sightRange
     */
    float GetSightRange(Player* player, sol::optional<WorldObjectRef> targetArg)
    {
        WorldObject* target = targetArg ? targetArg->Resolve() : nullptr;
        return player->GetSightRange(target);
    }

    /**
     * Calculates reputation gain for the [Player].
     *
     * @param uint32 source : reputation source
     * @param uint32 creatureOrQuestLevel : creature or quest level
     * @param float rep : base reputation
     * @param uint32 faction : faction ID
     * @param bool noQuestBonus : whether to skip quest bonus
     * @return float reputationGain
     */
    float CalculateReputationGain(Player* player, uint32 source, uint32 creatureOrQuestLevel, float rep, uint32 faction, sol::optional<bool> noQuestBonusArg)
    {
        bool noQuestBonus = noQuestBonusArg.value_or(false);

        return player->CalculateReputationGain((ReputationSource)source, creatureOrQuestLevel, rep, faction, noQuestBonus);
    }

    /**
     * Applies environmental damage to the [Player].
     *
     * @param uint32 type : environmental damage type
     * @param uint32 damage : damage amount
     * @return uint32 actualDamage : the actual damage dealt
     */
    uint32 EnvironmentalDamage(Player* player, uint32 type, uint32 damage)
    {
        return player->EnvironmentalDamage((EnviromentalDamage)type, damage);
    }

    /**
     * Initializes taxi nodes for the [Player]'s current level.
     */
    void InitTaxiNodesForLevel(Player* player)
    {
        player->InitTaxiNodesForLevel();
    }

    /**
     * Learns a pet talent for the specified [Pet] of the [Player].
     *
     * @param ObjectGuid petGuid : GUID of the pet to learn the talent for
     * @param uint32 talentId : ID of the talent to learn
     * @param uint32 talentRank : rank of the talent to learn
     */
    void LearnPetTalent(Player* player, ObjectGuid petGuid, uint32 talentId, uint32 talentRank)
    {
        player->LearnPetTalent(petGuid, talentId, talentRank);
    }

    /**
     * Returns `true` if the [Player] has a title by bit index, `false` otherwise.
     *
     * @param uint32 bitIndex : the title bit index to check
     * @return bool hasTitle
     */
    bool HasTitleByIndex(Player* player, uint32 bitIndex)
    {
        return player->HasTitle(bitIndex);
    }

    /**
     * Returns `true` if the [Player] is at group reward distance from the target, `false` otherwise.
     *
     * @param [WorldObject] target : the target to check distance to
     * @return bool isAtGroupRewardDistance
     */
    bool IsAtGroupRewardDistance(Player* player, WorldObject const* target)
    {
        return player->IsAtGroupRewardDistance(target);
    }

    /**
     * Returns `true` if the [Player] is at loot reward distance from the target, `false` otherwise.
     *
     * @param [WorldObject] target : the target to check distance to
     * @return bool isAtLootRewardDistance
     */
    bool IsAtLootRewardDistance(Player* player, WorldObject const* target)
    {
        return player->IsAtLootRewardDistance(target);
    }

    /**
     * Abandons a quest from the [Player]'s quest log.
     *
     * @param uint32 questId : the quest entry ID to abandon
     */
    void AbandonQuest(Player* player, uint32 questId)
    {
        player->AbandonQuest(questId);
    }

    /**
     * Returns `true` if the [Player] can tame exotic pets, and `false` otherwise.
     *
     * @return bool canTameExoticPets : `true` if the player can tame exotic pets, `false` otherwise
     */
    bool CanTameExoticPets(Player* player)
    {
        return player->CanTameExoticPets();
    }

    /**
     * Returns the [Player]'s weapon proficiency flags.
     *
     * @return uint32 proficiencyFlags : bitmask of weapon proficiencies
     */
    uint32 GetWeaponProficiency(Player* player)
    {
        return player->GetWeaponProficiency();
    }

    /**
     * Returns the temporary unsummoned pet number for the [Player].
     *
     * @return uint32 petNumber : the temporary unsummoned pet number
     */
    uint32 GetTemporaryUnsummonedPetNumber(Player* player)
    {
        return player->GetTemporaryUnsummonedPetNumber();
    }

    /**
     * Returns the [Player]'s armor proficiency flags.
     *
     * @return uint32 proficiencyFlags : bitmask of armor proficiencies
     */
    uint32 GetArmorProficiency(Player* player)
    {
        return player->GetArmorProficiency();
    }

    /**
     * Sets the temporary unsummoned pet number for the [Player].
     *
     * @param uint32 petNumber : the pet number to set
     */
    void SetTemporaryUnsummonedPetNumber(Player* player, uint32 petNumber)
    {
        player->SetTemporaryUnsummonedPetNumber(petNumber);
    }

    /**
     * Adds weapon proficiency to the [Player].
     *
     * @param uint32 flag : weapon proficiency flag to add
     */
    void AddWeaponProficiency(Player* player, uint32 flag)
    {
        player->AddWeaponProficiency(flag);
    }

    /**
     * Resummons the [Player]'s pet if it was temporarily unsummoned.
     *
     */
    void ResummonPetTemporaryUnSummonedIfAny(Player* player)
    {
        player->ResummonPetTemporaryUnSummonedIfAny();
    }

    /**
     * Adds armor proficiency to the [Player].
     *
     * @param uint32 flag : armor proficiency flag to add
     */
    void AddArmorProficiency(Player* player, uint32 flag)
    {
        player->AddArmorProficiency(flag);
    }

    /**
     * Returns `true` if the [Player] needs to temporarily unsummon their [Pet], and `false` otherwise.
     *
     *
     * @return bool isPetNeedBeTemporaryUnsummoned : `true` if the pet needs to be temporarily unsummoned, `false` otherwise
     */
    bool IsPetNeedBeTemporaryUnsummoned(Player* player)
    {
        return player->IsPetNeedBeTemporaryUnsummoned();
    }

    /**
     * Sets the [Player]'s ammo item.
     *
     * @param uint32 itemEntry : ammo item entry ID
     */
    void SetAmmo(Player* player, uint32 itemEntry)
    {
        player->SetAmmo(itemEntry);
    }

    /**
     * Removes the [Player]'s ammo.
     */
    void RemoveAmmo(Player* player)
    {
        player->RemoveAmmo();
    }

    /**
     * Returns the [Player]'s ammo DPS.
     *
     * @return float ammoDPS : damage per second from ammo
     */
    float GetAmmoDPS(Player* player)
    {
        return player->GetAmmoDPS();
    }

    /**
     * Returns `true` if the [Player] can resummon a [Pet] with the specified spell ID, and `false` otherwise.
     *
     * @param uint32 spellId : the spell ID to check
     * @return bool canResummon : `true` if the player can resummon the pet, `false` otherwise
     */
    bool CanResummonPet(Player* player, uint32 spellId)
    {
        return player->CanResummonPet(spellId);
    }

    /**
     * Returns the [Player]'s shield item.
     *
     * @return [Item] shield : the equipped shield or nil
     */
    Item* GetShield(Player* player)
    {
        return player->GetShield();
    }

    /**
     * Returns the last pet number for the [Player].
     *
     * @return uint32 petNumber : the last pet number
     */
    uint32 GetLastPetNumber(Player* player)
    {
        return player->GetLastPetNumber();
    }

    /**
     * Returns `true` if the [Player] can teleport, `false` otherwise.
     *
     * @return bool canTeleport
     */
    bool CanTeleport(Player* player)
    {
        return player->CanTeleport();
    }

    /**
     * Sets the last pet number for the [Player].
     *
     * @param uint32 petNumber : the pet number to set
     */
    void SetLastPetNumber(Player* player, uint32 petNumber)
    {
        player->SetLastPetNumber(petNumber);
    }

    /**
     * Sets whether the [Player] can teleport.
     *
     * @param bool canTeleport : true to allow teleportation, false to disallow
     */
    void SetCanTeleport(Player* player, bool canTeleport)
    {
        player->SetCanTeleport(canTeleport);
    }

    /**
     * Returns the spell ID of the [Player]'s last [Pet] summoning spell.
     *
     * @return uint32 petSpell : the pet spell ID
     */
    uint32 GetLastPetSpell(Player* player)
    {
        return player->GetLastPetSpell();
    }

    /**
     * Returns the [Player]'s runes state for Death Knights.
     *
     * @return uint32 runesState : current runes state bitmask
     */
    uint8 GetRunesState(Player* player)
    {
        return player->GetRunesState();
    }

    /**
     * Sets the spell ID of the [Player]'s last [Pet] summoning spell.
     *
     * @param uint32 petSpell : the pet spell ID to set
     */
    void SetLastPetSpell(Player* player, uint32 petSpell)
    {
        player->SetLastPetSpell(petSpell);
    }

    /**
     * Returns `true` if the [Player] is a spectator, `false` otherwise.
     *
     * @return bool isSpectator
     */
    bool IsSpectator(Player* player)
    {
        return player->IsSpectator();
    }

    /**
     * Sets the [Player] as spectator.
     *
     * @param bool isSpectator : true to set as spectator, false otherwise
     */
    void SetIsSpectator(Player* player, bool isSpectator)
    {
        player->SetIsSpectator(isSpectator);
    }

    /**
     * Returns `true` if the [Player] can see Death Knight [Pet]s, and `false` otherwise.
     *
     * @return bool canSeeDKPet
     */
    bool CanSeeDKPet(Player* player)
    {
        return player->CanSeeDKPet();
    }

    /**
     * Returns the [Player]'s current viewpoint target.
     *
     * @return [WorldObject] viewpoint : the object the player is viewing from
     */
    WorldObject* GetViewpoint(Player* player)
    {
        return player->GetViewpoint();
    }

    /**
     * Sets whether the [Player] can see Death  Knight [Pet]s.
     *
     * @param bool show : `true` to show DK pets, `false` to hide them
     */
    void SetShowDKPet(Player* player, bool show)
    {
        player->SetShowDKPet(show);
    }

    /**
     * Sets the [Player]'s viewpoint to the specified target.
     *
     * @param [WorldObject] target : the object to view from
     */
    void SetViewpoint(Player* player, WorldObject* target, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(false);
        player->SetViewpoint(target, apply);
    }

    /**
     * Toggles instant flight mode for the [Player].
     */
    void ToggleInstantFlight(Player* player)
    {
        player->ToggleInstantFlight();
    }

    /**
     * Returns the [Player]'s character creation time.
     *
     * @return uint32 creationTime : Unix timestamp of character creation
     */
    uint32 GetCreationTime(Player* player)
    {
        return static_cast<uint32>(player->GetCreationTime().count());
    }

    /**
     * Sets the [Player]'s character creation time.
     *
     * @param uint32 creationTime : Unix timestamp to set as creation time
     */
    void SetCreationTime(Player* player, uint32 creationTime)
    {
        player->SetCreationTime(Seconds(creationTime));
    }

    /**
     * Returns the [Player]'s dodge chance from agility.
     *
     * @return float dodgeChance : dodge percentage from agility stat
     */
    float GetDodgeFromAgility(Player* player)
    {
        float diminishing, nondiminishing;
        player->GetDodgeFromAgility(diminishing, nondiminishing);
        return diminishing + nondiminishing;
    }

    /**
     * Returns the [Player]'s melee critical hit chance from agility.
     *
     * @return float critChance : melee crit percentage from agility stat
     */
    float GetMeleeCritFromAgility(Player* player)
    {
        return player->GetMeleeCritFromAgility();
    }

    /**
     * Returns the [Player]'s spell critical hit chance from intellect.
     *
     * @return float critChance : spell crit percentage from intellect stat
     */
    float GetSpellCritFromIntellect(Player* player)
    {
        return player->GetSpellCritFromIntellect();
    }

    /**
     * Returns an item from the [Player]'s inventory by slot.
     *
     * @param uint32 slot : inventory slot number
     * @return [Item] item : the item in the specified slot or nil
     */
    Item* GetInventoryItem(Player* player, uint32 slot)
    {
        if (slot >= INVENTORY_SLOT_ITEM_END)
            return nullptr;

        return player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    }

    /**
     * Returns an item from the [Player]'s bank by slot.
     *
     * @param uint32 slot : bank slot number
     * @return [Item] item : the item in the specified bank slot or nil
     */
    Item* GetBankItem(Player* player, uint32 slot)
    {
        if (slot >= BANK_SLOT_ITEM_END)
            return nullptr;

        return player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot + BANK_SLOT_ITEM_START);
    }

    /**
     * Returns the [Quest] ID for the quest in the specified quest log slot.
     *
     * @param uint16 slot : quest log slot
     * @return uint32 questId : quest ID or 0 if slot is invalid
     */
    sol::optional<uint32> GetQuestSlotQuestId(Player* player, uint16 slot)
    {
        if (slot > MAX_QUEST_LOG_SIZE)
            return sol::nullopt;

        return player->GetQuestSlotQuestId(slot);
    }

    /**
     * Sets whether the [Player] can fly.
     *
     * @param bool activate = false : true to enable flying, false to disable
     */
    void SetCanFly(Player* player, sol::optional<bool> activateArg)
    {
        bool activate = activateArg.value_or(false);
        player->SetCanFly(activate);
    }

    /**
     * Applies a rating modifier to the [Player].
     *
     * @param int32 stat : combat rating type (see CombatRating enum)
     * @param float value : rating value to apply
     * @param bool apply = false : true to apply the modifier, false to remove it
     *
     *     enum CombatRating // 24 stat
     *     {
     *         CR_WEAPON_SKILL          = 0,
     *         CR_DEFENSE_SKILL         = 1,
     *         CR_DODGE                 = 2,
     *         CR_PARRY                 = 3,
     *         CR_BLOCK                 = 4,
     *         CR_HIT_MELEE             = 5,
     *         CR_HIT_RANGED            = 6,
     *         CR_HIT_SPELL             = 7,
     *         CR_CRIT_MELEE            = 8,
     *         CR_CRIT_RANGED           = 9,
     *         CR_CRIT_SPELL            = 10,
     *         CR_HIT_TAKEN_MELEE       = 11,
     *         CR_HIT_TAKEN_RANGED      = 12,
     *         CR_HIT_TAKEN_SPELL       = 13,
     *         CR_CRIT_TAKEN_MELEE      = 14,
     *         CR_CRIT_TAKEN_RANGED     = 15,
     *         CR_CRIT_TAKEN_SPELL      = 16,
     *         CR_HASTE_MELEE           = 17,
     *         CR_HASTE_RANGED          = 18,
     *         CR_HASTE_SPELL           = 19,
     *         CR_WEAPON_SKILL_MAINHAND = 20,
     *         CR_WEAPON_SKILL_OFFHAND  = 21,
     *         CR_WEAPON_SKILL_RANGED   = 22,
     *         CR_EXPERTISE             = 23,
     *         CR_ARMOR_PENETRATION     = 24
     *     };
     *
     */

    void ApplyRatingMod(Player* player, int32 stat, float value, sol::optional<bool> applyArg)
    {
        bool apply = applyArg.value_or(false);

        player->ApplyRatingMod(CombatRating(stat), value, apply);
    }

    /**
     * Returns `true` if the [Player] knows the given taxi node, `false` otherwise.
     *
     * @param uint32 nodeId
     * @return bool known
     */
    bool HasKnownTaxiNode(Player* player, uint32 nodeId)
    {
        if (nodeId == 0)
            return false;

        return player->m_taxi.IsTaximaskNodeKnown(nodeId);
    }

    /**
     * Returns `true` if the [Player] is a Playerbot/RNDBot, `false` otherwise.
     *
     * @return bool isBot
     */
    bool IsBot(Player* player)
    {
    #if defined(MOD_PLAYERBOTS)
        return player->GetSession()->IsBot();
    #else
        (void)player;
        return false;
    #endif
    }

    /**
     * Returns the [Player]s spent talent points in each talent tree for the active spec
     *
     * @return uint8 tree1, uint8 tree2, uint8 tree3
     */
    std::tuple<uint8, uint8, uint8> GetTalentTreePoints(Player* player)
    {
        uint8 specPoints[3] = {0, 0, 0};
        player->GetTalentTreePoints(specPoints);
        return std::tuple<uint8, uint8, uint8>(specPoints[0], specPoints[1], specPoints[2]);
    }

    /**
     * Returns the index of the talent tree the [Player] has spent the most points in for the active spec
     *
     * @return uint8 treeIndex
     */
    uint8 GetMostPointsTalentTree(Player* player)
    {
        return player->GetMostPointsTalentTree();
    }
}
void RegisterPlayerMethods(sol::state& lua)
{
    sol::usertype<PlayerRef> type = ALEBind::NewHandleType<PlayerRef, UnitRef, WorldObjectRef, ObjectRef>(lua, "Player");

    type["GetInventoryFreeSlots"              ] = ALEBind::Method(&LuaPlayer::GetInventoryFreeSlots);
    type["GetBankFreeSlots"                   ] = ALEBind::Method(&LuaPlayer::GetBankFreeSlots);
    type["GetSelection"                       ] = ALEBind::Method(&LuaPlayer::GetSelection);
    type["GetGMRank"                          ] = ALEBind::Method(&LuaPlayer::GetGMRank);
    type["GetGuildId"                         ] = ALEBind::Method(&LuaPlayer::GetGuildId);
    type["GetCoinage"                         ] = ALEBind::Method(&LuaPlayer::GetCoinage);
    type["GetTeam"                            ] = ALEBind::Method(&LuaPlayer::GetTeam);
    type["GetItemCount"                       ] = ALEBind::Method(&LuaPlayer::GetItemCount);
    type["GetGroup"                           ] = ALEBind::Method(&LuaPlayer::GetGroup);
    type["GetGuild"                           ] = ALEBind::Method(&LuaPlayer::GetGuild);
    type["GetAccountId"                       ] = ALEBind::Method(&LuaPlayer::GetAccountId);
    type["GetAccountName"                     ] = ALEBind::Method(&LuaPlayer::GetAccountName);
    type["GetCompletedQuestsCount"            ] = ALEBind::Method(&LuaPlayer::GetCompletedQuestsCount);
    type["GetArenaPoints"                     ] = ALEBind::Method(&LuaPlayer::GetArenaPoints);
    type["GetHonorPoints"                     ] = ALEBind::Method(&LuaPlayer::GetHonorPoints);
    type["GetTodayHonorPoints"                ] = ALEBind::Method(&LuaPlayer::GetTodayHonorPoints);
    type["GetYesterdayHonorPoints"            ] = ALEBind::Method(&LuaPlayer::GetYesterdayHonorPoints);
    type["GetLifetimeKills"                   ] = ALEBind::Method(&LuaPlayer::GetLifetimeKills);
    type["GetTodayKills"                      ] = ALEBind::Method(&LuaPlayer::GetTodayKills);
    type["GetYesterdayKills"                  ] = ALEBind::Method(&LuaPlayer::GetYesterdayKills);
    type["GetPlayerIP"                        ] = ALEBind::Method(&LuaPlayer::GetPlayerIP);
    type["GetLevelPlayedTime"                 ] = ALEBind::Method(&LuaPlayer::GetLevelPlayedTime);
    type["GetTotalPlayedTime"                 ] = ALEBind::Method(&LuaPlayer::GetTotalPlayedTime);
    type["GetItemByPos"                       ] = ALEBind::Method(&LuaPlayer::GetItemByPos);
    type["GetItemByEntry"                     ] = ALEBind::Method(&LuaPlayer::GetItemByEntry);
    type["GetItemByGUID"                      ] = ALEBind::Method(&LuaPlayer::GetItemByGUID);
    type["GetMailCount"                       ] = ALEBind::Method(&LuaPlayer::GetMailCount);
    type["GetMailItem"                        ] = ALEBind::Method(&LuaPlayer::GetMailItem);
    type["GetReputation"                      ] = ALEBind::Method(&LuaPlayer::GetReputation);
    type["GetEquippedItemBySlot"              ] = ALEBind::Method(&LuaPlayer::GetEquippedItemBySlot);
    type["GetQuestLevel"                      ] = ALEBind::Method(&LuaPlayer::GetQuestLevel);
    type["GetChatTag"                         ] = ALEBind::Method(&LuaPlayer::GetChatTag);
    type["GetRestBonus"                       ] = ALEBind::Method(&LuaPlayer::GetRestBonus);
    type["GetPhaseMaskForSpawn"               ] = ALEBind::Method(&LuaPlayer::GetPhaseMaskForSpawn);
    type["GetAchievementPoints"               ] = ALEBind::Method(&LuaPlayer::GetAchievementPoints);
    type["GetCompletedAchievementsCount"      ] = ALEBind::Method(&LuaPlayer::GetCompletedAchievementsCount);
    type["GetReqKillOrCastCurrentCount"       ] = ALEBind::Method(&LuaPlayer::GetReqKillOrCastCurrentCount);
    type["GetQuestStatus"                     ] = ALEBind::Method(&LuaPlayer::GetQuestStatus);
    type["GetInGameTime"                      ] = ALEBind::Method(&LuaPlayer::GetInGameTime);
    type["GetComboPoints"                     ] = ALEBind::Method(&LuaPlayer::GetComboPoints);
    type["GetComboTarget"                     ] = ALEBind::Method(&LuaPlayer::GetComboTarget);
    type["GetGuildName"                       ] = ALEBind::Method(&LuaPlayer::GetGuildName);
    type["GetFreeTalentPoints"                ] = ALEBind::Method(&LuaPlayer::GetFreeTalentPoints);
    type["GetActiveSpec"                      ] = ALEBind::Method(&LuaPlayer::GetActiveSpec);
    type["GetSpecsCount"                      ] = ALEBind::Method(&LuaPlayer::GetSpecsCount);
    type["GetSpellCooldownDelay"              ] = ALEBind::Method(&LuaPlayer::GetSpellCooldownDelay);
    type["GetGuildRank"                       ] = ALEBind::Method(&LuaPlayer::GetGuildRank);
    type["GetDifficulty"                      ] = ALEBind::Method(&LuaPlayer::GetDifficulty);
    type["GetHealthBonusFromStamina"          ] = ALEBind::Method(&LuaPlayer::GetHealthBonusFromStamina);
    type["GetManaBonusFromIntellect"          ] = ALEBind::Method(&LuaPlayer::GetManaBonusFromIntellect);
    type["GetMaxSkillValue"                   ] = ALEBind::Method(&LuaPlayer::GetMaxSkillValue);
    type["GetPureMaxSkillValue"               ] = ALEBind::Method(&LuaPlayer::GetPureMaxSkillValue);
    type["GetSkillValue"                      ] = ALEBind::Method(&LuaPlayer::GetSkillValue);
    type["GetBaseSkillValue"                  ] = ALEBind::Method(&LuaPlayer::GetBaseSkillValue);
    type["GetPureSkillValue"                  ] = ALEBind::Method(&LuaPlayer::GetPureSkillValue);
    type["GetSkillPermBonusValue"             ] = ALEBind::Method(&LuaPlayer::GetSkillPermBonusValue);
    type["GetSkillTempBonusValue"             ] = ALEBind::Method(&LuaPlayer::GetSkillTempBonusValue);
    type["GetReputationRank"                  ] = ALEBind::Method(&LuaPlayer::GetReputationRank);
    type["GetDrunkValue"                      ] = ALEBind::Method(&LuaPlayer::GetDrunkValue);
    type["GetBattlegroundId"                  ] = ALEBind::Method(&LuaPlayer::GetBattlegroundId);
    type["GetBattlegroundTypeId"              ] = ALEBind::Method(&LuaPlayer::GetBattlegroundTypeId);
    type["GetXP"                              ] = ALEBind::Method(&LuaPlayer::GetXP);
    type["GetXPRestBonus"                     ] = ALEBind::Method(&LuaPlayer::GetXPRestBonus);
    type["GetGroupInvite"                     ] = ALEBind::Method(&LuaPlayer::GetGroupInvite);
    type["GetSubGroup"                        ] = ALEBind::Method(&LuaPlayer::GetSubGroup);
    type["GetNextRandomRaidMember"            ] = ALEBind::Method(&LuaPlayer::GetNextRandomRaidMember);
    type["GetOriginalGroup"                   ] = ALEBind::Method(&LuaPlayer::GetOriginalGroup);
    type["GetOriginalSubGroup"                ] = ALEBind::Method(&LuaPlayer::GetOriginalSubGroup);
    type["GetChampioningFaction"              ] = ALEBind::Method(&LuaPlayer::GetChampioningFaction);
    type["GetLatency"                         ] = ALEBind::Method(&LuaPlayer::GetLatency);
    type["GetDbLocaleIndex"                   ] = ALEBind::Method(&LuaPlayer::GetDbLocaleIndex);
    type["GetDbcLocale"                       ] = ALEBind::Method(&LuaPlayer::GetDbcLocale);
    type["GetCorpse"                          ] = ALEBind::Method(&LuaPlayer::GetCorpse);
    type["GetGossipTextId"                    ] = ALEBind::Method(&LuaPlayer::GetGossipTextId);
    type["GetQuestRewardStatus"               ] = ALEBind::Method(&LuaPlayer::GetQuestRewardStatus);
    type["GetShieldBlockValue"                ] = ALEBind::Method(&LuaPlayer::GetShieldBlockValue);
    type["GetPlayerSettingValue"              ] = ALEBind::Method(&LuaPlayer::GetPlayerSettingValue);
    type["GetTrader"                          ] = ALEBind::Method(&LuaPlayer::GetTrader);
    type["GetBonusTalentCount"                ] = ALEBind::Method(&LuaPlayer::GetBonusTalentCount);
    type["GetKnownTaxiNodes"                  ] = ALEBind::Method(&LuaPlayer::GetKnownTaxiNodes);
    type["GetPet"                             ] = ALEBind::Method(&LuaPlayer::GetPet);
    type["GetTemporaryUnsummonedPetNumber"    ] = ALEBind::Method(&LuaPlayer::GetTemporaryUnsummonedPetNumber);
    type["GetLastPetNumber"                   ] = ALEBind::Method(&LuaPlayer::GetLastPetNumber);
    type["GetLastPetSpell"                    ] = ALEBind::Method(&LuaPlayer::GetLastPetSpell);
    type["GetQuestSlotQuestId"                ] = ALEBind::Method(&LuaPlayer::GetQuestSlotQuestId);
    type["GetTalentTreePoints"                ] = ALEBind::Method(&LuaPlayer::GetTalentTreePoints);
    type["GetMostPointsTalentTree"            ] = ALEBind::Method(&LuaPlayer::GetMostPointsTalentTree);
    type["SetTemporaryUnsummonedPetNumber"    ] = ALEBind::Method(&LuaPlayer::SetTemporaryUnsummonedPetNumber);
    type["SetLastPetNumber"                   ] = ALEBind::Method(&LuaPlayer::SetLastPetNumber);
    type["SetLastPetSpell"                    ] = ALEBind::Method(&LuaPlayer::SetLastPetSpell);
    type["SetShowDKPet"                       ] = ALEBind::Method(&LuaPlayer::SetShowDKPet);
    type["AdvanceSkillsToMax"                 ] = ALEBind::Method(&LuaPlayer::AdvanceSkillsToMax);
    type["AdvanceSkill"                       ] = ALEBind::Method(&LuaPlayer::AdvanceSkill);
    type["AdvanceAllSkills"                   ] = ALEBind::Method(&LuaPlayer::AdvanceAllSkills);
    type["AddLifetimeKills"                   ] = ALEBind::Method(&LuaPlayer::AddLifetimeKills);
    type["SetCoinage"                         ] = ALEBind::Method(&LuaPlayer::SetCoinage);
    type["SetKnownTitle"                      ] = ALEBind::Method(&LuaPlayer::SetKnownTitle);
    type["UnsetKnownTitle"                    ] = ALEBind::Method(&LuaPlayer::UnsetKnownTitle);
    type["SetBindPoint"                       ] = ALEBind::Method(&LuaPlayer::SetBindPoint);
    type["SetArenaPoints"                     ] = ALEBind::Method(&LuaPlayer::SetArenaPoints);
    type["SetHonorPoints"                     ] = ALEBind::Method(&LuaPlayer::SetHonorPoints);
    type["SetSpellPower"                      ] = ALEBind::Method(&LuaPlayer::SetSpellPower);
    type["SetLifetimeKills"                   ] = ALEBind::Method(&LuaPlayer::SetLifetimeKills);
    type["SetGameMaster"                      ] = ALEBind::Method(&LuaPlayer::SetGameMaster);
    type["SetGMChat"                          ] = ALEBind::Method(&LuaPlayer::SetGMChat);
    type["SetKnownTaxiNodes"                  ] = ALEBind::Method(&LuaPlayer::SetKnownTaxiNodes);
    type["SetTaxiCheat"                       ] = ALEBind::Method(&LuaPlayer::SetTaxiCheat);
    type["SetGMVisible"                       ] = ALEBind::Method(&LuaPlayer::SetGMVisible);
    type["SetPvPDeath"                        ] = ALEBind::Method(&LuaPlayer::SetPvPDeath);
    type["SetAcceptWhispers"                  ] = ALEBind::Method(&LuaPlayer::SetAcceptWhispers);
    type["SetRestBonus"                       ] = ALEBind::Method(&LuaPlayer::SetRestBonus);
    type["SetQuestStatus"                     ] = ALEBind::Method(&LuaPlayer::SetQuestStatus);
    type["SetReputation"                      ] = ALEBind::Method(&LuaPlayer::SetReputation);
    type["SetFreeTalentPoints"                ] = ALEBind::Method(&LuaPlayer::SetFreeTalentPoints);
    type["SetGuildRank"                       ] = ALEBind::Method(&LuaPlayer::SetGuildRank);
    type["SetSkill"                           ] = ALEBind::Method(&LuaPlayer::SetSkill);
    type["SetFactionForRace"                  ] = ALEBind::Method(&LuaPlayer::SetFactionForRace);
    type["SetDrunkValue"                      ] = ALEBind::Method(&LuaPlayer::SetDrunkValue);
    type["SetAtLoginFlag"                     ] = ALEBind::Method(&LuaPlayer::SetAtLoginFlag);
    type["SetPlayerLock"                      ] = ALEBind::Method(&LuaPlayer::SetPlayerLock);
    type["SetGender"                          ] = ALEBind::Method(&LuaPlayer::SetGender);
    type["SetSheath"                          ] = ALEBind::Method(&LuaPlayer::SetSheath);
    type["SetBonusTalentCount"                ] = ALEBind::Method(&LuaPlayer::SetBonusTalentCount);
    type["AddBonusTalent"                     ] = ALEBind::Method(&LuaPlayer::AddBonusTalent);
    type["RemoveBonusTalent"                  ] = ALEBind::Method(&LuaPlayer::RemoveBonusTalent);
    type["GetHomebind"                        ] = ALEBind::Method(&LuaPlayer::GetHomebind);
    type["GetSpells"                          ] = ALEBind::Method(&LuaPlayer::GetSpells);
    type["GetAverageItemLevel"                ] = ALEBind::Method(&LuaPlayer::GetAverageItemLevel);
    type["GetBarberShopCost"                  ] = ALEBind::Method(&LuaPlayer::GetBarberShopCost);
    type["GetSightRange"                      ] = ALEBind::Method(&LuaPlayer::GetSightRange);
    type["GetWeaponProficiency"               ] = ALEBind::Method(&LuaPlayer::GetWeaponProficiency);
    type["GetArmorProficiency"                ] = ALEBind::Method(&LuaPlayer::GetArmorProficiency);
    type["GetAmmoDPS"                         ] = ALEBind::Method(&LuaPlayer::GetAmmoDPS);
    type["GetShield"                          ] = ALEBind::Method(&LuaPlayer::GetShield);
    type["GetRunesState"                      ] = ALEBind::Method(&LuaPlayer::GetRunesState);
    type["GetViewpoint"                       ] = ALEBind::Method(&LuaPlayer::GetViewpoint);
    type["GetDodgeFromAgility"                ] = ALEBind::Method(&LuaPlayer::GetDodgeFromAgility);
    type["GetMeleeCritFromAgility"            ] = ALEBind::Method(&LuaPlayer::GetMeleeCritFromAgility);
    type["GetSpellCritFromIntellect"          ] = ALEBind::Method(&LuaPlayer::GetSpellCritFromIntellect);
    type["GetInventoryItem"                   ] = ALEBind::Method(&LuaPlayer::GetInventoryItem);
    type["GetBankItem"                        ] = ALEBind::Method(&LuaPlayer::GetBankItem);
    type["GetCreationTime"                    ] = ALEBind::Method(&LuaPlayer::GetCreationTime);
    type["SetCanFly"                          ] = ALEBind::Method(&LuaPlayer::SetCanFly);
    type["HasTankSpec"                        ] = ALEBind::Method(&LuaPlayer::HasTankSpec);
    type["HasMeleeSpec"                       ] = ALEBind::Method(&LuaPlayer::HasMeleeSpec);
    type["HasCasterSpec"                      ] = ALEBind::Method(&LuaPlayer::HasCasterSpec);
    type["HasHealSpec"                        ] = ALEBind::Method(&LuaPlayer::HasHealSpec);
    type["IsInGroup"                          ] = ALEBind::Method(&LuaPlayer::IsInGroup);
    type["IsInGuild"                          ] = ALEBind::Method(&LuaPlayer::IsInGuild);
    type["IsGM"                               ] = ALEBind::Method(&LuaPlayer::IsGM);
    type["IsImmuneToDamage"                   ] = ALEBind::Method(&LuaPlayer::IsImmuneToDamage);
    type["IsAlliance"                         ] = ALEBind::Method(&LuaPlayer::IsAlliance);
    type["IsHorde"                            ] = ALEBind::Method(&LuaPlayer::IsHorde);
    type["HasTitle"                           ] = ALEBind::Method(&LuaPlayer::HasTitle);
    type["HasItem"                            ] = ALEBind::Method(&LuaPlayer::HasItem);
    type["Teleport"                           ] = ALEBind::Method(&LuaPlayer::Teleport);
    type["AddItem"                            ] = ALEBind::Method(&LuaPlayer::AddItem);
    type["IsInArenaTeam"                      ] = ALEBind::Method(&LuaPlayer::IsInArenaTeam);
    type["CanRewardQuest"                     ] = ALEBind::Method(&LuaPlayer::CanRewardQuest);
    type["CanCompleteRepeatableQuest"         ] = ALEBind::Method(&LuaPlayer::CanCompleteRepeatableQuest);
    type["CanCompleteQuest"                   ] = ALEBind::Method(&LuaPlayer::CanCompleteQuest);
    type["CanEquipItem"                       ] = ALEBind::Method(&LuaPlayer::CanEquipItem);
    type["IsFalling"                          ] = ALEBind::Method(&LuaPlayer::IsFalling);
    type["ToggleAFK"                          ] = ALEBind::Method(&LuaPlayer::ToggleAFK);
    type["ToggleDND"                          ] = ALEBind::Method(&LuaPlayer::ToggleDND);
    type["IsAFK"                              ] = ALEBind::Method(&LuaPlayer::IsAFK);
    type["IsDND"                              ] = ALEBind::Method(&LuaPlayer::IsDND);
    type["IsAcceptingWhispers"                ] = ALEBind::Method(&LuaPlayer::IsAcceptingWhispers);
    type["IsGMChat"                           ] = ALEBind::Method(&LuaPlayer::IsGMChat);
    type["IsTaxiCheater"                      ] = ALEBind::Method(&LuaPlayer::IsTaxiCheater);
    type["IsGMVisible"                        ] = ALEBind::Method(&LuaPlayer::IsGMVisible);
    type["HasQuest"                           ] = ALEBind::Method(&LuaPlayer::HasQuest);
    type["InBattlegroundQueue"                ] = ALEBind::Method(&LuaPlayer::InBattlegroundQueue);
    type["CanSpeak"                           ] = ALEBind::Method(&LuaPlayer::CanSpeak);
    type["HasAtLoginFlag"                     ] = ALEBind::Method(&LuaPlayer::HasAtLoginFlag);
    type["HasAchieved"                        ] = ALEBind::Method(&LuaPlayer::HasAchieved);
    type["GetAchievementCriteriaProgress"     ] = ALEBind::Method(&LuaPlayer::GetAchievementCriteriaProgress);
    type["SetAchievement"                     ] = ALEBind::Method(&LuaPlayer::SetAchievement);
    type["CanUninviteFromGroup"               ] = ALEBind::Method(&LuaPlayer::CanUninviteFromGroup);
    type["IsRested"                           ] = ALEBind::Method(&LuaPlayer::IsRested);
    type["IsVisibleForPlayer"                 ] = ALEBind::Method(&LuaPlayer::IsVisibleForPlayer);
    type["HasQuestForItem"                    ] = ALEBind::Method(&LuaPlayer::HasQuestForItem);
    type["HasQuestForGO"                      ] = ALEBind::Method(&LuaPlayer::HasQuestForGO);
    type["CanShareQuest"                      ] = ALEBind::Method(&LuaPlayer::CanShareQuest);
    type["HasTalent"                          ] = ALEBind::Method(&LuaPlayer::HasTalent);
    type["IsInSameGroupWith"                  ] = ALEBind::Method(&LuaPlayer::IsInSameGroupWith);
    type["IsInSameRaidWith"                   ] = ALEBind::Method(&LuaPlayer::IsInSameRaidWith);
    type["IsGroupVisibleFor"                  ] = ALEBind::Method(&LuaPlayer::IsGroupVisibleFor);
    type["HasSkill"                           ] = ALEBind::Method(&LuaPlayer::HasSkill);
    type["IsHonorOrXPTarget"                  ] = ALEBind::Method(&LuaPlayer::IsHonorOrXPTarget);
    type["CanParry"                           ] = ALEBind::Method(&LuaPlayer::CanParry);
    type["CanBlock"                           ] = ALEBind::Method(&LuaPlayer::CanBlock);
    type["CanTitanGrip"                       ] = ALEBind::Method(&LuaPlayer::CanTitanGrip);
    type["InBattleground"                     ] = ALEBind::Method(&LuaPlayer::InBattleground);
    type["InArena"                            ] = ALEBind::Method(&LuaPlayer::InArena);
    type["CanUseItem"                         ] = ALEBind::Method(&LuaPlayer::CanUseItem);
    type["HasSpell"                           ] = ALEBind::Method(&LuaPlayer::HasSpell);
    type["HasSpellCooldown"                   ] = ALEBind::Method(&LuaPlayer::HasSpellCooldown);
    type["IsInWater"                          ] = ALEBind::Method(&LuaPlayer::IsInWater);
    type["CanFly"                             ] = ALEBind::Method(&LuaPlayer::CanFly);
    type["IsMoving"                           ] = ALEBind::Method(&LuaPlayer::IsMoving);
    type["IsFlying"                           ] = ALEBind::Method(&LuaPlayer::IsFlying);
    type["CanPetResurrect"                    ] = ALEBind::Method(&LuaPlayer::CanPetResurrect);
    type["IsExistPet"                         ] = ALEBind::Method(&LuaPlayer::IsExistPet);
    type["CanTameExoticPets"                  ] = ALEBind::Method(&LuaPlayer::CanTameExoticPets);
    type["IsPetNeedBeTemporaryUnsummoned"     ] = ALEBind::Method(&LuaPlayer::IsPetNeedBeTemporaryUnsummoned);
    type["CanResummonPet"                     ] = ALEBind::Method(&LuaPlayer::CanResummonPet);
    type["CanSeeDKPet"                        ] = ALEBind::Method(&LuaPlayer::CanSeeDKPet);
    type["IsMaxLevel"                         ] = ALEBind::Method(&LuaPlayer::IsMaxLevel);
    type["IsDailyQuestDone"                   ] = ALEBind::Method(&LuaPlayer::IsDailyQuestDone);
    type["IsPvP"                              ] = ALEBind::Method(&LuaPlayer::IsPvP);
    type["IsFFAPvP"                           ] = ALEBind::Method(&LuaPlayer::IsFFAPvP);
    type["IsUsingLfg"                         ] = ALEBind::Method(&LuaPlayer::IsUsingLfg);
    type["InRandomLfgDungeon"                 ] = ALEBind::Method(&LuaPlayer::InRandomLfgDungeon);
    type["CanInteractWithQuestGiver"          ] = ALEBind::Method(&LuaPlayer::CanInteractWithQuestGiver);
    type["CanSeeStartQuest"                   ] = ALEBind::Method(&LuaPlayer::CanSeeStartQuest);
    type["CanTakeQuest"                       ] = ALEBind::Method(&LuaPlayer::CanTakeQuest);
    type["CanAddQuest"                        ] = ALEBind::Method(&LuaPlayer::CanAddQuest);
    type["CalculateReputationGain"            ] = ALEBind::Method(&LuaPlayer::CalculateReputationGain);
    type["HasTitleByIndex"                    ] = ALEBind::Method(&LuaPlayer::HasTitleByIndex);
    type["IsAtGroupRewardDistance"            ] = ALEBind::Method(&LuaPlayer::IsAtGroupRewardDistance);
    type["IsAtLootRewardDistance"             ] = ALEBind::Method(&LuaPlayer::IsAtLootRewardDistance);
    type["CanTeleport"                        ] = ALEBind::Method(&LuaPlayer::CanTeleport);
    type["IsSpectator"                        ] = ALEBind::Method(&LuaPlayer::IsSpectator);
    type["HasKnownTaxiNode"                   ] = ALEBind::Method(&LuaPlayer::HasKnownTaxiNode);
    type["IsBot"                              ] = ALEBind::Method(&LuaPlayer::IsBot);
    type["GossipMenuAddItem"                  ] = ALEBind::Method(&LuaPlayer::GossipMenuAddItem);
    type["GossipSendMenu"                     ] = ALEBind::Method(&LuaPlayer::GossipSendMenu);
    type["GossipComplete"                     ] = ALEBind::Method(&LuaPlayer::GossipComplete);
    type["GossipClearMenu"                    ] = ALEBind::Method(&LuaPlayer::GossipClearMenu);
    type["SendBroadcastMessage"               ] = ALEBind::Method(&LuaPlayer::SendBroadcastMessage);
    type["SendAreaTriggerMessage"             ] = ALEBind::Method(&LuaPlayer::SendAreaTriggerMessage);
    type["SendNotification"                   ] = ALEBind::Method(&LuaPlayer::SendNotification);
    type["SendPacket"                         ] = ALEBind::Method(&LuaPlayer::SendPacket);
    type["SendAddonMessage"                   ] = ALEBind::Method(&LuaPlayer::SendAddonMessage);
    type["ModifyMoney"                        ] = ALEBind::Method(&LuaPlayer::ModifyMoney);
    type["LearnSpell"                         ] = ALEBind::Method(&LuaPlayer::LearnSpell);
    type["LearnTalent"                        ] = ALEBind::Method(&LuaPlayer::LearnTalent);
    type["RunCommand"                         ] = ALEBind::Method(&LuaPlayer::RunCommand);
    type["SetGlyph"                           ] = ALEBind::Method(&LuaPlayer::SetGlyph);
    type["GetGlyph"                           ] = ALEBind::Method(&LuaPlayer::GetGlyph);
    type["RemoveArenaSpellCooldowns"          ] = ALEBind::Method(&LuaPlayer::RemoveArenaSpellCooldowns);
    type["RemoveItem"                         ] = ALEBind::Method(&LuaPlayer::RemoveItem);
    type["RemoveLifetimeKills"                ] = ALEBind::Method(&LuaPlayer::RemoveLifetimeKills);
    type["ResurrectPlayer"                    ] = ALEBind::Method(&LuaPlayer::ResurrectPlayer);
    type["EquipItem"                          ] = ALEBind::Method(&LuaPlayer::EquipItem);
    type["ResetSpellCooldown"                 ] = ALEBind::Method(&LuaPlayer::ResetSpellCooldown);
    type["ResetTypeCooldowns"                 ] = ALEBind::Method(&LuaPlayer::ResetTypeCooldowns);
    type["ResetAllCooldowns"                  ] = ALEBind::Method(&LuaPlayer::ResetAllCooldowns);
    type["GiveXP"                             ] = ALEBind::Method(&LuaPlayer::GiveXP);
    type["Say"                                ] = ALEBind::Method(&LuaPlayer::Say);
    type["Yell"                               ] = ALEBind::Method(&LuaPlayer::Yell);
    type["TextEmote"                          ] = ALEBind::Method(&LuaPlayer::TextEmote);
    type["Whisper"                            ] = ALEBind::Method(&LuaPlayer::Whisper);
    type["CompleteQuest"                      ] = ALEBind::Method(&LuaPlayer::CompleteQuest);
    type["IncompleteQuest"                    ] = ALEBind::Method(&LuaPlayer::IncompleteQuest);
    type["FailQuest"                          ] = ALEBind::Method(&LuaPlayer::FailQuest);
    type["AddQuest"                           ] = ALEBind::Method(&LuaPlayer::AddQuest);
    type["RemoveQuest"                        ] = ALEBind::Method(&LuaPlayer::RemoveQuest);
    type["AreaExploredOrEventHappens"         ] = ALEBind::Method(&LuaPlayer::AreaExploredOrEventHappens);
    type["GroupEventHappens"                  ] = ALEBind::Method(&LuaPlayer::GroupEventHappens);
    type["KilledMonsterCredit"                ] = ALEBind::Method(&LuaPlayer::KilledMonsterCredit);
    type["TalkedToCreature"                   ] = ALEBind::Method(&LuaPlayer::TalkedToCreature);
    type["ResetPetTalents"                    ] = ALEBind::Method(&LuaPlayer::ResetPetTalents);
    type["AddComboPoints"                     ] = ALEBind::Method(&LuaPlayer::AddComboPoints);
    type["ClearComboPoints"                   ] = ALEBind::Method(&LuaPlayer::ClearComboPoints);
    type["RemoveSpell"                        ] = ALEBind::Method(&LuaPlayer::RemoveSpell);
    type["ResetTalents"                       ] = ALEBind::Method(&LuaPlayer::ResetTalents);
    type["ResetTalentsCost"                   ] = ALEBind::Method(&LuaPlayer::ResetTalentsCost);
    type["RemoveFromGroup"                    ] = ALEBind::Method(&LuaPlayer::RemoveFromGroup);
    type["KillPlayer"                         ] = ALEBind::Method(&LuaPlayer::KillPlayer);
    type["DurabilityLossAll"                  ] = ALEBind::Method(&LuaPlayer::DurabilityLossAll);
    type["DurabilityLoss"                     ] = ALEBind::Method(&LuaPlayer::DurabilityLoss);
    type["DurabilityPointsLoss"               ] = ALEBind::Method(&LuaPlayer::DurabilityPointsLoss);
    type["DurabilityPointsLossAll"            ] = ALEBind::Method(&LuaPlayer::DurabilityPointsLossAll);
    type["DurabilityPointLossForEquipSlot"    ] = ALEBind::Method(&LuaPlayer::DurabilityPointLossForEquipSlot);
    type["DurabilityRepairAll"                ] = ALEBind::Method(&LuaPlayer::DurabilityRepairAll);
    type["DurabilityRepair"                   ] = ALEBind::Method(&LuaPlayer::DurabilityRepair);
    type["ModifyHonorPoints"                  ] = ALEBind::Method(&LuaPlayer::ModifyHonorPoints);
    type["ModifyArenaPoints"                  ] = ALEBind::Method(&LuaPlayer::ModifyArenaPoints);
    type["LeaveBattleground"                  ] = ALEBind::Method(&LuaPlayer::LeaveBattleground);
    type["UnbindInstance"                     ] = ALEBind::Method(&LuaPlayer::UnbindInstance);
    type["UnbindAllInstances"                 ] = ALEBind::Method(&LuaPlayer::UnbindAllInstances);
    type["RemoveFromBattlegroundRaid"         ] = ALEBind::Method(&LuaPlayer::RemoveFromBattlegroundRaid);
    type["ResetAchievements"                  ] = ALEBind::Method(&LuaPlayer::ResetAchievements);
    type["KickPlayer"                         ] = ALEBind::Method(&LuaPlayer::KickPlayer);
    type["LogoutPlayer"                       ] = ALEBind::Method(&LuaPlayer::LogoutPlayer);
    type["SendTrainerList"                    ] = ALEBind::Method(&LuaPlayer::SendTrainerList);
    type["SendListInventory"                  ] = ALEBind::Method(&LuaPlayer::SendListInventory);
    type["SendShowBank"                       ] = ALEBind::Method(&LuaPlayer::SendShowBank);
    type["SendTabardVendorActivate"           ] = ALEBind::Method(&LuaPlayer::SendTabardVendorActivate);
    type["SendSpiritResurrect"                ] = ALEBind::Method(&LuaPlayer::SendSpiritResurrect);
    type["SendTaxiMenu"                       ] = ALEBind::Method(&LuaPlayer::SendTaxiMenu);
    type["SendUpdateWorldState"               ] = ALEBind::Method(&LuaPlayer::SendUpdateWorldState);
    type["RewardQuest"                        ] = ALEBind::Method(&LuaPlayer::RewardQuest);
    type["SendAuctionMenu"                    ] = ALEBind::Method(&LuaPlayer::SendAuctionMenu);
    type["SendShowMailBox"                    ] = ALEBind::Method(&LuaPlayer::SendShowMailBox);
    type["StartTaxi"                          ] = ALEBind::Method(&LuaPlayer::StartTaxi);
    type["GossipSendPOI"                      ] = ALEBind::Method(&LuaPlayer::GossipSendPOI);
    type["GossipAddQuests"                    ] = ALEBind::Method(&LuaPlayer::GossipAddQuests);
    type["SendQuestTemplate"                  ] = ALEBind::Method(&LuaPlayer::SendQuestTemplate);
    type["SpawnBones"                         ] = ALEBind::Method(&LuaPlayer::SpawnBones);
    type["RemovedInsignia"                    ] = ALEBind::Method(&LuaPlayer::RemovedInsignia);
    type["SendGuildInvite"                    ] = ALEBind::Method(&LuaPlayer::SendGuildInvite);
    type["Mute"                               ] = ALEBind::Method(&LuaPlayer::Mute);
    type["SummonPlayer"                       ] = ALEBind::Method(&LuaPlayer::SummonPlayer);
    type["SaveToDB"                           ] = ALEBind::Method(&LuaPlayer::SaveToDB);
    type["GroupInvite"                        ] = ALEBind::Method(&LuaPlayer::GroupInvite);
    type["GroupCreate"                        ] = ALEBind::Method(&LuaPlayer::GroupCreate);
    type["SendCinematicStart"                 ] = ALEBind::Method(&LuaPlayer::SendCinematicStart);
    type["SendMovieStart"                     ] = ALEBind::Method(&LuaPlayer::SendMovieStart);
    type["UpdatePlayerSetting"                ] = ALEBind::Method(&LuaPlayer::UpdatePlayerSetting);
    type["TeleportTo"                         ] = ALEBind::Method(&LuaPlayer::TeleportTo);
    type["SummonPet"                          ] = ALEBind::Method(&LuaPlayer::SummonPet);
    type["CreatePet"                          ] = ALEBind::Method(&LuaPlayer::CreatePet);
    type["UnsummonPetTemporarily"             ] = ALEBind::Method(&LuaPlayer::UnsummonPetTemporarily);
    type["RemovePet"                          ] = ALEBind::Method(&LuaPlayer::RemovePet);
    type["LearnPetTalent"                     ] = ALEBind::Method(&LuaPlayer::LearnPetTalent);
    type["ResummonPetTemporaryUnSummonedIfAny"] = ALEBind::Method(&LuaPlayer::ResummonPetTemporaryUnSummonedIfAny);
    type["SetPlayerFlag"                      ] = ALEBind::Method(&LuaPlayer::SetPlayerFlag);
    type["RemovePlayerFlag"                   ] = ALEBind::Method(&LuaPlayer::RemovePlayerFlag);
    type["DoRandomRoll"                       ] = ALEBind::Method(&LuaPlayer::DoRandomRoll);
    type["EnvironmentalDamage"                ] = ALEBind::Method(&LuaPlayer::EnvironmentalDamage);
    type["InitTaxiNodesForLevel"              ] = ALEBind::Method(&LuaPlayer::InitTaxiNodesForLevel);
    type["AbandonQuest"                       ] = ALEBind::Method(&LuaPlayer::AbandonQuest);
    type["AddWeaponProficiency"               ] = ALEBind::Method(&LuaPlayer::AddWeaponProficiency);
    type["AddArmorProficiency"                ] = ALEBind::Method(&LuaPlayer::AddArmorProficiency);
    type["SetAmmo"                            ] = ALEBind::Method(&LuaPlayer::SetAmmo);
    type["RemoveAmmo"                         ] = ALEBind::Method(&LuaPlayer::RemoveAmmo);
    type["SetCanTeleport"                     ] = ALEBind::Method(&LuaPlayer::SetCanTeleport);
    type["SetIsSpectator"                     ] = ALEBind::Method(&LuaPlayer::SetIsSpectator);
    type["SetViewpoint"                       ] = ALEBind::Method(&LuaPlayer::SetViewpoint);
    type["ToggleInstantFlight"                ] = ALEBind::Method(&LuaPlayer::ToggleInstantFlight);
    type["SetCreationTime"                    ] = ALEBind::Method(&LuaPlayer::SetCreationTime);
    type["ApplyRatingMod"                     ] = ALEBind::Method(&LuaPlayer::ApplyRatingMod);
}
