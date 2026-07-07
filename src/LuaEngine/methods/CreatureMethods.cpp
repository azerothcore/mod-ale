/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"
#include "ALEUtility.h"

#include "Creature.h"
#include "CreatureAI.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ThreatManager.h"

/***
 * Non-[Player] controlled [Unit]s (i.e. NPCs).
 *
 * Inherits all methods from: [Object], [WorldObject], [Unit]
 */
namespace LuaCreature
{
    /**
     * Returns `true` if the [Creature] can regenerate health,
     *   and returns `false` otherwise.
     *
     * @return bool isRegenerating
     */
    bool IsRegeneratingHealth(Creature* creature)
    {
        return creature->isRegeneratingHealth();
    }

    /**
     * Sets whether the [Creature] can regenerate health or not.
     *
     * @param bool enable = true : `true` to enable health regeneration, `false` to disable it
     */
    void SetRegeneratingHealth(Creature* creature, sol::optional<bool> enable)
    {
        creature->SetRegeneratingHealth(enable.value_or(true));
    }

    /**
     * Returns `true` if the [Creature] is set to not give reputation when killed,
     *   and returns `false` otherwise.
     *
     * @return bool reputationDisabled
     */
    bool IsReputationGainDisabled(Creature* creature)
    {
        return creature->IsReputationRewardDisabled();
    }

    /**
     * Returns `true` if the [Creature] completes the [Quest] with the ID `questID`,
     *   and returns `false` otherwise.
     *
     * @param uint32 questID : the ID of a [Quest]
     * @return bool completesQuest
     */
    bool CanCompleteQuest(Creature* creature, uint32 quest_id)
    {
        return creature->hasInvolvedQuest(quest_id);
    }

    /**
     * Returns `true` if the [Creature] can be targeted for attack,
     *   and returns `false` otherwise.
     *
     * @param bool mustBeDead = false : if `true`, only returns `true` if the [Creature] is also dead. Otherwise, it must be alive.
     * @return bool targetable
     */
    bool IsTargetableForAttack(Creature* creature, sol::optional<bool> mustBeDead)
    {
        return creature->isTargetableForAttack(mustBeDead.value_or(false));
    }

    /**
     * Returns `true` if the [Creature] can assist `friend` in combat against `enemy`,
     *   and returns `false` otherwise.
     *
     * @param [Unit] friend : the Unit we will be assisting
     * @param [Unit] enemy : the Unit that we would attack if we assist `friend`
     * @param bool checkFaction = true : if `true`, the [Creature] must be the same faction as `friend` to assist
     * @return bool canAssist
     */
    bool CanAssistTo(Creature* creature, Unit* u, Unit* enemy, sol::optional<bool> checkfaction)
    {
        return creature->CanAssistTo(u, enemy, checkfaction.value_or(true));
    }

    /**
     * Returns `true` if the [Creature] has searched for combat assistance already,
     *   and returns `false` otherwise.
     *
     * @return bool searchedForAssistance
     */
    bool HasSearchedAssistance(Creature* creature)
    {
        return creature->HasSearchedAssistance();
    }

    /**
     * Returns `true` if the [Creature] will give its loot to `player`,
     *   and returns `false` otherwise.
     *
     * @return bool tapped
     */
    bool IsTappedBy(Creature* creature, Player* player)
    {
        return creature->isTappedBy(player);
    }

    /**
     * Returns `true` if the [Creature] will give its loot to a [Player] or [Group],
     *   and returns `false` otherwise.
     *
     * @return bool hasLootRecipient
     */
    bool HasLootRecipient(Creature* creature)
    {
        return creature->hasLootRecipient();
    }

    /**
     * Returns `true` if the [Creature] can start attacking nearby hostile [Unit]s,
     *   and returns `false` otherwise.
     *
     * @return bool canAggro
     */
    bool CanAggro(Creature* creature)
    {
        return !creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    }

    /**
     * Returns `true` if the [Creature] can move through deep water,
     *   and returns `false` otherwise.
     *
     * @return bool canSwim
     */
    bool CanSwim(Creature* creature)
    {
        return creature->CanSwim();
    }

    /**
     * Returns `true` if the [Creature] can move on land,
     *   and returns `false` otherwise.
     *
     * @return bool canWalk
     */
    bool CanWalk(Creature* creature)
    {
        return creature->CanWalk();
    }

    /**
     * Returns `true` if the [Creature] is returning to its spawn position from combat,
     *   and returns `false` otherwise.
     *
     * @return bool inEvadeMode
     */
    bool IsInEvadeMode(Creature* creature)
    {
        return creature->IsInEvadeMode();
    }

    /**
     * Returns `true` if the [Creature]'s rank is Elite or Rare Elite,
     *   and returns `false` otherwise.
     *
     * @return bool isElite
     */
    bool IsElite(Creature* creature)
    {
        return creature->isElite();
    }

    /**
     * Returns `true` if the [Creature] is a city guard,
     *   and returns `false` otherwise.
     *
     * @return bool isGuard
     */
    bool IsGuard(Creature* creature)
    {
        return creature->IsGuard();
    }

    /**
     * Returns `true` if the [Creature] is a civilian,
     *   and returns `false` otherwise.
     *
     * @return bool isCivilian
     */
    bool IsCivilian(Creature* creature)
    {
        return creature->IsCivilian();
    }

    /**
     * Returns `true` if the [Creature] is the leader of a player faction,
     *   and returns `false` otherwise.
     *
     * @return bool isLeader
     */
    bool IsRacialLeader(Creature* creature)
    {
        return creature->IsRacialLeader();
    }

