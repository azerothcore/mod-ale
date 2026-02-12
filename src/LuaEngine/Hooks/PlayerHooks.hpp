/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_PLAYER_HOOKS_H
#define _ALE_PLAYER_HOOKS_H

#include "ScriptMgr.h"
#include "Player.h"
#include "Guild.h"
#include "Group.h"
#include "Channel.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "Creature.h"
#include "Item.h"
#include "Battleground.h"
#include "QuestDef.h"
#include "DBCStructure.h"

#include "TimedEventManager.h"
#include "EventManager.h"
#include "ALEManager.h"
#include "ALEConfig.h"

namespace ALE::Hooks
{
    /**
     * @brief Trigger player event helper (template implementation)
     *
     * Converts enum to uint32 and forwards to EventManager.
     *
     * @tparam Args Variadic argument types
     * @param eventType PlayerEvent enum value
     * @param args Arguments to forward to Lua callbacks
     */
    template<typename... Args>
    void TriggerPlayerEvent(Hooks::PlayerEvent eventType, Args&&... args)
    {
        // Cast enum to uint32
        uint32 eventId = static_cast<uint32>(eventType);

        // Trigger event
        Core::EventManager::GetInstance().TriggerGlobalEvent(
            eventType,
            eventId,
            std::forward<Args>(args)...
        );
    }

    class CommandHooks : public CommandSC
    {
    public:
        CommandHooks() : CommandSC("ALE_CommandHooks") { }

