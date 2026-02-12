/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_HOOKS_H
#define _ALE_HOOKS_H

#include <sol/sol.hpp>
#include <unordered_map>
#include <vector>

namespace ALE::Hooks
{
    /**
     * @enum PlayerEvent
     * @brief Player-specific event types
     *
     * Triggered by PlayerHooks when player state changes occur.
     */
    enum class PlayerEvent : uint32
    {
        ON_CHARACTER_CREATE                     =     1,        // (event, player)
        ON_CHARACTER_DELETE                     =     2,        // (event, guid)
        ON_LOGIN                                =     3,        // (event, player)
        ON_LOGOUT                               =     4,        // (event, player)
        ON_SPELL_CAST                           =     5,        // (event, player, spell, skipCheck)
        ON_KILL_PLAYER                          =     6,        // (event, player, victim)
        ON_KILL_CREATURE                        =     7,        // (event, player, creature)
        ON_KILLED_BY_CREATURE                   =     8,        // (event, player, creature)
        ON_DUEL_REQUEST                         =     9,        // (event, target, challenger)
        ON_DUEL_START                           =    10,        // (event, player1, player2)
        ON_DUEL_END                             =    11,        // (event, winner, loser, type)
        ON_GIVE_XP                              =    12,        // (event, player, amount, victim, xpSource) - Can return new XP amount
        ON_LEVEL_CHANGE                         =    13,        // (event, player, oldLevel)
        ON_MONEY_CHANGE                         =    14,        // (event, player, amount) - Can return new money amount
        ON_REPUTATION_CHANGE                    =    15,        // (event, player, factionID, standing, incremental) - Can return new standing -> if standing == -1, it will prevent default action (rep gain)
        ON_FREE_TALENT_POINTS_CHANGED           =    16,        // (event, player, points)
        ON_TALENTS_RESET                        =    17,        // (event, player, noCost)
        ON_CHAT                                 =    18,        // (event, player, type, lang, msg) - Can return false, newMessage
        ON_WHISPER                              =    19,        // (event, player, msg, lang, receiver) - Can return false, newMessage
        ON_GROUP_CHAT                           =    20,        // (event, player, msg, lang, channel) - Can return false, newMessage
        ON_GUILD_CHAT                           =    21,        // (event, player, msg, lang, guild) - Can return false, newMessage
        ON_CHANNEL_CHAT                         =    22,        // (event, player, msg, lang, channel) - Can return false, newMessage
        ON_EMOTE                                =    23,        // (event, player, emote) - Not triggered on any known emote
        ON_TEXT_EMOTE                           =    24,        // (event, player, textEmote, emoteNum, guid)
        ON_SAVE                                 =    25,        // (event, player)
        ON_BIND_TO_INSTANCE                     =    26,        // (event, player, difficulty, mapid, permanent)
        ON_UPDATE_ZONE                          =    27,        // (event, player, newZone, newArea)
        ON_MAP_CHANGED                          =    28,        // (event, player)
        ON_EQUIP_ITEM                           =    29,        // (event, player, item, bag, slot, update)
        ON_FIRST_LOGIN                          =    30,        // (event, player)
        ON_CAN_USE_ITEM                         =    31,        // (event, player, item, bag, slot) - Can return InventoryResult enum value
        ON_LOOT_ITEM                            =    32,        // (event, player, item, count, lootguid)
        ON_ENTER_COMBAT                         =    33,        // (event, player, enemy)
        ON_LEAVE_COMBAT                         =    34,        // (event, player)
        ON_CAN_REPOP_AT_GRAVEYARD               =    35,        // (event, player) - Can return false
        ON_RESURRECT                            =    36,        // (event, player, retore_percent, applySickness)
        // ON_LOOT_MONEY                        =    37,        // (event, player, amount) - Can return new money amount
        ON_QUEST_ABANDON                        =    38,        // (event, player, quest)
        ON_LEARN_TALENTS                        =    39,        // (event, player, talentId, talentRank, spellid)
        ON_CALCULATE_TALENTS_POINTS             =    40,        // (event, player, talentPointsForLevel) - Can return new talent points amount
        // UNUSED                               =    41,        // (event, player)
        ON_COMMAND                              =    42,        // (event, player, command, chatHandler) - player is nil if command used from console. Can return false
        // ON_PET_ADDED_TO_WORLD                  =    43,        // (event, player, pet)
        ON_LEARN_SPELL                          =    44,        // (event, player, spellId)
        ON_ACHIEVEMENT_COMPLETE                 =    45,        // (event, player, achievement)
        ON_FFA_PVP_STATE_UPDATE                 =    46,        // (event, player, isFlaggedForFfaPvp)
        ON_UPDATE_AREA                          =    47,        // (event, player, oldArea, newArea)
        ON_CAN_INIT_TRADE                       =    48,        // (event, player, target) - Can return false to prevent the trade
        ON_CAN_SEND_MAIL                        =    49,        // (event, player, receiverGuid, mailbox, subject, body, money, cod, item) - Can return false to prevent sending the mail
        ON_CAN_JOIN_LFG                         =    50,        // (event, player, roles, dungeons, comment) - Can return false to prevent queueing
        ON_QUEST_REWARD_ITEM                    =    51,        // (event, player, item, count)
        ON_CREATE_ITEM                          =    52,        // (event, player, item, count)
        ON_STORE_NEW_ITEM                       =    53,        // (event, player, item, count)
        ON_COMPLETE_QUEST                       =    54,        // (event, player, quest)
        ON_CAN_GROUP_INVITE                     =    55,        // (event, player, memberName) - Can return false to prevent the inviting
        ON_GROUP_ROLL_REWARD_ITEM               =    56,        // (event, player, item, count, voteType, roll)
        ON_BG_DESERTION                         =    57,        // (event, player, bgType)
        ON_PET_KILLED_CREATURE                  =    58,        // (event, player, creature)
        ON_CAN_RESURRECT                        =    59,        // (event, player) - Can return false to prevent resurrection
        ON_CAN_UPDATE_SKILL                     =    60,        // (event, player, skill_id) - Can return false to prevent skill up
        ON_BEFORE_UPDATE_SKILL                  =    61,        // (event, player, skill_id, value, max, step) - Can return new amount
        ON_UPDATE_SKILL                         =    62,        // (event, player, skill_id, value, max, step, new_value)
        // ON_QUEST_ACCEPTED                       =    63,        // (event, player, quest)
        ON_AURA_APPLY                           =    64,        // (event, player, aura)
        ON_HEAL                                 =    65,        // (event, player, target, heal_amount) - Can return new heal amount
        ON_DAMAGE                               =    66,        // (event, player, target, damage_amount) - Can return new damage amount
        ON_AURA_REMOVE                          =    67,        // (event, player, aura_application, remove_mode)
        ON_PERIODIC_DAMAGE_TICK                 =    68,        // (event, player, target, damage_amount, spellInfo) - Can return new damage amount
        ON_MELEE_DAMAGE                         =    69,        // (event, player, target, damage_amount) - Can return new damage amount
        ON_SPELL_DAMAGE_TAKEN                   =    70,        // (event, player, target, damage_amount, spellInfo) - Can return new damage amount
        ON_HEAL_RECEIVED                        =    71,        // (event, player, target, heal_amount, spellInfo) - Can return new heal amount
        ON_DEAL_DAMAGE                          =    72,        // (event, player, target, damage_amount, damage_type) - Can return new damage amount
        ON_RELEASED_GHOST                       =    73,        // (event, player)
        ON_BEFORE_ROLL_MELEE_OUTCOME            =    74,        // (event, player, outcome)
        ON_DISPLAY_ID_CHANGE                    =    75,        // (event, player, displayId)
        ON_DEATH                                =    76,        // (event, player) - Triggered when player dies
        ON_SET_SHAPESHIFT_FORM                  =    77,        // (event, player, form)
        ON_GOSSIP_SELECT                        =    78,        // (event, player, menu_id, sender, action, code)
        ON_BEFORE_DURABILITY_REPAIR             =    79,        // (event, player, npcGUID, itemGUID, discountMod, guildBank) - Can return new discountMod
        ON_SEND_INITIAL_PACKETS                 =    80,        // (event, player, packet)
        ON_CAN_FLY_IN_ZONE                      =    81,        // (event, player, mapId, zoneId, spellInfo) - Can return false
        ON_PVP_FLAG_CHANGE                      =    82,        // (event, player, state)
        ON_AFTER_SPEC_SLOT_CHANGED              =    83,        // (event, player, newSlot)
        ON_BEFORE_LOOT_MONEY                    =    84,        // (event, player, loot)
        ON_REPUTATION_RANK_CHANGE               =    85,        // (event, player, factionID, newRank, oldRank, increased)
        ON_GIVE_REPUTATION                      =    86,        // (event, player, factionID, amount, repSource) - Can return new amount
        ON_FORGOT_SPELL                         =    87,        // (event, player, spellID)
        ON_BEFORE_SEND_CHAT_MESSAGE             =    88,        // (event, player, type, lang, msg)
        ON_BEFORE_UPDATE                        =    89,        // (event, player, diff)
        ON_LOAD_FROM_DB                         =    90,        // (event, player)
        ON_BEFORE_LOGOUT                        =    91,        // (event, player)
        ON_FAILED_DELETE                        =    92,        // (event, guid, accountId)
        ON_BEFORE_TELEPORT                      =    93,        // (event, player, mapid, x, y, z, orientation, options, target) - Can return false
        ON_UPDATE_FACTION                       =    94,        // (event, player)
        ON_ADD_TO_BATTLEGROUND                  =    95,        // (event, player, bg)
        ON_QUEUE_RANDOM_DUNGEON                 =    96,        // (event, player, rDungeonId) - Can return new rDungeonId
        ON_REMOVE_FROM_BATTLEGROUND             =    97,        // (event, player, bg)
        ON_BEFORE_ACHIEVEMENT_COMPLETE          =    98,        // (event, player, achievement) - Can return false
        ON_BEFORE_CRITERIA_PROGRESS             =    99,        // (event, player, criteria) - Can return false
        ON_CRITERIA_PROGRESS                    =   100,        // (event, player, criteria)
        ON_ACHIEVEMENT_SAVE                     =   101,        // (event, player, achiId, achiData)
        ON_CRITERIA_SAVE                        =   102,        // (event, player, critId, criteriaData)
        ON_BEING_CHARMED                        =   103,        // (event, player, charmer, oldFactionId, newFactionId)
        ON_AFTER_SET_VISIBLE_ITEM_SLOT          =   104,        // (event, player, slot, item)
        ON_AFTER_MOVE_ITEM_FROM_INVENTORY       =   105,        // (event, player, item, bag, slot, update)
        ON_UNEQUIP_ITEM                         =   106,        // (event, player, item)
        ON_JOIN_BG                              =   107,        // (event, player)
        ON_JOIN_ARENA                           =   108,        // (event, player)
        ON_GET_MAX_PERSONAL_ARENA_RATING        =   109,        // (event, player, minSlot, maxArenaRating) - Can return new maxArenaRating
        ON_BEFORE_FILL_QUEST_LOOT_ITEM          =   110,        // (event, player, item)
        ON_CAN_PLACE_AUCTION_BID                =   111,        // (event, player, auction) - Can return false
        ON_BEFORE_OPEN_ITEM                     =   112,        // (event, player, item) - Can return false
        ON_SET_MAX_LEVEL                        =   113,        // (event, player, maxPlayerLevel) - Can return new maxPlayerLevel
        ON_CAN_JOIN_BG_QUEUE                    =   114,        // (event, player, battlemasterGuid, bgTypeId, joinAsGroup, err) - Can return false
        ON_SHOULD_REWARD_MONEY                  =   115,        // (event, player) - Can return false
        ON_BEFORE_TEMP_SUMMON                   =   116,        // (event, player, tempSummon, duration) - Can return new duration
        ON_BEFORE_GUARDIAN_INIT                 =   117,        // (event, player, guardian, cinfo, petType)
        ON_AFTER_GUARDIAN_INIT                  =   118,        // (event, player, guardian)
        ON_BEFORE_LOAD_PET                      =   119,        // (event, player, petEntry, petNumber, current, forceLoadFromDB)
        ON_BEFORE_BUY_ITEM                      =   120,        // (event, player, vendorGuid, vendorSlot, item, count, bag, slot) - Can return new item
        ON_AFTER_STORE_OR_EQUIP                 =   121,        // (event, player, vendorSlot, item, count, bag, slot, proto, vendor, crItem, store)
        ON_AFTER_UPDATE_MAX_POWER               =   122,        // (event, player, power, value)
        ON_AFTER_UPDATE_MAX_HEALTH              =   123,        // (event, player, value) - Can return new value
        ON_BEFORE_UPDATE_ATTACK_POWER           =   124,        // (event, player, level, val2, ranged)
        ON_AFTER_UPDATE_ATTACK_POWER            =   125,        // (event, player, level, base_attPower, attPowerMod, attPowerMultiplier, ranged)
        ON_BEFORE_INIT_TALENT                   =   126,        // (event, player, level, talentPointsForLevel)
        ON_BEFORE_QUEST_COMPLETE                =   127,        // (event, player, questId) - Can return false
        ON_QUEST_COMPUTE_XP                     =   128,        // (event, player, quest, xpValue) - Can return new xpValue
        ON_BEFORE_STORE_OR_EQUIP                =   129,        // (event, player, vendorSlot, item, count, bag, slot, proto, vendor, crItem, store) - Can return new item
        ON_CAN_JOIN_ARENA_QUEUE                 =   130,        // (event, player, battlemasterGuid, arenaSlot, bgTypeId, joinAsGroup, isRated, err) - Can return false
        ON_CAN_BATTLEFIELD_PORT                 =   131,        // (event, player, arenaType, bgTypeId, action) - Can return false
        ON_CAN_GROUP_ACCEPT                     =   132,        // (event, player, group) - Can return false
        ON_CAN_SELL_ITEM                        =   133,        // (event, player, item, creature) - Can return false
        ON_CAN_SEND_ERROR_LOOTED                =   134,        // (event, player) - Can return false
        ON_AFTER_CREATURE_LOOT                  =   135,        // (event, player)
        ON_AFTER_CREATURE_LOOT_MONEY            =   136,        // (event, player)
        ON_PETITION_BUY                         =   137,        // (event, player, creature, charterId, cost, type)
        ON_PETITION_SHOW_LIST                   =   138,        // (event, player, creature, charterEntry, charterDisplayId, charterCost)
        ON_REWARD_KILL_REWARDER                 =   139,        // (event, player, rewarder, isDungeon, rate) - Can return new rate
        ON_CAN_GIVE_MAIL_REWARD                 =   140,        // (event, player, level) - Can return false
        ON_DELETE_FROM_DB                       =   141,        // (event, transaction, guid)
        ON_IS_CLASS                             =   142,        // (event, player, class, context)
        ON_GET_MAX_SKILL_VALUE                  =   143,        // (event, player, skill, result, isPure) - Can return new result
        ON_HAS_ACTIVE_POWER_TYPE                =   144,        // (event, player, power) - Can return false
        ON_UPDATE_GATHERING_SKILL               =   145,        // (event, player, skillId, currentLevel, gray, green, yellow, gain) - Can return new gain
        ON_UPDATE_CRAFTING_SKILL                =   146,        // (event, player, skill, currentLevel, gain) - Can return new gain
        ON_UPDATE_FISHING_SKILL                 =   147,        // (event, player, skill, zoneSkill, chance, roll) - Can return false
        ON_CAN_AREA_EXPLORE_OUTDOOR             =   148,        // (event, player) - Can return false
        ON_VICTIM_REWARD_BEFORE                 =   149,        // (event, player, victim, killerTitle, victimRank)
        ON_VICTIM_REWARD_AFTER                  =   150,        // (event, player, victim, killerTitle, victimRank, honor)
        ON_CUSTOM_SCALING_STAT_BEFORE           =   151,        // (event, player, proto, slot, apply, value) - Can return new value
        ON_CUSTOM_SCALING_STAT                  =   152,        // (event, player, proto, statType, val, itemProtoStatNumber, scalingStatValue, ssv)
        ON_APPLY_ITEM_MODS_BEFORE               =   153,        // (event, player, slot, apply, itemProtoStatNumber, statType, val) - Can return new val
        ON_APPLY_ENCHANT_ITEM_MODS_BEFORE       =   154,        // (event, player, item, slot, apply, spellId, amount) - Can return new amount
        ON_APPLY_WEAPON_DAMAGE                  =   155,        // (event, player, slot, proto, minDamage, maxDamage, damageIndex)
        ON_CAN_ARMOR_DAMAGE_MODIFIER            =   156,        // (event, player) - Can return false
        ON_GET_FERAL_AP_BONUS                   =   157,        // (event, player, feralBonus, dpsMod, proto, ssv) - Can return new feralBonus
        ON_CAN_APPLY_WEAPON_AURA_DAMAGE         =   158,        // (event, player, item, attackType, aura, apply) - Can return false
        ON_CAN_APPLY_EQUIP_SPELL                =   159,        // (event, player, spellInfo, item, apply, formChange) - Can return false
        ON_CAN_APPLY_EQUIP_SPELLS_ITEM_SET      =   160,        // (event, player, eff) - Can return false
        ON_CAN_CAST_ITEM_COMBAT_SPELL           =   161,        // (event, player, target, attackType, procVictim, procEx, item, proto) - Can return false
        ON_CAN_CAST_ITEM_USE_SPELL              =   162,        // (event, player, item, targets, castCount, glyphIndex) - Can return false
        ON_APPLY_AMMO_BONUSES                   =   163,        // (event, player, proto, currentAmmoDPS) - Can return new currentAmmoDPS
        ON_CAN_EQUIP_ITEM                       =   164,        // (event, player, slot, dest, item, swap, notLoading) - Can return false
        ON_CAN_UNEQUIP_ITEM                     =   165,        // (event, player, pos, swap) - Can return false
        ON_CAN_SAVE_EQUIP_NEW_ITEM              =   166,        // (event, player, item, pos, update) - Can return false
        ON_CAN_APPLY_ENCHANTMENT                =   167,        // (event, player, item, slot, apply, applyDur, ignoreCondition) - Can return false
        ON_GET_QUEST_RATE                       =   168,        // (event, player, rate) - Can return new rate
        ON_PASSED_QUEST_MONSTER_CREDIT          =   169,        // (event, player, qinfo, entry, realEntry, guid) - Can return false
        ON_CHECK_ITEM_SLOT_LOAD                 =   170,        // (event, player, item, slot, err, dest) - Can return false
        ON_NOT_AVOID_SATISFY                    =   171,        // (event, player, ar, targetMap, report) - Can return false
        ON_NOT_VISIBLE_GLOBALLY                 =   172,        // (event, player, target) - Can return false
        ON_GET_ARENA_PERSONAL_RATING            =   173,        // (event, player, slot, result) - Can return new result
        ON_GET_ARENA_TEAM_ID                    =   174,        // (event, player, slot, result) - Can return new result
        ON_IS_FFA_PVP                           =   175,        // (event, player, result) - Can return new result
        ON_IS_PVP                               =   176,        // (event, player, result) - Can return new result
        ON_GET_MAX_SKILL_FOR_LEVEL              =   177,        // (event, player, result) - Can return new result
        ON_NOT_SET_ARENA_TEAM_INFO              =   178,        // (event, player, slot, type, value) - Can return false
        ON_CAN_SET_TRADE_ITEM                   =   179,        // (event, player, item, tradeSlot) - Can return false
        ON_SET_SERVER_SIDE_VISIBILITY           =   180,        // (event, player, type, sec)
        ON_SET_SERVER_SIDE_VISIBILITY_DETECT    =   181,        // (event, player, type, sec)
        ON_BEFORE_CHOOSE_GRAVEYARD              =   182,        // (event, player, teamId, nearCorpse, graveyardOverride) - Can return new graveyardOverride
        ON_CAN_GIVE_LEVEL                       =   183,        // (event, player, newLevel) - Can return false
        ON_SEND_LIST_INVENTORY                  =   184,        // (event, player, vendorGuid, vendorEntry) - Can return new vendorEntry
        ON_CAN_ENTER_MAP                        =   185,        // (event, player, entry, instance, mapDiff, loginCheck) - Can return false
        ON_ANTICHEAT_SET_CAN_FLY                =   186,        // (event, player, apply)
        ON_ANTICHEAT_SET_UNDER_ACK_MOUNT        =   187,        // (event, player)
        ON_ANTICHEAT_SET_ROOT_ACK_UPD           =   188,        // (event, player)
        ON_ANTICHEAT_SET_JUMPING                =   189,        // (event, player, jump)
        ON_ANTICHEAT_UPDATE_MOVEMENT            =   190,        // (event, player, movementInfo)
        ON_ANTICHEAT_HANDLE_DOUBLE_JUMP         =   191,        // (event, player, mover) - Can return false
        ON_ANTICHEAT_CHECK_MOVEMENT             =   192,        // (event, player, movementInfo, mover, jump) - Can return false
        ON_UPDATE                               =   193         // (event, player, diff)
    };