    /**
     * Returns `true` if the [Creature]'s flags_extra includes Dungeon Boss (0x1000000),
     *   and returns `false` otherwise.
     *
     * @return bool isDungeonBoss
     */
    bool IsDungeonBoss(Creature* creature)
    {
        return creature->IsDungeonBoss();
    }

    /**
     * Returns `true` if the [Creature]'s rank is Boss,
     *   and returns `false` otherwise.
     *
     * @return bool isWorldBoss
     */
    bool IsWorldBoss(Creature* creature)
    {
        return creature->isWorldBoss();
    }

    /**
     * Returns `true` if the [Creature] cannot cast `spellId` due to a category cooldown,
     *   and returns `false` otherwise.
     *
     * @param uint32 spellId : the ID of a [Spell]
     * @return bool hasCooldown
     */
    bool HasCategoryCooldown(Creature* creature, uint32 spell)
    {
        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spell))
            return info->GetCategory() && creature->HasSpellCooldown(spell);

        return false;
    }

    /**
     * Returns `true` if the [Creature] can cast `spellId` when mind-controlled,
     *   and returns `false` otherwise.
     *
     * @param uint32 spellId : the ID of a [Spell]
     * @return bool hasSpell
     */
    bool HasSpell(Creature* creature, uint32 id)
    {
        return creature->HasSpell(id);
    }

    /**
     * Returns `true` if the [Creature] starts the [Quest] `questId`,
     *   and returns `false` otherwise.
     *
     * @param uint32 questId : the ID of a [Quest]
     * @return bool hasQuest
     */
    bool HasQuest(Creature* creature, uint32 questId)
    {
        return creature->hasQuest(questId);
    }

    /**
     * Returns `true` if the [Creature] has `spellId` on cooldown,
     *   and returns `false` otherwise.
     *
     * @param uint32 spellId : the ID of a [Spell]
     * @return bool hasCooldown
     */
    bool HasSpellCooldown(Creature* creature, uint32 spellId)
    {
        return creature->HasSpellCooldown(spellId);
    }

    /**
     * Returns `true` if the [Creature] can fly,
     *   and returns `false` otherwise.
     *
     * @return bool canFly
     */
    bool CanFly(Creature* creature)
    {
        return creature->CanFly();
    }

    /**
     * Returns `true` if the [Creature] is an invisible trigger,
     *   and returns `false` otherwise.
     *
     * @return bool canFly
     */
    bool IsTrigger(Creature* creature)
    {
        return creature->IsTrigger();
    }

    /**
     * Returns true if the [Creature] is damaged enough for looting
     *
     * @return bool isDamagedEnough
     */
    bool IsDamageEnoughForLootingAndReward(Creature* creature)
    {
        return creature->IsDamageEnoughForLootingAndReward();
    }

    /**
     * Returns true if the [Creature] can start attacking specified target
     *
     * Does not work on most targets
     *
     * @param [Unit] target
     * @param bool force = true : force [Creature] to attack
     */
    bool CanStartAttack(Creature* creature, Unit* target) // TODO: Implement core side
    {
        return creature->CanStartAttack(target);
    }

    /**
     * Returns true if [Creature] has the specified loot mode
     *
     * @param uint16 lootMode
     * @return bool hasLootMode
     */
    bool HasLootMode(Creature* creature, uint16 lootMode) // TODO: Implement LootMode features
    {
        return creature->HasLootMode(lootMode);
    }

    /**
     * Returns the time it takes for this [Creature] to respawn once killed.
     *
     * This value does not usually change over a [Creature]'s lifespan,
     *   but can be modified by [Creature:SetRespawnDelay].
     *
     * @return uint32 respawnDelay : the respawn delay, in seconds
     */
    uint32 GetRespawnDelay(Creature* creature)
    {
        return creature->GetRespawnDelay();
    }

    /**
     * Returns the radius the [Creature] is permitted to wander from its
     *   respawn point.
     *
     * @return float wanderRadius
     */
    float GetWanderRadius(Creature* creature)
    {
        return creature->GetWanderDistance();
    }

    /**
     * Returns the current waypoint path ID of the [Creature].
     *
     * @return uint32 pathId
     */
    uint32 GetWaypointPath(Creature* creature)
    {
        return creature->GetWaypointPath();
    }

    /**
     * Returns the current waypoint ID of the [Creature].
     *
     * @return uint32 wpId
     */
    uint32 GetCurrentWaypointId(Creature* creature)
    {
        return creature->GetCurrentWaypointID();
    }

    /**
    * Returns the spawn ID for this [Creature].
    *
    * @return uint32 spawnId
    */
    uint32 GetSpawnId(Creature* creature)
    {
        return creature->GetSpawnId();
    }

    /**
     * Returns the default movement type for this [Creature].
     *
     * @return [MovementGeneratorType] defaultMovementType
     */
    MovementGeneratorType GetDefaultMovementType(Creature* creature)
    {
        return creature->GetDefaultMovementType();
    }

    /**
     * Returns the aggro range of the [Creature] for `target`.
     *
     * @param [Unit] target
     * @return float aggroRange
     */
    float GetAggroRange(Creature* creature, Unit* target)
    {
        return creature->GetAggroRange(target);
    }

    /**
     * Returns the [Group] that can loot this [Creature].
     *
     * @return [Group] lootRecipientGroup : the group or `nil`
     */
    Group* GetLootRecipientGroup(Creature* creature)
    {
        return creature->GetLootRecipientGroup();
    }

    /**
     * Returns the [Player] that can loot this [Creature].
     *
     * @return [Player] lootRecipient : the player or `nil`
     */
    Player* GetLootRecipient(Creature* creature)
    {
        return creature->GetLootRecipient();
    }

    /**
     * Returns the [Creature]'s script name.
     *
     * This is used by the core to apply C++ scripts to the Creature.
     *
     * It is not used by ALE. ALE will override AI scripts.
     *
     * @return string scriptName
     */
    std::string GetScriptName(Creature* creature)
    {
        return creature->GetScriptName();
    }

    /**
     * Returns the [Creature]'s AI name.
     *
     * This is used by the core to assign the Creature's default AI.
     *
     * If the Creature is scripted by ALE, the AI is overriden.
     *
     * @return string AIName
     */
    std::string GetAIName(Creature* creature)
    {
        return creature->GetAIName();
    }

    /**
     * Returns the [Creature]'s script ID.
     *
     * Every C++ script name is assigned a unique ID by the core.
     *   This returns the ID for this [Creature]'s script name.
     *
     * @return uint32 scriptID
     */
    uint32 GetScriptId(Creature* creature)
    {
        return creature->GetScriptId();
    }

    /**
     * Returns the [Creature]'s cooldown for `spellID`.
     *
     * @param uint32 spellID
     * @return uint32 cooldown : the cooldown, in milliseconds
     */
    uint32 GetCreatureSpellCooldownDelay(Creature* creature, uint32 spell)
    {
        if (sSpellMgr->GetSpellInfo(spell))
            return creature->GetSpellCooldown(spell);

        return 0;
    }

    /**
     * Returns the delay between when the [Creature] dies and when its body despawns.
     *
     * @return uint32 corpseDelay : the delay, in seconds
     */
    uint32 GetCorpseDelay(Creature* creature)
    {
        return creature->GetCorpseDelay();
    }

    /**
     * Returns position the [Creature] returns to when evading from combat
     *   or respawning.
     *
     * @return float x
     * @return float y
     * @return float z
     * @return float o
     */
    std::tuple<float, float, float, float> GetHomePosition(Creature* creature)
    {
        float x, y, z, o;
        creature->GetHomePosition(x, y, z, o);

        return std::tuple<float, float, float, float>(x, y, z, o);
    }

    /**
     * Sets the position the [Creature] returns to when evading from combat
     *   or respawning.
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float o
     */
    void SetHomePosition(Creature* creature, float x, float y, float z, float o)
    {
        creature->SetHomePosition(x, y, z, o);
    }

    enum SelectAggroTarget
    {
        SELECT_TARGET_RANDOM = 0,   // Just selects a random target
        SELECT_TARGET_TOPAGGRO,     // Selects targes from top aggro to bottom
        SELECT_TARGET_BOTTOMAGGRO,  // Selects targets from bottom aggro to top
        SELECT_TARGET_NEAREST,
        SELECT_TARGET_FARTHEST
    };

    /**
    * Returns a target from the [Creature]'s threat list based on the
    *   supplied arguments.
    *
    *     enum SelectAggroTarget
    *     {
    *         SELECT_TARGET_RANDOM = 0,  //Just selects a random target
    *         SELECT_TARGET_TOPAGGRO,    //Selects targets from top aggro to bottom
    *         SELECT_TARGET_BOTTOMAGGRO, //Selects targets from bottom aggro to top
    *         SELECT_TARGET_NEAREST,
    *         SELECT_TARGET_FARTHEST
    *     };
    *
    * For example, if you wanted to select the third-farthest [Player]
    *   within 50 yards that has the [Aura] "Corrupted Blood" (ID 24328),
    *   you could use this function like so:
    *
    *     target = creature:GetAITarget(4, true, 3, 50, 24328)
    *
    * @param [SelectAggroTarget] targetType : how the threat list should be sorted
    * @param bool playerOnly = false : if `true`, skips targets that aren't [Player]s
    * @param uint32 position = 0 : used as an offset into the threat list. If `targetType` is random, used as the number of players from top of aggro to choose from
    * @param float distance = 0.0 : if positive, the maximum distance for the target. If negative, the minimum distance
    * @param int32 aura = 0 : if positive, the target must have this [Aura]. If negative, the the target must not have this Aura
    * @return [Unit] target : the target, or `nil`
    */
    Unit* GetAITarget(Creature* creature, uint32 targetType, sol::optional<bool> playerOnlyArg, sol::optional<uint32> positionArg, sol::optional<float> distArg, sol::optional<int32> auraArg)
    {
        bool playerOnly = playerOnlyArg.value_or(false);
        uint32 position = positionArg.value_or(0);
        float dist = distArg.value_or(0.0f);
        int32 aura = auraArg.value_or(0);

        ThreatManager const& threatMgr = creature->GetThreatMgr();

        if (threatMgr.IsThreatListEmpty())
            return nullptr;
        if (position >= threatMgr.GetThreatListSize())
            return nullptr;

        std::list<Unit*> targetList;

        for (ThreatReference const* ref : threatMgr.GetSortedThreatList())
        {
            Unit* target = ref->GetVictim();

            if (!target)
                continue;
            if (playerOnly && target->GetTypeId() != TYPEID_PLAYER)
                continue;
            if (aura > 0 && !target->HasAura(aura))
                continue;
            else if (aura < 0 && target->HasAura(-aura))
                continue;
            if (dist > 0.0f && !creature->IsWithinDist(target, dist))
                continue;
            else if (dist < 0.0f && creature->IsWithinDist(target, -dist))
                continue;
            targetList.push_back(target);
        }

        if (targetList.empty())
            return nullptr;
        if (position >= targetList.size())
            return nullptr;

        if (targetType == SELECT_TARGET_NEAREST || targetType == SELECT_TARGET_FARTHEST)
            targetList.sort(ALEUtil::ObjectDistanceOrderPred(creature));

        switch (targetType)
        {
            case SELECT_TARGET_NEAREST:
            case SELECT_TARGET_TOPAGGRO:
                {
                    std::list<Unit*>::const_iterator itr = targetList.begin();
                    if (position)
                        std::advance(itr, position);
                    return *itr;
                }
            case SELECT_TARGET_FARTHEST:
            case SELECT_TARGET_BOTTOMAGGRO:
                {
                    std::list<Unit*>::reverse_iterator ritr = targetList.rbegin();
                    if (position)
                        std::advance(ritr, position);
                    return *ritr;
                }
            case SELECT_TARGET_RANDOM:
                {
                    std::list<Unit*>::const_iterator itr = targetList.begin();
                    if (position)
                        std::advance(itr, urand(0, position));
                    else
                        std::advance(itr, urand(0, targetList.size() - 1));
                    return *itr;
                }
            default:
                throw std::invalid_argument("SelectAggroTarget expected");
        }

        return nullptr;
    }

    /**
     * Returns all [Unit]s in the [Creature]'s threat list.
     *
     * @return table targets
     */
    sol::table GetAITargets(Creature* creature, sol::this_state s)
    {
        ThreatManager const& threatMgr = creature->GetThreatMgr();

        sol::state_view lua(s);
        sol::table tbl = lua.create_table(static_cast<int>(threatMgr.GetThreatListSize()), 0);
        uint32 i = 0;

        for (ThreatReference const* ref : threatMgr.GetSortedThreatList())
            tbl[++i] = ALEBind::ToLuaDynamic(lua, ref->GetVictim());

        return tbl;
    }

    /**
     * Returns the number of [Unit]s in this [Creature]'s threat list.
     *
     * @return int targetsCount
     */
    std::size_t GetAITargetsCount(Creature* creature)
    {
        return creature->GetThreatMgr().GetThreatListSize();
    }

    /**
     * Returns the [Creature]'s NPC flags.
     *
     * These are used to control whether the NPC is a vendor, can repair items,
     *   can give quests, etc.
     *
     * @return [NPCFlags] npcFlags
     */
    uint32 GetNPCFlags(Creature* creature)
    {
        return creature->GetUInt32Value(UNIT_NPC_FLAGS);
    }

    /**
     * Returns the [Creature]'s Unit flags.
     *
     * These are used to control whether the NPC is attackable or not, among other things.
     *
     * @return [UnitFlags] unitFlags
     */
    uint32 GetUnitFlags(Creature* creature)
    {
        return creature->GetUInt32Value(UNIT_FIELD_FLAGS);
    }

    /**
     * Returns the [Creature]'s Unit flags 2.
     *
     * @return [UnitFlags2] unitFlags2
     */
    uint32 GetUnitFlagsTwo(Creature* creature)
    {
        return creature->GetUInt32Value(UNIT_FIELD_FLAGS_2);
    }

    /**
     * Returns the [Creature]'s Extra flags.
     *
     * These are used to control whether the NPC is a civilian, uses pathfinding,
     *   if it's a guard, etc.
     *
     * @return [ExtraFlags] extraFlags
     */
    uint32 GetExtraFlags(Creature* creature)
    {
        return creature->GetCreatureTemplate()->flags_extra;
    }

    /**
     * Returns the [Creature]'s rank.
     *
     * @return [Rank] rank
     */
    uint32 GetRank(Creature* creature)
    {
        return creature->GetCreatureTemplate()->rank;
    }

    /**
     * Returns the [Creature]'s shield block value.
     *
     * @return uint32 shieldBlockValue
     */
    uint32 GetShieldBlockValue(Creature* creature)
    {
        return creature->GetShieldBlockValue();
    }

    /**
     * Returns the loot mode flags for the specified [Creature].
     *
     * @param [Creature] creature : the creature whose loot mode to get
     * @return uint16 lootMode : the loot mode bitmask of the creature
     */
    uint16 GetLootMode(Creature* creature) // TODO: Implement LootMode features
    {
        return creature->GetLootMode();
    }

    /**
     * Returns the guid of the [Creature] that is used as the ID in the database
     *
     * @return uint32 dbguid
     */
    uint32 GetDBTableGUIDLow(Creature* creature)
    {
        return creature->GetSpawnId();
    }

    /**
     * Returns the [Creature]'s current ReactState.
     *
     * <pre>
     * enum ReactState
     * {
     *     REACT_PASSIVE       = 0,
     *     REACT_DEFENSIVE     = 1,
     *     REACT_AGGRESSIVE    = 2
     * };
     * </pre>
     *
     * @return [ReactState] state
     */
    int32 GetReactState(Creature* creature)
    {
        ReactStates state = creature->GetReactState();
        return static_cast<int32>(state);
    }

    /**
     * Sets the [Creature]'s NPC flags to `flags`.
     *
     * @param [NPCFlags] flags
     */
    void SetNPCFlags(Creature* creature, uint32 flags)
    {
        creature->SetUInt32Value(UNIT_NPC_FLAGS, flags);
    }

    /**
     * Sets the [Creature]'s Unit flags to `flags`.
     *
     * @param [UnitFlags] flags
     */
    void SetUnitFlags(Creature* creature, uint32 flags)
    {
        creature->SetUInt32Value(UNIT_FIELD_FLAGS, flags);
    }

    /**
     * Sets the [Creature]'s Unit flags2 to `flags`.
     *
     * @param [UnitFlags2] flags
     */
    void SetUnitFlagsTwo(Creature* creature, uint32 flags)
    {
        creature->SetUInt32Value(UNIT_FIELD_FLAGS_2, flags);
    }

    /**
     * Sets the [Creature]'s ReactState to `state`.
     *
     * @param [ReactState] state
     */
    void SetReactState(Creature* creature, uint32 state)
    {
        creature->SetReactState((ReactStates)state);
    }

    /**
     * Makes the [Creature] able to fly if enabled.
     *
     * @param bool disable
     */
    void SetDisableGravity(Creature* creature, bool disable)
    {
        creature->SetDisableGravity(disable);
    }

    /**
     * Sets the loot mode flags for the specified [Creature].
     *
     * @param [Creature] creature : the creature whose loot mode to set
     * @param uint16 lootMode : the loot mode bitmask to apply
     */
    void SetLootMode(Creature* creature, uint16 lootMode) // TODO: Implement LootMode features
    {
        creature->SetLootMode(lootMode);
    }

    /**
     * Sets the [Creature]'s death state to `deathState`.
     *
     * @param [DeathState] deathState
     */
    void SetDeathState(Creature* creature, int32 state)
    {
        creature->setDeathState((DeathState)state);
    }

    /**
     * Sets whether the [Creature] is currently walking or running.
     *
     * @param bool enable = true : `true` to enable walking, `false` for running
     */
    void SetWalk(Creature* creature, sol::optional<bool> enable)           // TODO: Move same to Player ?
    {
        creature->SetWalk(enable.value_or(true));
    }

    /**
     * Equips given [Item]s to the [Unit]. Using 0 removes the equipped [Item]
     *
     * @param uint32 main_hand : main hand [Item]'s entry
     * @param uint32 off_hand : off hand [Item]'s entry
     * @param uint32 ranged : ranged [Item]'s entry
     */
    void SetEquipmentSlots(Creature* creature, uint32 main_hand, uint32 off_hand, uint32 ranged)
    {
        creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, main_hand);
        creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, off_hand);
        creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, ranged);
    }

    /**
     * Sets whether the [Creature] can be aggroed.
     *
     * @param bool allow = true : `true` to allow aggro, `false` to disable aggro
     */
    void SetAggroEnabled(Creature* creature, sol::optional<bool> allowArg)
    {
        bool allow = allowArg.value_or(true);

        if (allow)
            creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        else
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    }

    /**
     * Sets whether the [Creature] gives reputation or not.
     *
     * @param bool disable = true : `true` to disable reputation, `false` to enable
     */
    void SetDisableReputationGain(Creature* creature, sol::optional<bool> disable)
    {
        creature->SetReputationRewardDisabled(disable.value_or(true));
    }

    /**
     * Sets the [Creature] as in combat with all [Player]s in the dungeon instance.
     *
     * This is used by raid bosses to prevent Players from using out-of-combat
     *   actions once the encounter has begun.
     */
    void SetInCombatWithZone(Creature* creature)
    {
        if (creature->IsAIEnabled)
            creature->AI()->DoZoneInCombat();
    }

    /**
     * Sets the distance the [Creature] can wander from it's spawn point.
     *
     * @param float distance
     */
    void SetWanderRadius(Creature* creature, float dist)
    {
        creature->SetWanderDistance(dist);
    }

    /**
     * Sets the time it takes for the [Creature] to respawn when killed.
     *
     * @param uint32 delay : the delay, in seconds
     */
    void SetRespawnDelay(Creature* creature, uint32 delay)
    {
        creature->SetRespawnDelay(delay);
    }

    /**
     * Sets the default movement type of the [Creature].
     *
     * @param [MovementGeneratorType] type
     */
    void SetDefaultMovementType(Creature* creature, int32 type)
    {
        creature->SetDefaultMovementType((MovementGeneratorType)type);
    }

    /**
     * Sets whether the [Creature] can search for assistance at low health or not.
     *
     * @param bool enable = true : `true` to disable searching, `false` to allow
     */
    void SetNoSearchAssistance(Creature* creature, sol::optional<bool> val)
    {
        creature->SetNoSearchAssistance(val.value_or(true));
    }

    /**
     * Sets whether the [Creature] can call nearby enemies for help in combat or not.
     *
     * @param bool enable = true : `true` to disable calling for help, `false` to enable
     */
    void SetNoCallAssistance(Creature* creature, sol::optional<bool> val)
    {
        creature->SetNoCallAssistance(val.value_or(true));
    }

    /**
     * Sets whether the creature is hovering / levitating or not.
     *
     * @param bool enable = true : `true` to enable hovering, `false` to disable
     */
    void SetHover(Creature* creature, sol::optional<bool> enable)
    {
        creature->SetHover(enable.value_or(true));
    }

    /**
     * Despawn this [Creature].
     *
     * @param uint32 delay = 0 : dely to despawn in milliseconds
     */
    void DespawnOrUnsummon(Creature* creature, sol::optional<uint32> msTimeToDespawn)
    {
        creature->DespawnOrUnsummon(Milliseconds(msTimeToDespawn.value_or(0)));
    }

    /**
     * Respawn this [Creature].
     */
    void Respawn(Creature* creature)
    {
        creature->Respawn();
    }

    /**
     * Remove this [Creature]'s corpse.
     */
    void RemoveCorpse(Creature* creature)
    {
        creature->RemoveCorpse();
    }

    /**
     * Handles this [Creature]'s corpse state after all loot is removed.
     */
    void AllLootRemovedFromCorpse(Creature* creature)
    {
        creature->AllLootRemovedFromCorpse();
    }

    /**
     * Sets the time it takes for the [Creature]'s corpse to despawn when killed.
     *
     * @param uint32 delay : the delay, in seconds
     */
    void SetCorpseDelay(Creature* creature, uint32 delay)
    {
        creature->SetCorpseDelay(delay);
    }

    /**
     * Make the [Creature] start following its waypoint path.
     */
    void MoveWaypoint(Creature* creature)
    {
        creature->GetMotionMaster()->MoveWaypoint(creature->GetWaypointPath(), true);
    }

    /**
     * Make the [Creature] call for assistance in combat from other nearby [Creature]s.
     */
    void CallAssistance(Creature* creature)
    {
        creature->CallAssistance();
    }

    /**
     * Make the [Creature] call for help in combat from friendly [Creature]s within `radius`.
     *
     * @param float radius
     */
    void CallForHelp(Creature* creature, float radius)
    {
        creature->CallForHelp(radius);
    }

    /**
     * Make the [Creature] flee combat to get assistance from a nearby friendly [Creature].
     */
    void FleeToGetAssistance(Creature* creature)
    {
        creature->DoFleeToGetAssistance();
    }

    /**
     * Make the [Creature] attack `target`.
     *
     * @param [Unit] target
     */
    void AttackStart(Creature* creature, Unit* target)
    {
        creature->AI()->AttackStart(target);
    }

    /**
     * Save the [Creature] in the database.
     */
    void SaveToDB(Creature* creature)
    {
        creature->SaveToDB();
    }

    /**
     * Make the [Creature] try to find a new target.
     *
     * This should be called every update cycle for the Creature's AI.
     */
    Unit* SelectVictim(Creature* creature)
    {
        return creature->SelectVictim();
    }

    /**
     * Transform the [Creature] into another Creature.
     *
     * @param uint32 entry : the Creature ID to transform into
     * @param uint32 dataGUIDLow = 0 : use this Creature's model and equipment instead of the defaults
     */
    void UpdateEntry(Creature* creature, uint32 entry, sol::optional<uint32> dataGuidLowArg)
    {
        uint32 dataGuidLow = dataGuidLowArg.value_or(0);

        creature->UpdateEntry(entry, dataGuidLow ? sObjectMgr->GetCreatureData(dataGuidLow) : nullptr);
    }

    /**
     * Resets [Creature]'s loot mode to default
     */
    void ResetLootMode(Creature* creature) // TODO: Implement LootMode features
    {
        creature->ResetLootMode();
    }

    /**
     * Removes specified loot mode from [Creature]
     *
     * @param uint16 lootMode
     */
    void RemoveLootMode(Creature* creature, uint16 lootMode) // TODO: Implement LootMode features
    {
        creature->RemoveLootMode(lootMode);
    }

    /**
     * Adds a loot mode to the [Creature]
     *
     * @param uint16 lootMode
     */
    void AddLootMode(Creature* creature, uint16 lootMode) // TODO: Implement LootMode features
    {
        creature->AddLootMode(lootMode);
    }

    /**
     * Returns the [Creature]'s creature family ID (enumerated in CreatureFamily.dbc).
     *
     * <pre>
     * enum CreatureFamily
     * {
     *     CREATURE_FAMILY_NONE                = 0,    // TrinityCore only
     *     CREATURE_FAMILY_WOLF                = 1,
     *     CREATURE_FAMILY_CAT                 = 2,
     *     CREATURE_FAMILY_SPIDER              = 3,
     *     CREATURE_FAMILY_BEAR                = 4,
     *     CREATURE_FAMILY_BOAR                = 5,
     *     CREATURE_FAMILY_CROCOLISK           = 6,
     *     CREATURE_FAMILY_CARRION_BIRD        = 7,
     *     CREATURE_FAMILY_CRAB                = 8,
     *     CREATURE_FAMILY_GORILLA             = 9,
     *     CREATURE_FAMILY_HORSE_CUSTOM        = 10,   // Does not exist in DBC but used for horse like beasts in DB
     *     CREATURE_FAMILY_RAPTOR              = 11,
     *     CREATURE_FAMILY_TALLSTRIDER         = 12,
     *     CREATURE_FAMILY_FELHUNTER           = 15,
     *     CREATURE_FAMILY_VOIDWALKER          = 16,
     *     CREATURE_FAMILY_SUCCUBUS            = 17,
     *     CREATURE_FAMILY_DOOMGUARD           = 19,
     *     CREATURE_FAMILY_SCORPID             = 20,
     *     CREATURE_FAMILY_TURTLE              = 21,
     *     CREATURE_FAMILY_IMP                 = 23,
     *     CREATURE_FAMILY_BAT                 = 24,
     *     CREATURE_FAMILY_HYENA               = 25,
     *     CREATURE_FAMILY_BIRD_OF_PREY        = 26,   // Named CREATURE_FAMILY_OWL in Mangos
     *     CREATURE_FAMILY_WIND_SERPENT        = 27,
     *     CREATURE_FAMILY_REMOTE_CONTROL      = 28,
     *     CREATURE_FAMILY_FELGUARD            = 29,   // This and below is TBC+
     *     CREATURE_FAMILY_DRAGONHAWK          = 30,
     *     CREATURE_FAMILY_RAVAGER             = 31,
     *     CREATURE_FAMILY_WARP_STALKER        = 32,
     *     CREATURE_FAMILY_SPOREBAT            = 33,
     *     CREATURE_FAMILY_NETHER_RAY          = 34,
     *     CREATURE_FAMILY_SERPENT             = 35,
     *     CREATURE_FAMILY_SEA_LION            = 36,   // TBC only
     *     CREATURE_FAMILY_MOTH                = 37,   // This and below is WotLK+
     *     CREATURE_FAMILY_CHIMAERA            = 38,
     *     CREATURE_FAMILY_DEVILSAUR           = 39,
     *     CREATURE_FAMILY_GHOUL               = 40,
     *     CREATURE_FAMILY_SILITHID            = 41,
     *     CREATURE_FAMILY_WORM                = 42,
     *     CREATURE_FAMILY_RHINO               = 43,
     *     CREATURE_FAMILY_WASP                = 44,
     *     CREATURE_FAMILY_CORE_HOUND          = 45,
     *     CREATURE_FAMILY_SPIRIT_BEAST        = 46
     * };
     * </pre>
     *
     * @return [CreatureFamily] creatureFamily
     */
    sol::optional<uint32> GetCreatureFamily(Creature* creature)
    {
        uint32 entry = creature->GetEntry();

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(entry);
        if (cInfo)
            return cInfo->family;

        return sol::nullopt;
    }

    /**
     * Returns the [Creature]'s loot.
     *
     * @return [Loot] loot : the loot object
     */
    Loot* GetLoot(Creature* creature)
    {
        return &creature->loot;
    }
}