        bool OnTryExecuteCommand(ChatHandler& handler, std::string_view cmdStr) override
        {
            std::string text = std::string(cmdStr).c_str();
            Player* player = handler.IsConsole() ? nullptr : handler.GetSession()->GetPlayer();

            // Check for "reload ale" command (admin only)
            if (!player || player->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR)
            {
                std::string command = text;
                std::transform(command.begin(), command.end(), command.begin(), ::tolower);

                if (command.find("reload ale") == 0)
                {
                    if (ALE::Core::ALEManager::GetInstance().Reload())
                    {
                        if (ALEConfig::GetInstance().IsPlayerAnnounceReloadEnabled())
                        {
                            // Todo: Send Message to all player
                        }
                    }

                    return false; // Prevent command from continuing
                }
            }

            // Trigger ON_COMMAND event for Lua scripts
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_COMMAND);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_COMMAND,
                false,
                eventId,
                player,
                handler,
                text
            );
        }
    };

    /**
     * @class PlayerHooks
     * @brief AzerothCore player event interceptor for ALE Lua system
     * @note Registered in ALEScriptLoader via sScriptMgr->AddScript<PlayerHooks>()
     */
    class PlayerHooks : public PlayerScript
    {
    public:
        /**
         * @brief Constructor - registers PlayerHooks with AzerothCore
         */
        PlayerHooks() : PlayerScript("ALE_PlayerHooks") { }

        /**
         * @brief Player login event (PlayerEvent::ON_LOGIN (ID = 2))
         *
         * Called when player finishes loading into world.
         * Lua signature: function(eventId, player)
         *
         * @param player
         */
        void OnPlayerLogin(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LOGIN, player);
        }

        /**
         * @brief Player logout event (PlayerEvent::ON_LOGOUT (ID = 3))
         *
         * Called when player disconnects.
         * Lua signature: function(eventId, player)
         *
         * @param player
         */
        void OnPlayerLogout(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LOGOUT, player);
        }

        /**
         * @brief Money change event (PlayerEvent::ON_MONEY_CHANGE (ID = 6))
         *
         * Called before player's money changes.
         * Lua signature: function(eventId, player, amount)
         *
         * @param player
         * @param amount
         */
        void OnPlayerMoneyChanged(Player* player, int32& amount) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_MONEY_CHANGE);
            amount = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_MONEY_CHANGE,
                amount,
                eventId,
                player,
                amount
            );
        }

        /**
         * @brief XP gain event (PlayerEvent::ON_GIVE_XP (ID = 7))
         *
         * Called when player is about to receive XP.
         * Lua can modify amount by reference.
         *
         * Lua signature: function(eventId, player, amount, victim, xpSource)
         *
         * @param player
         * @param amount
         * @param victim
         * @param xpSource
         */
        void OnPlayerGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GIVE_XP);
            amount = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_GIVE_XP,
                amount,
                eventId,
                player,
                amount,
                victim,
                xpSource
            );
        }

        /**
         * @brief Player update tick event (triggers ON_UPDATE)
         *
         * Called every world tick (typically ~100ms) per player.
         *
         * Lua signature: function(eventId, player, diff)
         *
         * @param player Player being updated
         * @param diff Time delta since last update (milliseconds)
         */
        void OnPlayerUpdate(Player* player, uint32 diff) override
        {
            // Early exit: player not valid or not in world
            if (!player || !player->IsInWorld())
                return;

            auto& stateMgr = Core::StateManager::GetInstance();
            Core::TimedEventManager* mgr = stateMgr.GetTimedEventManager(-1);

            // Execute timed events for this player
            if (mgr)
                mgr->UpdateObjectEvents(player, diff);

            TriggerPlayerEvent(Hooks::PlayerEvent::ON_UPDATE, player, diff);
        }

        void OnPlayerResurrect(Player* player, float restore_percent, bool applySickness) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_RESURRECT, player, restore_percent, applySickness);
        }

        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
        {
            if (type != CHAT_MSG_SAY && type != CHAT_MSG_YELL && type != CHAT_MSG_EMOTE)
                return true;

            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CHAT);
            auto [allowed, modifiedMsg] = Core::EventManager::GetInstance()
                .TriggerGlobalEventWithReturn<std::tuple<bool, std::string>>(
                    Hooks::PlayerEvent::ON_CHAT,
                    std::make_tuple(true, msg),
                    eventId,
                    player,
                    type,
                    lang,
                    msg
                );

            if (!allowed)
                return false;

            msg = modifiedMsg;
            return true;
        }

        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* target) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_WHISPER);
            auto [allowed, modifiedMsg] = Core::EventManager::GetInstance()
                .TriggerGlobalEventWithReturn<std::tuple<bool, std::string>>(
                    Hooks::PlayerEvent::ON_WHISPER,
                    std::make_tuple(true, msg),
                    eventId,
                    player,
                    type,
                    lang,
                    msg,
                    target
                );

            if (!allowed)
                return false;

            msg = modifiedMsg;
            return true;
        }

        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GROUP_CHAT);
            auto [allowed, modifiedMsg] = Core::EventManager::GetInstance()
                .TriggerGlobalEventWithReturn<std::tuple<bool, std::string>>(
                    Hooks::PlayerEvent::ON_GROUP_CHAT,
                    std::make_tuple(true, msg),
                    eventId,
                    player,
                    type,
                    lang,
                    msg,
                    group
                );

            if (!allowed)
                return false;

            msg = modifiedMsg;
            return true;
        }

        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GUILD_CHAT);
            auto [allowed, modifiedMsg] = Core::EventManager::GetInstance()
                .TriggerGlobalEventWithReturn<std::tuple<bool, std::string>>(
                    Hooks::PlayerEvent::ON_GUILD_CHAT,
                    std::make_tuple(true, msg),
                    eventId,
                    player,
                    type,
                    lang,
                    msg,
                    guild
                );

            if (!allowed)
                return false;

            msg = modifiedMsg;
            return true;
        }

        bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CHANNEL_CHAT);
            auto [allowed, modifiedMsg] = Core::EventManager::GetInstance()
                .TriggerGlobalEventWithReturn<std::tuple<bool, std::string>>(
                    Hooks::PlayerEvent::ON_CHANNEL_CHAT,
                    std::make_tuple(true, msg),
                    eventId,
                    player,
                    type,
                    lang,
                    msg,
                    channel
                );

            if (!allowed)
                return false;

            msg = modifiedMsg;
            return true;
        }

        void OnPlayerLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LOOT_ITEM, player, item, count, lootguid.GetRawValue());
        }

        void OnPlayerLearnTalents(Player* player, uint32 talentId, uint32 talentRank, uint32 spellid) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LEARN_TALENTS, player, talentId, talentRank, spellid);
        }

        bool OnPlayerCanUseItem(Player* player, ItemTemplate const* proto, InventoryResult& result) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_USE_ITEM);
            uint32 val = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_CAN_USE_ITEM,
                EQUIP_ERR_OK,
                eventId,
                player,
                proto,
                result
            );

            result = static_cast<InventoryResult>(val);
            return result == EQUIP_ERR_OK;
        }

        void OnPlayerEquip(Player* player, Item* it, uint8 bag, uint8 slot, bool update) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_EQUIP_ITEM, player, it, bag, slot, update);
        }

        void OnPlayerEnterCombat(Player* player, Unit* enemy) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ENTER_COMBAT, player, enemy);
        }

        void OnPlayerLeaveCombat(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LEAVE_COMBAT, player);
        }

        bool OnPlayerCanRepopAtGraveyard(Player* player) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_REPOP_AT_GRAVEYARD);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_REPOP_AT_GRAVEYARD,
                true,
                eventId,
                player
            );
        }

        void OnPlayerQuestAbandon(Player* player, uint32 questId) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_QUEST_ABANDON, player, questId);
        }

        void OnPlayerMapChanged(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_MAP_CHANGED, player);
        }

        void OnPlayerGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_GOSSIP_SELECT, player, menu_id, sender, action, "");
        }

        void OnPlayerGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, const char* code) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_GOSSIP_SELECT, player, menu_id, sender, action, code);
        }

        void OnPlayerPVPKill(Player* player, Player* victim) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_KILL_PLAYER, player, victim);
        }

        void OnPlayerCreatureKill(Player* player, Creature* creature) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_KILL_CREATURE, player, creature);
        }

        void OnPlayerKilledByCreature(Creature* creature, Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_KILLED_BY_CREATURE, player, creature);
        }

        void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LEVEL_CHANGE, player, oldLevel);
        }

        void OnPlayerFreeTalentPointsChanged(Player* player, uint32 points) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_FREE_TALENT_POINTS_CHANGED, player, points);
        }

        void OnPlayerTalentsReset(Player* player, bool noCost) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_TALENTS_RESET, player, noCost);
        }

        bool OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_REPUTATION_CHANGE);
            standing = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<int32>(
                Hooks::PlayerEvent::ON_REPUTATION_CHANGE,
                standing,
                eventId,
                player,
                factionID,
                standing,
                incremental
            );

            if (standing == -1)
                return false;

            return true;
        }

        void OnPlayerDuelRequest(Player* target, Player* challenger) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_DUEL_REQUEST, target, challenger);
        }

        void OnPlayerDuelStart(Player* player1, Player* player2) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_DUEL_START, player1, player2);
        }

        void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_DUEL_END, winner, loser, type);
        }

        void OnPlayerEmote(Player* player, uint32 emote) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_EMOTE, player, emote);
        }

        void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_TEXT_EMOTE, player, textEmote, emoteNum, guid.GetCounter());
        }

        void OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_SPELL_CAST, player, spell, skipCheck);
        }

        void OnPlayerCreate(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_CHARACTER_CREATE, player);
        }

        void OnPlayerSave(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_SAVE, player);
        }

        void OnPlayerDelete(ObjectGuid guid, uint32 accountId) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_CHARACTER_DELETE, guid.GetCounter(), accountId);
        }

        void OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BIND_TO_INSTANCE, player, difficulty, mapid, permanent);
        }

        void OnPlayerUpdateArea(Player* player, uint32 oldArea, uint32 newArea) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_UPDATE_AREA, player, oldArea, newArea);
        }

        void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_UPDATE_ZONE, player, newZone, newArea);
        }

        void OnPlayerFirstLogin(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_FIRST_LOGIN, player);
        }

        void OnPlayerLearnSpell(Player* player, uint32 spellId) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LEARN_SPELL, player, spellId);
        }

        void OnPlayerAchievementComplete(Player* player, AchievementEntry const* achievement) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ACHIEVEMENT_COMPLETE, player, achievement);
        }

        void OnPlayerFfaPvpStateUpdate(Player* player, bool IsFlaggedForFfaPvp) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_FFA_PVP_STATE_UPDATE, player, IsFlaggedForFfaPvp);
        }

        bool OnPlayerCanInitTrade(Player* player, Player* target) override
        {
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_INIT_TRADE,
                true,
                Hooks::PlayerEvent::ON_CAN_INIT_TRADE,
                player,
                target
            );
        }

        bool OnPlayerCanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint32 money, uint32 cod, Item* item) override
        {
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_SEND_MAIL,
                true,
                Hooks::PlayerEvent::ON_CAN_SEND_MAIL,
                player,
                receiverGuid,
                mailbox,
                subject,
                body,
                money,
                cod,
                item
            );
        }

        bool OnPlayerCanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, const std::string& comment) override
        {
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_JOIN_LFG,
                true,
                Hooks::PlayerEvent::ON_CAN_JOIN_LFG,
                player,
                roles,
                dungeons,
                comment
            );
        }

        void OnPlayerQuestRewardItem(Player* player, Item* item, uint32 count) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_QUEST_REWARD_ITEM, player, item, count);
        }

        void OnPlayerGroupRollRewardItem(Player* player, Item* item, uint32 count, RollVote voteType, Roll* roll) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_GROUP_ROLL_REWARD_ITEM, player, item, count, voteType, roll);
        }

        void OnPlayerCreateItem(Player* player, Item* item, uint32 count) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_CREATE_ITEM, player, item, count);
        }

        void OnPlayerStoreNewItem(Player* player, Item* item, uint32 count) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_STORE_NEW_ITEM, player, item, count);
        }

        void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_COMPLETE_QUEST, player, quest);
        }

        bool OnPlayerCanGroupInvite(Player* player, std::string& memberName) override
        {
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_GROUP_INVITE,
                true,
                Hooks::PlayerEvent::ON_CAN_GROUP_INVITE,
                player,
                memberName
            );
        }

        void OnPlayerBattlegroundDesertion(Player* player, const BattlegroundDesertionType type) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BG_DESERTION, player, type);
        }

        void OnPlayerCreatureKilledByPet(Player* player, Creature* killed) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_PET_KILLED_CREATURE, player, killed);
        }

        bool OnPlayerCanUpdateSkill(Player* player, uint32 skill_id) override
        {
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_UPDATE_SKILL,
                true,
                Hooks::PlayerEvent::ON_CAN_UPDATE_SKILL,
                player,
                skill_id
            );
        }

        void OnPlayerBeforeUpdateSkill(Player* player, uint32 skill_id, uint32& value, uint32 max, uint32 step) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_UPDATE_SKILL);
            value = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_BEFORE_UPDATE_SKILL,
                value,
                eventId,
                player,
                skill_id,
                value,
                max,
                step
            );
        }

        void OnPlayerUpdateSkill(Player* player, uint32 skill_id, uint32 value, uint32 max, uint32 step, uint32 new_value) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_UPDATE_SKILL, player, skill_id, value, max, step, new_value);
        }

        bool OnPlayerCanResurrect(Player* player) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_RESURRECT);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_RESURRECT,
                true,
                eventId,
                player
            );
        }

        void OnPlayerReleasedGhost(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_RELEASED_GHOST, player);
        }

        void OnPlayerBeforeDurabilityRepair(Player* player, ObjectGuid npcGUID, ObjectGuid itemGUID, float& discountMod, uint8 guildBank) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_DURABILITY_REPAIR);
            discountMod = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<float>(
                Hooks::PlayerEvent::ON_BEFORE_DURABILITY_REPAIR,
                discountMod,
                eventId,
                player,
                npcGUID,
                itemGUID,
                discountMod,
                guildBank
            );
        }

        void OnPlayerSendInitialPacketsBeforeAddToMap(Player* player, WorldPacket& data) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_SEND_INITIAL_PACKETS, player, &data);
        }

        void OnPlayerCalculateTalentsPoints(Player const* player, uint32& talentPointsForLevel) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CALCULATE_TALENTS_POINTS);
            talentPointsForLevel = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_CALCULATE_TALENTS_POINTS,
                talentPointsForLevel,
                eventId,
                player,
                talentPointsForLevel
            );
        }

        bool OnPlayerCanFlyInZone(Player* player, uint32 mapId, uint32 zoneId, SpellInfo const* bySpell) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_FLY_IN_ZONE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_FLY_IN_ZONE,
                false,
                eventId,
                player,
                mapId,
                zoneId,
                bySpell
            );
        }

        void OnPlayerPVPFlagChange(Player* player, bool state) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_PVP_FLAG_CHANGE, player, state);
        }

        void OnPlayerAfterSpecSlotChanged(Player* player, uint8 newSlot) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_SPEC_SLOT_CHANGED, player, newSlot);
        }

        void OnPlayerBeforeLootMoney(Player* player, Loot* loot) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_LOOT_MONEY, player, loot);
        }

        void OnPlayerReputationRankChange(Player* player, uint32 factionID, ReputationRank newRank, ReputationRank oldRank, bool increased) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_REPUTATION_RANK_CHANGE, player, factionID, newRank, oldRank, increased);
        }

        void OnPlayerGiveReputation(Player* player, int32 factionID, float& amount, ReputationSource repSource) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GIVE_REPUTATION);
            amount = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<float>(
                Hooks::PlayerEvent::ON_GIVE_REPUTATION,
                amount,
                eventId,
                player,
                factionID,
                amount,
                repSource
            );
        }

        void OnPlayerForgotSpell(Player* player, uint32 spellID) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_FORGOT_SPELL, player, spellID);
        }

        void OnPlayerBeforeSendChatMessage(Player* player, uint32& type, uint32& lang, std::string& msg) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_SEND_CHAT_MESSAGE, player, type, lang, msg);
        }

        void OnPlayerBeforeUpdate(Player* player, uint32 p_time) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_UPDATE, player, p_time);
        }

        void OnPlayerLoadFromDB(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_LOAD_FROM_DB, player);
        }

        void OnPlayerBeforeLogout(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_LOGOUT, player);
        }

        void OnPlayerFailedDelete(ObjectGuid guid, uint32 accountId) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_FAILED_DELETE, guid, accountId);
        }

        bool OnPlayerBeforeTeleport(Player* player, uint32 mapid, float x, float y, float z, float orientation, uint32 options, Unit* target) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_TELEPORT);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_BEFORE_TELEPORT,
                true,
                eventId,
                player,
                mapid,
                x,
                y,
                z,
                orientation,
                options,
                target
            );
        }

        void OnPlayerUpdateFaction(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_UPDATE_FACTION, player);
        }

        void OnPlayerAddToBattleground(Player* player, Battleground* bg) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ADD_TO_BATTLEGROUND, player, bg);
        }

        void OnPlayerQueueRandomDungeon(Player* player, uint32& rDungeonId) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_QUEUE_RANDOM_DUNGEON);
            rDungeonId = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_QUEUE_RANDOM_DUNGEON,
                rDungeonId,
                eventId,
                player,
                rDungeonId
            );
        }

        void OnPlayerRemoveFromBattleground(Player* player, Battleground* bg) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_REMOVE_FROM_BATTLEGROUND, player, bg);
        }

        bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* achievement) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_ACHIEVEMENT_COMPLETE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_BEFORE_ACHIEVEMENT_COMPLETE,
                true,
                eventId,
                player,
                achievement
            );
        }

        bool OnPlayerBeforeCriteriaProgress(Player* player, AchievementCriteriaEntry const* criteria) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_CRITERIA_PROGRESS);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_BEFORE_CRITERIA_PROGRESS,
                true,
                eventId,
                player,
                criteria
            );
        }

        void OnPlayerCriteriaProgress(Player* player, AchievementCriteriaEntry const* criteria) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_CRITERIA_PROGRESS, player, criteria);
        }

        void OnPlayerAchievementSave(CharacterDatabaseTransaction /*trans*/, Player* player, uint16 achiId, CompletedAchievementData achiData) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ACHIEVEMENT_SAVE, player, achiId, achiData);
        }

        void OnPlayerCriteriaSave(CharacterDatabaseTransaction /*trans*/, Player* player, uint16 critId, CriteriaProgress criteriaData) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_CRITERIA_SAVE, player, critId, criteriaData);
        }

        void OnPlayerBeingCharmed(Player* player, Unit* charmer, uint32 oldFactionId, uint32 newFactionId) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEING_CHARMED, player, charmer, oldFactionId, newFactionId);
        }

        void OnPlayerAfterSetVisibleItemSlot(Player* player, uint8 slot, Item* item) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_SET_VISIBLE_ITEM_SLOT, player, slot, item);
        }

        void OnPlayerAfterMoveItemFromInventory(Player* player, Item* it, uint8 bag, uint8 slot, bool update) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_MOVE_ITEM_FROM_INVENTORY, player, it, bag, slot, update);
        }

        void OnPlayerUnequip(Player* player, Item* it) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_UNEQUIP_ITEM, player, it);
        }

        void OnPlayerJoinBG(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_JOIN_BG, player);
        }

        void OnPlayerJoinArena(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_JOIN_ARENA, player);
        }

        void OnPlayerGetMaxPersonalArenaRatingRequirement(Player const* player, uint32 minSlot, uint32& maxArenaRating) const override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_MAX_PERSONAL_ARENA_RATING);
            maxArenaRating = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_GET_MAX_PERSONAL_ARENA_RATING,
                maxArenaRating,
                eventId,
                player,
                minSlot,
                maxArenaRating
            );
        }

        void OnPlayerBeforeFillQuestLootItem(Player* player, LootItem& item) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_FILL_QUEST_LOOT_ITEM, player, &item);
        }

        bool OnPlayerCanPlaceAuctionBid(Player* player, AuctionEntry* auction) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_PLACE_AUCTION_BID);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_PLACE_AUCTION_BID,
                true,
                eventId,
                player,
                auction
            );
        }

        bool OnPlayerBeforeOpenItem(Player* player, Item* item) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_OPEN_ITEM);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_BEFORE_OPEN_ITEM,
                true,
                eventId,
                player,
                item
            );
        }

        void OnPlayerSetMaxLevel(Player* player, uint32& maxPlayerLevel) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_SET_MAX_LEVEL);
            maxPlayerLevel = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_SET_MAX_LEVEL,
                maxPlayerLevel,
                eventId,
                player,
                maxPlayerLevel
            );
        }

        bool OnPlayerCanJoinInBattlegroundQueue(Player* player, ObjectGuid BattlemasterGuid, BattlegroundTypeId BGTypeID, uint8 joinAsGroup, GroupJoinBattlegroundResult& err) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_JOIN_BG_QUEUE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_JOIN_BG_QUEUE,
                true,
                eventId,
                player,
                BattlemasterGuid,
                BGTypeID,
                joinAsGroup,
                err // ref? complex object.
            );
        }

        bool OnPlayerShouldBeRewardedWithMoneyInsteadOfExp(Player* player) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_SHOULD_REWARD_MONEY);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_SHOULD_REWARD_MONEY,
                false,
                eventId,
                player
            );
        }

        void OnPlayerBeforeTempSummonInitStats(Player* player, TempSummon* tempSummon, uint32& duration) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_TEMP_SUMMON);
            duration = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_BEFORE_TEMP_SUMMON,
                duration,
                eventId,
                player,
                tempSummon,
                duration
            );
        }

        void OnPlayerBeforeGuardianInitStatsForLevel(Player* player, Guardian* guardian, CreatureTemplate const* cinfo, PetType& petType) override
        {
            // petType ref
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_GUARDIAN_INIT, player, guardian, cinfo, petType);
        }

        void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_GUARDIAN_INIT, player, guardian);
        }

        void OnPlayerBeforeLoadPetFromDB(Player* player, uint32& petentry, uint32& petnumber, bool& current, bool& forceLoadFromDB) override
        {
            // multiple refs
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_LOAD_PET, player, petentry, petnumber, current, forceLoadFromDB);
        }

        void OnPlayerBeforeBuyItemFromVendor(Player* player, ObjectGuid vendorguid, uint32 vendorslot, uint32& item, uint8 count, uint8 bag, uint8 slot) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_BUY_ITEM);
            item = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_BEFORE_BUY_ITEM,
                item,
                eventId,
                player,
                vendorguid,
                vendorslot,
                item,
                count,
                bag,
                slot
            );
        }

        void OnPlayerAfterStoreOrEquipNewItem(Player* player, uint32 vendorslot, Item* item, uint8 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_STORE_OR_EQUIP, player, vendorslot, item, count, bag, slot, pProto, pVendor, crItem, bStore);
        }

        void OnPlayerAfterUpdateMaxPower(Player* player, Powers& power, float& value) override
        {
             // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_UPDATE_MAX_POWER, player, power, value);
        }

        void OnPlayerAfterUpdateMaxHealth(Player* player, float& value) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_AFTER_UPDATE_MAX_HEALTH);
            value = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<float>(
                Hooks::PlayerEvent::ON_AFTER_UPDATE_MAX_HEALTH,
                value,
                eventId,
                player,
                value
            );
        }

        void OnPlayerBeforeUpdateAttackPowerAndDamage(Player* player, float& level, float& val2, bool ranged) override
        {
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_UPDATE_ATTACK_POWER, player, level, val2, ranged);
        }

        void OnPlayerAfterUpdateAttackPowerAndDamage(Player* player, float& level, float& base_attPower, float& attPowerMod, float& attPowerMultiplier, bool ranged) override
        {
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_UPDATE_ATTACK_POWER, player, level, base_attPower, attPowerMod, attPowerMultiplier, ranged);
        }

        void OnPlayerBeforeInitTalentForLevel(Player* player, uint8& level, uint32& talentPointsForLevel) override
        {
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_BEFORE_INIT_TALENT, player, level, talentPointsForLevel);
        }

        bool OnPlayerBeforeQuestComplete(Player* player, uint32 quest_id) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_QUEST_COMPLETE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_BEFORE_QUEST_COMPLETE,
                true,
                eventId,
                player,
                quest_id
            );
        }

        void OnPlayerQuestComputeXP(Player* player, Quest const* quest, uint32& xpValue) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_QUEST_COMPUTE_XP);
            xpValue = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_QUEST_COMPUTE_XP,
                xpValue,
                eventId,
                player,
                quest,
                xpValue
            );
        }

        void OnPlayerBeforeStoreOrEquipNewItem(Player* player, uint32 vendorslot, uint32& item, uint8 count, uint8 bag, uint8 slot, ItemTemplate const* pProto, Creature* pVendor, VendorItem const* crItem, bool bStore) override
        {
             // item ref
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_STORE_OR_EQUIP);
             item = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_BEFORE_STORE_OR_EQUIP,
                item,
                eventId,
                player,
                vendorslot,
                item,
                count,
                bag,
                slot,
                pProto,
                pVendor,
                crItem,
                bStore
            );
        }

        bool OnPlayerCanJoinInArenaQueue(Player* player, ObjectGuid BattlemasterGuid, uint8 arenaslot, BattlegroundTypeId BGTypeID, uint8 joinAsGroup, uint8 IsRated, GroupJoinBattlegroundResult& err) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_JOIN_ARENA_QUEUE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_JOIN_ARENA_QUEUE,
                true,
                eventId,
                player,
                BattlemasterGuid,
                arenaslot,
                BGTypeID,
                joinAsGroup,
                IsRated,
                err
            );
        }

        bool OnPlayerCanBattleFieldPort(Player* player, uint8 arenaType, BattlegroundTypeId BGTypeID, uint8 action) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_BATTLEFIELD_PORT);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_BATTLEFIELD_PORT,
                true,
                eventId,
                player,
                arenaType,
                BGTypeID,
                action
            );
        }

        bool OnPlayerCanGroupAccept(Player* player, Group* group) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_GROUP_ACCEPT);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_GROUP_ACCEPT,
                true,
                eventId,
                player,
                group
            );
        }

        bool OnPlayerCanSellItem(Player* player, Item* item, Creature* creature) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_SELL_ITEM);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_SELL_ITEM,
                true,
                eventId,
                player,
                item,
                creature
            );
        }

        bool OnPlayerCanSendErrorAlreadyLooted(Player* player) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_SEND_ERROR_LOOTED);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_SEND_ERROR_LOOTED,
                true,
                eventId,
                player
            );
        }

        void OnPlayerAfterCreatureLoot(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_CREATURE_LOOT, player);
        }

        void OnPlayerAfterCreatureLootMoney(Player* player) override
        {
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_AFTER_CREATURE_LOOT_MONEY, player);
        }

        void OnPlayerPetitionBuy(Player* player, Creature* creature, uint32& charterid, uint32& cost, uint32& type) override
        {
             // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_PETITION_BUY, player, creature, charterid, cost, type);
        }

        void OnPlayerPetitionShowList(Player* player, Creature* creature, uint32& CharterEntry, uint32& CharterDispayID, uint32& CharterCost) override
        {
            // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_PETITION_SHOW_LIST, player, creature, CharterEntry, CharterDispayID, CharterCost);
        }

        void OnPlayerRewardKillRewarder(Player* player, KillRewarder* rewarder, bool isDungeon, float& rate) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_REWARD_KILL_REWARDER);
             rate = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<float>(
                Hooks::PlayerEvent::ON_REWARD_KILL_REWARDER,
                rate,
                eventId,
                player,
                rewarder,
                isDungeon,
                rate
            );
        }

        bool OnPlayerCanGiveMailRewardAtGiveLevel(Player* player, uint8 level) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_GIVE_MAIL_REWARD);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_GIVE_MAIL_REWARD,
                true,
                eventId,
                player,
                level
            );
        }

        void OnPlayerDeleteFromDB(CharacterDatabaseTransaction trans, uint32 guid) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_DELETE_FROM_DB, trans, guid);
        }

        Optional<bool> OnPlayerIsClass(Player const* /*player*/, Classes /*unitClass*/, ClassContext /*context*/) override
        {
             return {};
        }

        void OnPlayerGetMaxSkillValue(Player* player, uint32 skill, int32& result, bool IsPure) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_MAX_SKILL_VALUE);
             result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<int32>(
                Hooks::PlayerEvent::ON_GET_MAX_SKILL_VALUE,
                result,
                eventId,
                player,
                skill,
                result,
                IsPure
            );
        }

        bool OnPlayerHasActivePowerType(Player const* player, Powers power) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_HAS_ACTIVE_POWER_TYPE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_HAS_ACTIVE_POWER_TYPE,
                false, // default false per ScriptMgr
                eventId,
                player,
                power
            );
        }

        void OnPlayerUpdateGatheringSkill(Player *player, uint32 skillId, uint32 currentLevel, uint32 gray, uint32 green, uint32 yellow, uint32 &gain) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_UPDATE_GATHERING_SKILL);
             gain = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_UPDATE_GATHERING_SKILL,
                gain,
                eventId,
                player,
                skillId,
                currentLevel,
                gray,
                green,
                yellow,
                gain
            );
        }

        void OnPlayerUpdateCraftingSkill(Player *player, SkillLineAbilityEntry const* skill, uint32 currentLevel, uint32& gain) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_UPDATE_CRAFTING_SKILL);
             gain = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_UPDATE_CRAFTING_SKILL,
                gain,
                eventId,
                player,
                skill,
                currentLevel,
                gain
            );
        }

        bool OnPlayerUpdateFishingSkill(Player* player, int32 skill, int32 zone_skill, int32 chance, int32 roll) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_UPDATE_FISHING_SKILL);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_UPDATE_FISHING_SKILL,
                true,
                eventId,
                player,
                skill,
                zone_skill,
                chance,
                roll
            );
        }

        bool OnPlayerCanAreaExploreAndOutdoor(Player* player) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_AREA_EXPLORE_OUTDOOR);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_AREA_EXPLORE_OUTDOOR,
                true,
                eventId,
                player
            );
        }

        void OnPlayerVictimRewardBefore(Player* player, Player* victim, uint32& killer_title, int32& victim_rank) override
        {
             // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_VICTIM_REWARD_BEFORE, player, victim, killer_title, victim_rank);
        }

        void OnPlayerVictimRewardAfter(Player* player, Player* victim, uint32& killer_title, int32& victim_rank, float& honor_f) override
        {
             // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_VICTIM_REWARD_AFTER, player, victim, killer_title, victim_rank, honor_f);
        }

        void OnPlayerCustomScalingStatValueBefore(Player* player, ItemTemplate const* proto, uint8 slot, bool apply, uint32& CustomScalingStatValue) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CUSTOM_SCALING_STAT_BEFORE);
             CustomScalingStatValue = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_CUSTOM_SCALING_STAT_BEFORE,
                CustomScalingStatValue,
                eventId,
                player,
                proto,
                slot,
                apply,
                CustomScalingStatValue
            );
        }

        void OnPlayerCustomScalingStatValue(Player* player, ItemTemplate const* proto, uint32& statType, int32& val, uint8 itemProtoStatNumber, uint32 ScalingStatValue, ScalingStatValuesEntry const* ssv) override
        {
             // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_CUSTOM_SCALING_STAT, player, proto, statType, val, itemProtoStatNumber, ScalingStatValue, ssv);
        }

        void OnPlayerApplyItemModsBefore(Player* player, uint8 slot, bool apply, uint8 itemProtoStatNumber, uint32 statType, int32& val) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_APPLY_ITEM_MODS_BEFORE);
             val = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<int32>(
                Hooks::PlayerEvent::ON_APPLY_ITEM_MODS_BEFORE,
                val,
                eventId,
                player,
                slot,
                apply,
                itemProtoStatNumber,
                statType,
                val
            );
        }

        void OnPlayerApplyEnchantmentItemModsBefore(Player* player, Item* item, EnchantmentSlot slot, bool apply, uint32 enchant_spell_id, uint32& enchant_amount) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_APPLY_ENCHANT_ITEM_MODS_BEFORE);
             enchant_amount = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_APPLY_ENCHANT_ITEM_MODS_BEFORE,
                enchant_amount,
                eventId,
                player,
                item,
                slot,
                apply,
                enchant_spell_id,
                enchant_amount
            );
        }

        void OnPlayerApplyWeaponDamage(Player* player, uint8 slot, ItemTemplate const* proto, float& minDamage, float& maxDamage, uint8 damageIndex) override
        {
             // multiple refs
             TriggerPlayerEvent(Hooks::PlayerEvent::ON_APPLY_WEAPON_DAMAGE, player, slot, proto, minDamage, maxDamage, damageIndex);
        }

        bool OnPlayerCanArmorDamageModifier(Player* player) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_ARMOR_DAMAGE_MODIFIER);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_ARMOR_DAMAGE_MODIFIER,
                true,
                eventId,
                player
            );
        }

        void OnPlayerGetFeralApBonus(Player* player, int32& feral_bonus, int32 dpsMod, ItemTemplate const* proto, ScalingStatValuesEntry const* ssv) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_FERAL_AP_BONUS);
             feral_bonus = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<int32>(
                Hooks::PlayerEvent::ON_GET_FERAL_AP_BONUS,
                feral_bonus,
                eventId,
                player,
                feral_bonus,
                dpsMod,
                proto,
                ssv
            );
        }

        bool OnPlayerCanApplyWeaponDependentAuraDamageMod(Player* player, Item* item, WeaponAttackType attackType, AuraEffect const* aura, bool apply) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_APPLY_WEAPON_AURA_DAMAGE);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_APPLY_WEAPON_AURA_DAMAGE,
                true,
                eventId,
                player,
                item,
                attackType,
                aura,
                apply
            );
        }

        bool OnPlayerCanApplyEquipSpell(Player* player, SpellInfo const* spellInfo, Item* item, bool apply, bool form_change) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_APPLY_EQUIP_SPELL);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_APPLY_EQUIP_SPELL,
                true,
                eventId,
                player,
                spellInfo,
                item,
                apply,
                form_change
            );
        }

        bool OnPlayerCanApplyEquipSpellsItemSet(Player* player, ItemSetEffect* eff) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_APPLY_EQUIP_SPELLS_ITEM_SET);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_APPLY_EQUIP_SPELLS_ITEM_SET,
                true,
                eventId,
                player,
                eff
            );
        }

        bool OnPlayerCanCastItemCombatSpell(Player* player, Unit* target, WeaponAttackType attType, uint32 procVictim, uint32 procEx, Item* item, ItemTemplate const* proto) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_CAST_ITEM_COMBAT_SPELL);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_CAST_ITEM_COMBAT_SPELL,
                true,
                eventId,
                player,
                target,
                attType,
                procVictim,
                procEx,
                item,
                proto
            );
        }

        bool OnPlayerCanCastItemUseSpell(Player* player, Item* item, SpellCastTargets const& targets, uint8 cast_count, uint32 glyphIndex) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_CAST_ITEM_USE_SPELL);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_CAST_ITEM_USE_SPELL,
                true,
                eventId,
                player,
                item,
                targets,
                cast_count,
                glyphIndex
            );
        }

        void OnPlayerApplyAmmoBonuses(Player* player, ItemTemplate const* proto, float& currentAmmoDPS) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_APPLY_AMMO_BONUSES);
             currentAmmoDPS = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<float>(
                Hooks::PlayerEvent::ON_APPLY_AMMO_BONUSES,
                currentAmmoDPS,
                eventId,
                player,
                proto,
                currentAmmoDPS
            );
        }

        bool OnPlayerCanEquipItem(Player* player, uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading) override
        {
             // dest is ref, but function returns bool.
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_EQUIP_ITEM);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_EQUIP_ITEM,
                true,
                eventId,
                player,
                slot,
                dest,
                pItem,
                swap,
                not_loading
            );
        }

        bool OnPlayerCanUnequipItem(Player* player, uint16 pos, bool swap) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_UNEQUIP_ITEM);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_UNEQUIP_ITEM,
                true,
                eventId,
                player,
                pos,
                swap
            );
        }

        bool OnPlayerCanSaveEquipNewItem(Player* player, Item* item, uint16 pos, bool update) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_SAVE_EQUIP_NEW_ITEM);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_SAVE_EQUIP_NEW_ITEM,
                true,
                eventId,
                player,
                item,
                pos,
                update
            );
        }

        bool OnPlayerCanApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool apply_dur, bool ignore_condition) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_APPLY_ENCHANTMENT);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_APPLY_ENCHANTMENT,
                true,
                eventId,
                player,
                item,
                slot,
                apply,
                apply_dur,
                ignore_condition
            );
        }

        void OnPlayerGetQuestRate(Player* player, float& result) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_QUEST_RATE);
             result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<float>(
                Hooks::PlayerEvent::ON_GET_QUEST_RATE,
                result,
                eventId,
                player,
                result
            );
        }

        bool OnPlayerPassedQuestKilledMonsterCredit(Player* player, Quest const* qinfo, uint32 entry, uint32 real_entry, ObjectGuid guid) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_PASSED_QUEST_MONSTER_CREDIT);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_PASSED_QUEST_MONSTER_CREDIT,
                true,
                eventId,
                player,
                qinfo,
                entry,
                real_entry,
                guid
            );
        }

        bool OnPlayerCheckItemInSlotAtLoadInventory(Player* player, Item* item, uint8 slot, uint8& err, uint16& dest) override
        {
            // hooks with refs and return bool... complex.
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CHECK_ITEM_SLOT_LOAD);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CHECK_ITEM_SLOT_LOAD,
                true,
                eventId,
                player,
                item,
                slot,
                err,
                dest
            );
        }

        bool OnPlayerNotAvoidSatisfy(Player* player, DungeonProgressionRequirements const* ar, uint32 target_map, bool report) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_NOT_AVOID_SATISFY);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_NOT_AVOID_SATISFY,
                true,
                eventId,
                player,
                ar,
                target_map,
                report
            );
        }

        bool OnPlayerNotVisibleGloballyFor(Player* player, Player const* u) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_NOT_VISIBLE_GLOBALLY);
            return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_NOT_VISIBLE_GLOBALLY,
                true,
                eventId,
                player,
                u
            );
        }

        void OnPlayerGetArenaPersonalRating(Player* player, uint8 slot, uint32& result) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_ARENA_PERSONAL_RATING);
             result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_GET_ARENA_PERSONAL_RATING,
                result,
                eventId,
                player,
                slot,
                result
            );
        }

        void OnPlayerGetArenaTeamId(Player* player, uint8 slot, uint32& result) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_ARENA_TEAM_ID);
             result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_GET_ARENA_TEAM_ID,
                result,
                eventId,
                player,
                slot,
                result
            );
        }

        void OnPlayerIsFFAPvP(Player* player, bool& result) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_IS_FFA_PVP);
             // result is ref bool
             // TriggerGlobalEventWithReturn doesn't strictly support bool ref return well if return type is bool (might be confused with success status) 
             // but let's assume it works or compiles to value assignment.
             result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_IS_FFA_PVP,
                result,
                eventId,
                player,
                result
            );
        }

        void OnPlayerIsPvP(Player* player, bool& result) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_IS_PVP);
            result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_IS_PVP,
                result,
                eventId,
                player,
                result
            );
        }

        void OnPlayerGetMaxSkillValueForLevel(Player* player, uint16& result) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_GET_MAX_SKILL_FOR_LEVEL);
             result = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint16>(
                Hooks::PlayerEvent::ON_GET_MAX_SKILL_FOR_LEVEL,
                result,
                eventId,
                player,
                result
            );
        }

        bool OnPlayerNotSetArenaTeamInfoField(Player* player, uint8 slot, ArenaTeamInfoType type, uint32 value) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_NOT_SET_ARENA_TEAM_INFO);
             return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_NOT_SET_ARENA_TEAM_INFO,
                true,
                eventId,
                player,
                slot,
                type,
                value
            );
        }

        bool OnPlayerCanSetTradeItem(Player* player, Item* tradedItem, uint8 tradeSlot) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_SET_TRADE_ITEM);
             return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_SET_TRADE_ITEM,
                true,
                eventId,
                player,
                tradedItem,
                tradeSlot
            );
        }

        void OnPlayerSetServerSideVisibility(Player* player, ServerSideVisibilityType& type, AccountTypes& sec) override
        {
            // multiple refs
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_SET_SERVER_SIDE_VISIBILITY, player, type, sec);
        }

        void OnPlayerSetServerSideVisibilityDetect(Player* player, ServerSideVisibilityType& type, AccountTypes& sec) override
        {
            // multiple refs
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_SET_SERVER_SIDE_VISIBILITY_DETECT, player, type, sec);
        }

        void OnPlayerBeforeChooseGraveyard(Player* player, TeamId teamId, bool nearCorpse, uint32& graveyardOverride) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_BEFORE_CHOOSE_GRAVEYARD);
             graveyardOverride = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_BEFORE_CHOOSE_GRAVEYARD,
                graveyardOverride,
                eventId,
                player,
                teamId,
                nearCorpse,
                graveyardOverride
            );
        }

        bool OnPlayerCanGiveLevel(Player* player, uint8 newLevel) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_GIVE_LEVEL);
             return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_GIVE_LEVEL,
                true,
                eventId,
                player,
                newLevel
            );
        }

        void OnPlayerSendListInventory(Player* player, ObjectGuid vendorGuid, uint32& vendorEntry) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_SEND_LIST_INVENTORY);
             vendorEntry = Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<uint32>(
                Hooks::PlayerEvent::ON_SEND_LIST_INVENTORY,
                vendorEntry,
                eventId,
                player,
                vendorGuid,
                vendorEntry
            );
        }

        bool OnPlayerCanEnterMap(Player* player, MapEntry const* entry, InstanceTemplate const* instance, MapDifficulty const* mapDiff, bool loginCheck) override
        {
            uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_CAN_ENTER_MAP);
             return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_CAN_ENTER_MAP,
                true,
                eventId,
                player,
                entry,
                instance,
                mapDiff,
                loginCheck
            );
        }

        void AnticheatSetCanFlybyServer(Player* player, bool apply) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ANTICHEAT_SET_CAN_FLY, player, apply);
        }

        void AnticheatSetUnderACKmount(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ANTICHEAT_SET_UNDER_ACK_MOUNT, player);
        }

        void AnticheatSetRootACKUpd(Player* player) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ANTICHEAT_SET_ROOT_ACK_UPD, player);
        }

        void AnticheatSetJumpingbyOpcode(Player* player, bool jump) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ANTICHEAT_SET_JUMPING, player, jump);
        }

        void AnticheatUpdateMovementInfo(Player* player, MovementInfo const& movementInfo) override
        {
            TriggerPlayerEvent(Hooks::PlayerEvent::ON_ANTICHEAT_UPDATE_MOVEMENT, player, &movementInfo);
        }

        bool AnticheatHandleDoubleJump(Player* player, Unit* mover) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_ANTICHEAT_HANDLE_DOUBLE_JUMP);
             return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_ANTICHEAT_HANDLE_DOUBLE_JUMP,
                true,
                eventId,
                player,
                mover
            );
        }

        bool AnticheatCheckMovementInfo(Player* player, MovementInfo const& movementInfo, Unit* mover, bool jump) override
        {
             uint32 eventId = static_cast<uint32>(Hooks::PlayerEvent::ON_ANTICHEAT_CHECK_MOVEMENT);
             return Core::EventManager::GetInstance().TriggerGlobalEventWithReturn<bool>(
                Hooks::PlayerEvent::ON_ANTICHEAT_CHECK_MOVEMENT,
                true,
                eventId,
                player,
                &movementInfo,
                mover,
                jump
            );
        }
    };

} // namespace ALE::Hooks

#endif // _ALE_PLAYER_HOOKS_H