    /**
     * @enum WorldEvent
     * @brief World-level event types
     *
     * Triggered by WorldHooks for server-wide events.
     */
    enum class WorldEvent : uint32
    {
        ON_UPDATE = 1,              // < World update tick
        ON_CONFIG_LOAD = 2,         // < Configuration reloaded
        ON_SHUTDOWN_INIT = 3,       // < Shutdown initiated
        ON_SHUTDOWN_CANCEL = 4,     // < Shutdown cancelled
        ON_STARTUP = 14,            // < Server startup complete
        ON_SHUTDOWN = 6             // < Server shutting down
    };

    /**
     * @enum WorldObjectEvent
     * @brief WorldObject lifecycle event types
     *
     * Triggered by WorldObjectHooks for GameObjects.
     */
    enum class WorldObjectEvent : uint32
    {
        ON_CREATE = 1,  // < GameObject created in world
        ON_DESTROY = 2, // < GameObject removed from world
        ON_UPDATE = 3,  // < GameObject updated
        ON_REPOP = 4    // < GameObject respawned
    };

    /**
     * @enum CreatureEvent
     * @brief Creature-specific event types (per entry)
     *
     * Triggered for specific creature entries.
     * Use RegisterCreatureEvent(entry, eventId, callback) in Lua.
     */
    enum class CreatureEvent : uint32
    {
        ON_HEAL = 1,                          // < Creature healed another unit
        ON_DAMAGE = 2,                        // < Creature dealt damage
        ON_AURA_APPLY = 3,                    // < Aura applied to creature
        ON_AURA_REMOVE = 4,                   // < Aura removed from creature
        ON_PERIODIC_DAMAGE_TICK = 5,          // < Periodic damage tick
        ON_MELEE_DAMAGE = 6,                  // < Melee damage dealt
        ON_SPELL_DAMAGE_TAKEN = 7,            // < Spell damage taken
        ON_HEAL_RECEIVED = 8,                 // < Heal received
        ON_DEAL_DAMAGE = 9,                   // < Damage dealt (DealDamage hook)
        ON_BEFORE_ROLL_MELEE_OUTCOME = 10,    // < Before melee outcome roll
        ON_DISPLAY_ID_CHANGE = 11,            // < Display ID changed
        ON_ENTER_EVADE_MODE = 12,             // < Creature entered evade mode
        ON_ENTER_COMBAT = 13,                 // < Creature entered combat
        ON_DEATH = 14,                        // < Creature died
        ON_SET_SHAPESHIFT_FORM = 15           // < Shapeshift form changed
    };