void RegisterCreatureMethods(sol::state& lua)
{
    sol::usertype<CreatureRef> type = ALEBind::NewHandleType<CreatureRef, UnitRef, WorldObjectRef, ObjectRef>(lua, "Creature");

    type["IsRegeneratingHealth"]              = ALEBind::Method(&LuaCreature::IsRegeneratingHealth);
    type["SetRegeneratingHealth"]             = ALEBind::Method(&LuaCreature::SetRegeneratingHealth);
    type["IsReputationGainDisabled"]          = ALEBind::Method(&LuaCreature::IsReputationGainDisabled);
    type["CanCompleteQuest"]                  = ALEBind::Method(&LuaCreature::CanCompleteQuest);
    type["IsTargetableForAttack"]             = ALEBind::Method(&LuaCreature::IsTargetableForAttack);
    type["CanAssistTo"]                       = ALEBind::Method(&LuaCreature::CanAssistTo);
    type["HasSearchedAssistance"]             = ALEBind::Method(&LuaCreature::HasSearchedAssistance);
    type["IsTappedBy"]                        = ALEBind::Method(&LuaCreature::IsTappedBy);
    type["HasLootRecipient"]                  = ALEBind::Method(&LuaCreature::HasLootRecipient);
    type["CanAggro"]                          = ALEBind::Method(&LuaCreature::CanAggro);
    type["CanSwim"]                           = ALEBind::Method(&LuaCreature::CanSwim);
    type["CanWalk"]                           = ALEBind::Method(&LuaCreature::CanWalk);
    type["IsInEvadeMode"]                     = ALEBind::Method(&LuaCreature::IsInEvadeMode);
    type["IsElite"]                           = ALEBind::Method(&LuaCreature::IsElite);
    type["IsGuard"]                           = ALEBind::Method(&LuaCreature::IsGuard);
    type["IsCivilian"]                        = ALEBind::Method(&LuaCreature::IsCivilian);
    type["IsRacialLeader"]                    = ALEBind::Method(&LuaCreature::IsRacialLeader);
    type["IsDungeonBoss"]                     = ALEBind::Method(&LuaCreature::IsDungeonBoss);
    type["IsWorldBoss"]                       = ALEBind::Method(&LuaCreature::IsWorldBoss);
    type["HasCategoryCooldown"]               = ALEBind::Method(&LuaCreature::HasCategoryCooldown);
    type["HasSpell"]                          = ALEBind::Method(&LuaCreature::HasSpell);
    type["HasQuest"]                          = ALEBind::Method(&LuaCreature::HasQuest);
    type["HasSpellCooldown"]                  = ALEBind::Method(&LuaCreature::HasSpellCooldown);
    type["CanFly"]                            = ALEBind::Method(&LuaCreature::CanFly);
    type["IsTrigger"]                         = ALEBind::Method(&LuaCreature::IsTrigger);
    type["IsDamageEnoughForLootingAndReward"] = ALEBind::Method(&LuaCreature::IsDamageEnoughForLootingAndReward);
    type["CanStartAttack"]                    = ALEBind::Method(&LuaCreature::CanStartAttack);
    type["HasLootMode"]                       = ALEBind::Method(&LuaCreature::HasLootMode);
    type["GetRespawnDelay"]                   = ALEBind::Method(&LuaCreature::GetRespawnDelay);
    type["GetWanderRadius"]                   = ALEBind::Method(&LuaCreature::GetWanderRadius);
    type["GetWaypointPath"]                   = ALEBind::Method(&LuaCreature::GetWaypointPath);
    type["GetCurrentWaypointId"]              = ALEBind::Method(&LuaCreature::GetCurrentWaypointId);
    type["GetSpawnId"]                        = ALEBind::Method(&LuaCreature::GetSpawnId);
    type["GetDefaultMovementType"]            = ALEBind::Method(&LuaCreature::GetDefaultMovementType);
    type["GetAggroRange"]                     = ALEBind::Method(&LuaCreature::GetAggroRange);
    type["GetLootRecipientGroup"]             = ALEBind::Method(&LuaCreature::GetLootRecipientGroup);
    type["GetLootRecipient"]                  = ALEBind::Method(&LuaCreature::GetLootRecipient);
    type["GetScriptName"]                     = ALEBind::Method(&LuaCreature::GetScriptName);
    type["GetAIName"]                         = ALEBind::Method(&LuaCreature::GetAIName);
    type["GetScriptId"]                       = ALEBind::Method(&LuaCreature::GetScriptId);
    type["GetCreatureSpellCooldownDelay"]     = ALEBind::Method(&LuaCreature::GetCreatureSpellCooldownDelay);
    type["GetCorpseDelay"]                    = ALEBind::Method(&LuaCreature::GetCorpseDelay);
    type["GetHomePosition"]                   = ALEBind::Method(&LuaCreature::GetHomePosition);
    type["SetHomePosition"]                   = ALEBind::Method(&LuaCreature::SetHomePosition);
    type["GetAITarget"]                       = ALEBind::Method(&LuaCreature::GetAITarget);
    type["GetAITargets"]                      = ALEBind::Method(&LuaCreature::GetAITargets);
    type["GetAITargetsCount"]                 = ALEBind::Method(&LuaCreature::GetAITargetsCount);
    type["GetNPCFlags"]                       = ALEBind::Method(&LuaCreature::GetNPCFlags);
    type["GetUnitFlags"]                      = ALEBind::Method(&LuaCreature::GetUnitFlags);
    type["GetUnitFlagsTwo"]                   = ALEBind::Method(&LuaCreature::GetUnitFlagsTwo);
    type["GetExtraFlags"]                     = ALEBind::Method(&LuaCreature::GetExtraFlags);
    type["GetRank"]                           = ALEBind::Method(&LuaCreature::GetRank);
    type["GetShieldBlockValue"]               = ALEBind::Method(&LuaCreature::GetShieldBlockValue);
    type["GetLootMode"]                       = ALEBind::Method(&LuaCreature::GetLootMode);
    type["GetDBTableGUIDLow"]                 = ALEBind::Method(&LuaCreature::GetDBTableGUIDLow);
    type["GetReactState"]                     = ALEBind::Method(&LuaCreature::GetReactState);
    type["SetNPCFlags"]                       = ALEBind::Method(&LuaCreature::SetNPCFlags);
    type["SetUnitFlags"]                      = ALEBind::Method(&LuaCreature::SetUnitFlags);
    type["SetUnitFlagsTwo"]                   = ALEBind::Method(&LuaCreature::SetUnitFlagsTwo);
    type["SetReactState"]                     = ALEBind::Method(&LuaCreature::SetReactState);
    type["SetDisableGravity"]                 = ALEBind::Method(&LuaCreature::SetDisableGravity);
    type["SetLootMode"]                       = ALEBind::Method(&LuaCreature::SetLootMode);
    type["SetDeathState"]                     = ALEBind::Method(&LuaCreature::SetDeathState);
    type["SetWalk"]                           = ALEBind::Method(&LuaCreature::SetWalk);
    type["SetEquipmentSlots"]                 = ALEBind::Method(&LuaCreature::SetEquipmentSlots);
    type["SetAggroEnabled"]                   = ALEBind::Method(&LuaCreature::SetAggroEnabled);
    type["SetDisableReputationGain"]          = ALEBind::Method(&LuaCreature::SetDisableReputationGain);
    type["SetInCombatWithZone"]               = ALEBind::Method(&LuaCreature::SetInCombatWithZone);
    type["SetWanderRadius"]                   = ALEBind::Method(&LuaCreature::SetWanderRadius);
    type["SetRespawnDelay"]                   = ALEBind::Method(&LuaCreature::SetRespawnDelay);
    type["SetDefaultMovementType"]            = ALEBind::Method(&LuaCreature::SetDefaultMovementType);
    type["SetNoSearchAssistance"]             = ALEBind::Method(&LuaCreature::SetNoSearchAssistance);
    type["SetNoCallAssistance"]               = ALEBind::Method(&LuaCreature::SetNoCallAssistance);
    type["SetHover"]                          = ALEBind::Method(&LuaCreature::SetHover);
    type["DespawnOrUnsummon"]                 = ALEBind::Method(&LuaCreature::DespawnOrUnsummon);
    type["Respawn"]                           = ALEBind::Method(&LuaCreature::Respawn);
    type["RemoveCorpse"]                      = ALEBind::Method(&LuaCreature::RemoveCorpse);
    type["AllLootRemovedFromCorpse"]          = ALEBind::Method(&LuaCreature::AllLootRemovedFromCorpse);
    type["SetCorpseDelay"]                    = ALEBind::Method(&LuaCreature::SetCorpseDelay);
    type["MoveWaypoint"]                      = ALEBind::Method(&LuaCreature::MoveWaypoint);
    type["CallAssistance"]                    = ALEBind::Method(&LuaCreature::CallAssistance);
    type["CallForHelp"]                       = ALEBind::Method(&LuaCreature::CallForHelp);
    type["FleeToGetAssistance"]               = ALEBind::Method(&LuaCreature::FleeToGetAssistance);
    type["AttackStart"]                       = ALEBind::Method(&LuaCreature::AttackStart);
    type["SaveToDB"]                          = ALEBind::Method(&LuaCreature::SaveToDB);
    type["SelectVictim"]                      = ALEBind::Method(&LuaCreature::SelectVictim);
    type["UpdateEntry"]                       = ALEBind::Method(&LuaCreature::UpdateEntry);
    type["ResetLootMode"]                     = ALEBind::Method(&LuaCreature::ResetLootMode);
    type["RemoveLootMode"]                    = ALEBind::Method(&LuaCreature::RemoveLootMode);
    type["AddLootMode"]                       = ALEBind::Method(&LuaCreature::AddLootMode);
    type["GetCreatureFamily"]                 = ALEBind::Method(&LuaCreature::GetCreatureFamily);
    type["GetLoot"]                           = ALEBind::Method(&LuaCreature::GetLoot);
}