    /**
     * @enum AllCreatureEvent
     * @brief Global creature event types (all creatures)
     *
     * Triggered for ALL creatures regardless of entry.
     * Use RegisterAllCreatureEvent(eventId, callback) in Lua.
     */
    enum class AllCreatureEvent : uint32
    {
        ON_HEAL = 1,                          // < Any creature healed another unit
        ON_DAMAGE = 2,                        // < Any creature dealt damage
        ON_AURA_APPLY = 3,                    // < Aura applied to any creature
        ON_AURA_REMOVE = 4,                   // < Aura removed from any creature
        ON_PERIODIC_DAMAGE_TICK = 5,          // < Periodic damage tick
        ON_MELEE_DAMAGE = 6,                  // < Melee damage dealt
        ON_SPELL_DAMAGE_TAKEN = 7,            // < Spell damage taken
        ON_HEAL_RECEIVED = 8,                 // < Heal received
        ON_DEAL_DAMAGE = 9,                   // < Damage dealt (DealDamage hook)
        ON_BEFORE_ROLL_MELEE_OUTCOME = 10,    // < Before melee outcome roll
        ON_DISPLAY_ID_CHANGE = 11,            // < Display ID changed
        ON_ENTER_EVADE_MODE = 12,             // < Creature entered evade mode
        ON_ENTER_COMBAT = 13,                 // < Creature entered combat
        ON_DEATH = 14,                        // < Creature died
        ON_SET_SHAPESHIFT_FORM = 15           // < Shapeshift form changed
    };
}

#endif // _ALE_HOOKS_H