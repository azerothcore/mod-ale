/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Chat.h"
#include "ALEEventMgr.h"
#include "Log.h"
#include "LuaEngine.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

class ALE_AllCreatureScript : public AllCreatureScript
{
public:
    ALE_AllCreatureScript() : AllCreatureScript("ALE_AllCreatureScript") { }

    // Creature
    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (sALE->OnGossipHello(player, creature))
            return true;

        return false;
    }

    bool CanCreatureGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (sALE->OnGossipSelect(player, creature, sender, action))
            return true;

        return false;
    }

    bool CanCreatureGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
    {
        if (sALE->OnGossipSelectCode(player, creature, sender, action, code))
            return true;

        return false;
    }

    void OnCreatureAddWorld(Creature* creature) override
    {
        sALE->OnAddToWorld(creature);
        sALE->OnAllCreatureAddToWorld(creature);

        if (creature->IsGuardian() && creature->ToTempSummon() && creature->ToTempSummon()->GetSummonerGUID().IsPlayer())
            sALE->OnPetAddedToWorld(creature->ToTempSummon()->GetSummonerUnit()->ToPlayer(), creature);
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        sALE->OnRemoveFromWorld(creature);
        sALE->OnAllCreatureRemoveFromWorld(creature);
    }

    bool CanCreatureQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        sALE->OnPlayerQuestAccept(player, quest);
        sALE->OnQuestAccept(player, creature, quest);
        return false;
    }

    bool CanCreatureQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) override
    {
        if (sALE->OnQuestReward(player, creature, quest, opt))
        {
            ClearGossipMenuFor(player);
            return true;
        }

        return false;
    }

    CreatureAI* GetCreatureAI(Creature* creature) const override
    {
        if (CreatureAI* luaAI = sALE->GetAI(creature))
            return luaAI;

        return nullptr;
    }

    void OnCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature) override
    {
        sALE->OnAllCreatureSelectLevel(cinfo, creature);
    }

    void OnBeforeCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature, uint8& level) override
    {
        sALE->OnAllCreatureBeforeSelectLevel(cinfo, creature, level);
    }
};

class ALE_AllGameObjectScript : public AllGameObjectScript
{
public:
    ALE_AllGameObjectScript() : AllGameObjectScript("ALE_AllGameObjectScript") { }

    void OnGameObjectAddWorld(GameObject* go) override
    {
        sALE->OnAddToWorld(go);
    }

    void OnGameObjectRemoveWorld(GameObject* go) override
    {
        sALE->OnRemoveFromWorld(go);
    }

    void OnGameObjectUpdate(GameObject* go, uint32 diff) override
    {
        sALE->UpdateAI(go, diff);
    }

    bool CanGameObjectGossipHello(Player* player, GameObject* go) override
    {
        if (sALE->OnGossipHello(player, go))
            return true;

        if (sALE->OnGameObjectUse(player, go))
            return true;

        return false;
    }

    void OnGameObjectDamaged(GameObject* go, Player* player) override
    {
        sALE->OnDamaged(go, player);
    }

    void OnGameObjectDestroyed(GameObject* go, Player* player) override
    {
        sALE->OnDestroyed(go, player);
    }

    void OnGameObjectLootStateChanged(GameObject* go, uint32 state, Unit* /*unit*/) override
    {
        sALE->OnLootStateChanged(go, state);
    }

    void OnGameObjectStateChanged(GameObject* go, uint32 state) override
    {
        sALE->OnGameObjectStateChanged(go, state);
    }

    bool CanGameObjectQuestAccept(Player* player, GameObject* go, Quest const* quest) override
    {
        sALE->OnPlayerQuestAccept(player, quest);
        sALE->OnQuestAccept(player, go, quest);
        return false;
    }

    bool CanGameObjectGossipSelect(Player* player, GameObject* go, uint32 sender, uint32 action) override
    {
        if (sALE->OnGossipSelect(player, go, sender, action))
            return true;

        return false;
    }

    bool CanGameObjectGossipSelectCode(Player* player, GameObject* go, uint32 sender, uint32 action, const char* code) override
    {
        if (sALE->OnGossipSelectCode(player, go, sender, action, code))
            return true;

        return false;
    }

    bool CanGameObjectQuestReward(Player* player, GameObject* go, Quest const* quest, uint32 opt) override
    {
        if (sALE->OnQuestAccept(player, go, quest))
        {
            sALE->OnPlayerQuestAccept(player, quest);
            return false;
        }

        if (sALE->OnQuestReward(player, go, quest, opt))
            return false;

        return false;
    }

    GameObjectAI* GetGameObjectAI(GameObject* go) const override
    {
        sALE->OnSpawn(go);
        return nullptr;
    }
};

class ALE_AllItemScript : public AllItemScript
{
public:
    ALE_AllItemScript() : AllItemScript("ALE_AllItemScript") { }

    bool CanItemQuestAccept(Player* player, Item* item, Quest const* quest) override
    {
        if (sALE->OnQuestAccept(player, item, quest))
        {
            sALE->OnPlayerQuestAccept(player, quest);
            return false;
        }

        return true;
    }

    bool CanItemUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (!sALE->OnUse(player, item, targets))
            return true;

        return false;
    }

    bool CanItemExpire(Player* player, ItemTemplate const* proto) override
    {
        if (sALE->OnExpire(player, proto))
            return false;

        return true;
    }

    bool CanItemRemove(Player* player, Item* item) override
    {
        if (sALE->OnRemove(player, item))
            return false;

        return true;
    }

    void OnItemGossipSelect(Player* player, Item* item, uint32 sender, uint32 action) override
    {
        sALE->HandleGossipSelectOption(player, item, sender, action, "");
    }

    void OnItemGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code) override
    {
        sALE->HandleGossipSelectOption(player, item, sender, action, code);
    }
};

class ALE_AllMapScript : public AllMapScript
{
public:
    ALE_AllMapScript() : AllMapScript("ALE_AllMapScript", {
        ALLMAPHOOK_ON_BEFORE_CREATE_INSTANCE_SCRIPT,
        ALLMAPHOOK_ON_DESTROY_INSTANCE,
        ALLMAPHOOK_ON_CREATE_MAP,
        ALLMAPHOOK_ON_DESTROY_MAP,
        ALLMAPHOOK_ON_PLAYER_ENTER_ALL,
        ALLMAPHOOK_ON_PLAYER_LEAVE_ALL,
        ALLMAPHOOK_ON_MAP_UPDATE
    }) { }

    void OnBeforeCreateInstanceScript(InstanceMap* instanceMap, InstanceScript** instanceData, bool /*load*/, std::string /*data*/, uint32 /*completedEncounterMask*/) override
    {
        if (instanceData)
            *instanceData = sALE->GetInstanceData(instanceMap);
    }

    void OnDestroyInstance(MapInstanced* /*mapInstanced*/, Map* map) override
    {
        sALE->FreeInstanceId(map->GetInstanceId());
    }

    void OnCreateMap(Map* map) override
    {
        sALE->OnCreate(map);
    }

    void OnDestroyMap(Map* map) override
    {
        sALE->OnDestroy(map);
    }

    void OnPlayerEnterAll(Map* map, Player* player) override
    {
        sALE->OnPlayerEnter(map, player);
    }

    void OnPlayerLeaveAll(Map* map, Player* player) override
    {
        sALE->OnPlayerLeave(map, player);
    }

    void OnMapUpdate(Map* map, uint32 diff) override
    {
        sALE->OnUpdate(map, diff);
    }
};

class ALE_AuctionHouseScript : public AuctionHouseScript
{
public:
    ALE_AuctionHouseScript() : AuctionHouseScript("ALE_AuctionHouseScript", {
        AUCTIONHOUSEHOOK_ON_AUCTION_ADD,
        AUCTIONHOUSEHOOK_ON_AUCTION_REMOVE,
        AUCTIONHOUSEHOOK_ON_AUCTION_SUCCESSFUL,
        AUCTIONHOUSEHOOK_ON_AUCTION_EXPIRE
    }) { }

    void OnAuctionAdd(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sALE->OnAdd(ah, entry);
    }

    void OnAuctionRemove(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sALE->OnRemove(ah, entry);
    }

    void OnAuctionSuccessful(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sALE->OnSuccessful(ah, entry);
    }

    void OnAuctionExpire(AuctionHouseObject* ah, AuctionEntry* entry) override
    {
        sALE->OnExpire(ah, entry);
    }
};

class ALE_BGScript : public BGScript
{
public:
    ALE_BGScript() : BGScript("ALE_BGScript", {
        ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_START,
        ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_END,
        ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_DESTROY,
        ALLBATTLEGROUNDHOOK_ON_BATTLEGROUND_CREATE
    }) { }

    void OnBattlegroundStart(Battleground* bg) override
    {
        sALE->OnBGStart(bg, bg->GetBgTypeID(), bg->GetInstanceID());
    }

    void OnBattlegroundEnd(Battleground* bg, TeamId winnerTeam) override
    {
        sALE->OnBGEnd(bg, bg->GetBgTypeID(), bg->GetInstanceID(), winnerTeam);
    }

    void OnBattlegroundDestroy(Battleground* bg) override
    {
        sALE->OnBGDestroy(bg, bg->GetBgTypeID(), bg->GetInstanceID());
    }

    void OnBattlegroundCreate(Battleground* bg) override
    {
        sALE->OnBGCreate(bg, bg->GetBgTypeID(), bg->GetInstanceID());
    }
};

class ALE_CommandSC : public CommandSC
{
public:
    ALE_CommandSC() : CommandSC("ALE_CommandSC", {
        ALLCOMMANDHOOK_ON_TRY_EXECUTE_COMMAND
    }) { }

    bool OnTryExecuteCommand(ChatHandler& handler, std::string_view cmdStr) override
    {
        if (!sALE->OnCommand(handler, std::string(cmdStr).c_str()))
        {
            return false;
        }

        return true;
    }
};

class ALE_ALEScript : public ALEScript
{
public:
    ALE_ALEScript() : ALEScript("ALE_ALEScript") { }

    // Weather
    void OnWeatherChange(Weather* weather, WeatherState state, float grade) override
    {
        sALE->OnChange(weather, weather->GetZone(), state, grade);
    }

    // AreaTriger
    bool CanAreaTrigger(Player* player, AreaTrigger const* trigger) override
    {
        if (sALE->OnAreaTrigger(player, trigger))
            return true;

        return false;
    }
};

class ALE_GameEventScript : public GameEventScript
{
public:
    ALE_GameEventScript() : GameEventScript("ALE_GameEventScript", {
        GAMEEVENTHOOK_ON_START,
        GAMEEVENTHOOK_ON_STOP
    }) { }

    void OnStart(uint16 eventID) override
    {
        sALE->OnGameEventStart(eventID);
    }

    void OnStop(uint16 eventID) override
    {
        sALE->OnGameEventStop(eventID);
    }
};

class ALE_GroupScript : public GroupScript
{
public:
    ALE_GroupScript() : GroupScript("ALE_GroupScript", {
        GROUPHOOK_ON_ADD_MEMBER,
        GROUPHOOK_ON_INVITE_MEMBER,
        GROUPHOOK_ON_REMOVE_MEMBER,
        GROUPHOOK_ON_CHANGE_LEADER,
        GROUPHOOK_ON_DISBAND,
        GROUPHOOK_ON_CREATE
    }) { }

    void OnAddMember(Group* group, ObjectGuid guid) override
    {
        sALE->OnAddMember(group, guid);
    }

    void OnInviteMember(Group* group, ObjectGuid guid) override
    {
        sALE->OnInviteMember(group, guid);
    }

    void OnRemoveMember(Group* group, ObjectGuid guid, RemoveMethod method, ObjectGuid /* kicker */, const char* /* reason */) override
    {
        sALE->OnRemoveMember(group, guid, method);
    }

    void OnChangeLeader(Group* group, ObjectGuid newLeaderGuid, ObjectGuid oldLeaderGuid) override
    {
        sALE->OnChangeLeader(group, newLeaderGuid, oldLeaderGuid);
    }

    void OnDisband(Group* group) override
    {
        sALE->OnDisband(group);
    }

    void OnCreate(Group* group, Player* leader) override
    {
        sALE->OnCreate(group, leader->GetGUID(), group->GetGroupType());
    }
};

class ALE_GuildScript : public GuildScript
{
public:
    ALE_GuildScript() : GuildScript("ALE_GuildScript", {
        GUILDHOOK_ON_ADD_MEMBER,
        GUILDHOOK_ON_REMOVE_MEMBER,
        GUILDHOOK_ON_MOTD_CHANGED,
        GUILDHOOK_ON_INFO_CHANGED,
        GUILDHOOK_ON_CREATE,
        GUILDHOOK_ON_DISBAND,
        GUILDHOOK_ON_MEMBER_WITDRAW_MONEY,
        GUILDHOOK_ON_MEMBER_DEPOSIT_MONEY,
        GUILDHOOK_ON_ITEM_MOVE,
        GUILDHOOK_ON_EVENT,
        GUILDHOOK_ON_BANK_EVENT
    }) { }

    void OnAddMember(Guild* guild, Player* player, uint8& plRank) override
    {
        sALE->OnAddMember(guild, player, plRank);
    }

    void OnRemoveMember(Guild* guild, Player* player, bool isDisbanding, bool /*isKicked*/) override
    {
        sALE->OnRemoveMember(guild, player, isDisbanding);
    }

    void OnMOTDChanged(Guild* guild, const std::string& newMotd) override
    {
        sALE->OnMOTDChanged(guild, newMotd);
    }

    void OnInfoChanged(Guild* guild, const std::string& newInfo) override
    {
        sALE->OnInfoChanged(guild, newInfo);
    }

    void OnCreate(Guild* guild, Player* leader, const std::string& name) override
    {
        sALE->OnCreate(guild, leader, name);
    }

    void OnDisband(Guild* guild) override
    {
        sALE->OnDisband(guild);
    }

    void OnMemberWitdrawMoney(Guild* guild, Player* player, uint32& amount, bool isRepair) override
    {
        sALE->OnMemberWitdrawMoney(guild, player, amount, isRepair);
    }

    void OnMemberDepositMoney(Guild* guild, Player* player, uint32& amount) override
    {
        sALE->OnMemberDepositMoney(guild, player, amount);
    }

    void OnItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId,
        bool isDestBank, uint8 destContainer, uint8 destSlotId) override
    {
        sALE->OnItemMove(guild, player, pItem, isSrcBank, srcContainer, srcSlotId, isDestBank, destContainer, destSlotId);
    }

    void OnEvent(Guild* guild, uint8 eventType, ObjectGuid::LowType playerGuid1, ObjectGuid::LowType playerGuid2, uint8 newRank) override
    {
        sALE->OnEvent(guild, eventType, playerGuid1, playerGuid2, newRank);
    }

    void OnBankEvent(Guild* guild, uint8 eventType, uint8 tabId, ObjectGuid::LowType playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId) override
    {
        sALE->OnBankEvent(guild, eventType, tabId, playerGuid, itemOrMoney, itemStackCount, destTabId);
    }
};

class ALE_LootScript : public LootScript
{
public:
    ALE_LootScript() : LootScript("ALE_LootScript", {
        LOOTHOOK_ON_LOOT_MONEY
    }) { }

    void OnLootMoney(Player* player, uint32 gold) override
    {
        sALE->OnLootMoney(player, gold);
    }
};

class ALE_MiscScript : public MiscScript
{
public:
    ALE_MiscScript() : MiscScript("ALE_MiscScript", {
        MISCHOOK_GET_DIALOG_STATUS
    }) { }

    void GetDialogStatus(Player* player, Object* questgiver) override
    {
        if (questgiver->GetTypeId() == TYPEID_GAMEOBJECT)
            sALE->GetDialogStatus(player, questgiver->ToGameObject());
        else if (questgiver->GetTypeId() == TYPEID_UNIT)
            sALE->GetDialogStatus(player, questgiver->ToCreature());
    }
};

class ALE_PetScript : public PetScript
{
public:
    ALE_PetScript() : PetScript("ALE_PetScript", {
        PETHOOK_ON_PET_ADD_TO_WORLD
    }) { }

    void OnPetAddToWorld(Pet* pet) override
    {
        sALE->OnPetAddedToWorld(pet->GetOwner(), pet);
    }
};

class ALE_PlayerScript : public PlayerScript
{
public:
    ALE_PlayerScript() : PlayerScript("ALE_PlayerScript", {
        PLAYERHOOK_ON_PLAYER_RESURRECT,
        PLAYERHOOK_CAN_PLAYER_USE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GUILD_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_CHANNEL_CHAT,
        PLAYERHOOK_ON_LOOT_ITEM,
        PLAYERHOOK_ON_PLAYER_LEARN_TALENTS,
        PLAYERHOOK_CAN_USE_ITEM,
        PLAYERHOOK_ON_EQUIP,
        PLAYERHOOK_ON_PLAYER_ENTER_COMBAT,
        PLAYERHOOK_ON_PLAYER_LEAVE_COMBAT,
        PLAYERHOOK_CAN_REPOP_AT_GRAVEYARD,
        PLAYERHOOK_ON_QUEST_ABANDON,
        PLAYERHOOK_ON_MAP_CHANGED,
        PLAYERHOOK_ON_GOSSIP_SELECT,
        PLAYERHOOK_ON_GOSSIP_SELECT_CODE,
        PLAYERHOOK_ON_PVP_KILL,
        PLAYERHOOK_ON_CREATURE_KILL,
        PLAYERHOOK_ON_PLAYER_KILLED_BY_CREATURE,
        PLAYERHOOK_ON_LEVEL_CHANGED,
        PLAYERHOOK_ON_FREE_TALENT_POINTS_CHANGED,
        PLAYERHOOK_ON_TALENTS_RESET,
        PLAYERHOOK_ON_MONEY_CHANGED,
        PLAYERHOOK_ON_GIVE_EXP,
        PLAYERHOOK_ON_REPUTATION_CHANGE,
        PLAYERHOOK_ON_DUEL_REQUEST,
        PLAYERHOOK_ON_DUEL_START,
        PLAYERHOOK_ON_DUEL_END,
        PLAYERHOOK_ON_EMOTE,
        PLAYERHOOK_ON_TEXT_EMOTE,
        PLAYERHOOK_ON_SPELL_CAST,
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_CREATE,
        PLAYERHOOK_ON_SAVE,
        PLAYERHOOK_ON_DELETE,
        PLAYERHOOK_ON_BIND_TO_INSTANCE,
        PLAYERHOOK_ON_UPDATE_AREA,
        PLAYERHOOK_ON_UPDATE_ZONE,
        PLAYERHOOK_ON_FIRST_LOGIN,
        PLAYERHOOK_ON_LEARN_SPELL,
        PLAYERHOOK_ON_ACHI_COMPLETE,
        PLAYERHOOK_ON_FFA_PVP_STATE_UPDATE,
        PLAYERHOOK_CAN_INIT_TRADE,
        PLAYERHOOK_CAN_SEND_MAIL,
        PLAYERHOOK_CAN_JOIN_LFG,
        PLAYERHOOK_ON_QUEST_REWARD_ITEM,
        PLAYERHOOK_ON_GROUP_ROLL_REWARD_ITEM,
        PLAYERHOOK_ON_CREATE_ITEM,
        PLAYERHOOK_ON_STORE_NEW_ITEM,
        PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST,
        PLAYERHOOK_CAN_GROUP_INVITE,
        PLAYERHOOK_ON_BATTLEGROUND_DESERTION,
        PLAYERHOOK_ON_CREATURE_KILLED_BY_PET,
        PLAYERHOOK_ON_CAN_UPDATE_SKILL,
        PLAYERHOOK_ON_BEFORE_UPDATE_SKILL,
        PLAYERHOOK_ON_UPDATE_SKILL,
        PLAYERHOOK_CAN_RESURRECT
    }) { }

    void OnPlayerResurrect(Player* player, float /*restore_percent*/, bool /*applySickness*/) override
    {
        sALE->OnResurrect(player);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
    {
        if (type != CHAT_MSG_SAY && type != CHAT_MSG_YELL && type != CHAT_MSG_EMOTE)
            return true;

        if (!sALE->OnChat(player, type, lang, msg))
            return false;

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* target) override
    {
        if (!sALE->OnChat(player, type, lang, msg, target))
            return false;

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* group) override
    {
        if (!sALE->OnChat(player, type, lang, msg, group))
            return false;

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* guild) override
    {
        if (!sALE->OnChat(player, type, lang, msg, guild))
            return false;

        return true;
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Channel* channel) override
    {
        if (!sALE->OnChat(player, type, lang, msg, channel))
            return false;

        return true;
    }

    void OnPlayerLootItem(Player* player, Item* item, uint32 count, ObjectGuid lootguid) override
    {
        sALE->OnLootItem(player, item, count, lootguid);
    }

    void OnPlayerLearnTalents(Player* player, uint32 talentId, uint32 talentRank, uint32 spellid) override
    {
        sALE->OnLearnTalents(player, talentId, talentRank, spellid);
    }

    bool OnPlayerCanUseItem(Player* player, ItemTemplate const* proto, InventoryResult& result) override
    {
        result = sALE->OnCanUseItem(player, proto->ItemId);
        return result != EQUIP_ERR_OK ? false : true;
    }

    void OnPlayerEquip(Player* player, Item* it, uint8 bag, uint8 slot, bool /*update*/) override
    {
        sALE->OnEquip(player, it, bag, slot);
    }

    void OnPlayerEnterCombat(Player* player, Unit* enemy) override
    {
        sALE->OnPlayerEnterCombat(player, enemy);
    }

    void OnPlayerLeaveCombat(Player* player) override
    {
        sALE->OnPlayerLeaveCombat(player);
    }

    bool OnPlayerCanRepopAtGraveyard(Player* player) override
    {
        sALE->OnRepop(player);
        return true;
    }

    void OnPlayerQuestAbandon(Player* player, uint32 questId) override
    {
        sALE->OnQuestAbandon(player, questId);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        sALE->OnMapChanged(player);
    }

    void OnPlayerGossipSelect(Player* player, uint32 menu_id, uint32 sender, uint32 action) override
    {
        sALE->HandleGossipSelectOption(player, menu_id, sender, action, "");
    }

    void OnPlayerGossipSelectCode(Player* player, uint32 menu_id, uint32 sender, uint32 action, const char* code) override
    {
        sALE->HandleGossipSelectOption(player, menu_id, sender, action, code);
    }

    void OnPlayerPVPKill(Player* killer, Player* killed) override
    {
        sALE->OnPVPKill(killer, killed);
    }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        sALE->OnCreatureKill(killer, killed);
    }

    void OnPlayerKilledByCreature(Creature* killer, Player* killed) override
    {
        sALE->OnPlayerKilledByCreature(killer, killed);
    }

    void OnPlayerLevelChanged(Player* player, uint8 oldLevel) override
    {
        sALE->OnLevelChanged(player, oldLevel);
    }

    void OnPlayerFreeTalentPointsChanged(Player* player, uint32 points) override
    {
        sALE->OnFreeTalentPointsChanged(player, points);
    }

    void OnPlayerTalentsReset(Player* player, bool noCost) override
    {
        sALE->OnTalentsReset(player, noCost);
    }

    void OnPlayerMoneyChanged(Player* player, int32& amount) override
    {
        sALE->OnMoneyChanged(player, amount);
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* victim, uint8 xpSource) override
    {
        sALE->OnGiveXP(player, amount, victim, xpSource);
    }

    bool OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental) override
    {
        return sALE->OnReputationChange(player, factionID, standing, incremental);
    }

    void OnPlayerDuelRequest(Player* target, Player* challenger) override
    {
        sALE->OnDuelRequest(target, challenger);
    }

    void OnPlayerDuelStart(Player* player1, Player* player2) override
    {
        sALE->OnDuelStart(player1, player2);
    }

    void OnPlayerDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
    {
        sALE->OnDuelEnd(winner, loser, type);
    }

    void OnPlayerEmote(Player* player, uint32 emote) override
    {
        sALE->OnEmote(player, emote);
    }

    void OnPlayerTextEmote(Player* player, uint32 textEmote, uint32 emoteNum, ObjectGuid guid) override
    {
        sALE->OnTextEmote(player, textEmote, emoteNum, guid);
    }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool skipCheck) override
    {
        sALE->OnPlayerSpellCast(player, spell, skipCheck);
    }

    void OnPlayerLogin(Player* player) override
    {
        sALE->OnLogin(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        sALE->OnLogout(player);
    }

    void OnPlayerCreate(Player* player) override
    {
        sALE->OnCreate(player);
    }

    void OnPlayerSave(Player* player) override
    {
        sALE->OnSave(player);
    }

    void OnPlayerDelete(ObjectGuid guid, uint32 /*accountId*/) override
    {
        sALE->OnDelete(guid.GetCounter());
    }

    void OnPlayerBindToInstance(Player* player, Difficulty difficulty, uint32 mapid, bool permanent) override
    {
        sALE->OnBindToInstance(player, difficulty, mapid, permanent);
    }

    void OnPlayerUpdateArea(Player* player, uint32 oldArea, uint32 newArea) override
    {
        sALE->OnUpdateArea(player, oldArea, newArea);
    }

    void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 newArea) override
    {
        sALE->OnUpdateZone(player, newZone, newArea);
    }

    void OnPlayerFirstLogin(Player* player) override
    {
        sALE->OnFirstLogin(player);
    }

    void OnPlayerLearnSpell(Player* player, uint32 spellId) override
    {
        sALE->OnLearnSpell(player, spellId);
    }

    void OnPlayerAchievementComplete(Player* player, AchievementEntry const* achievement) override
    {
        sALE->OnAchiComplete(player, achievement);
    }

    void OnPlayerFfaPvpStateUpdate(Player* player, bool IsFlaggedForFfaPvp) override
    {
        sALE->OnFfaPvpStateUpdate(player, IsFlaggedForFfaPvp);
    }

    bool OnPlayerCanInitTrade(Player* player, Player* target) override
    {
        return sALE->OnCanInitTrade(player, target);
    }

    bool OnPlayerCanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint32 money, uint32 cod, Item* item) override
    {
        return sALE->OnCanSendMail(player, receiverGuid, mailbox, subject, body, money, cod, item);
    }

    bool OnPlayerCanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, const std::string& comment) override
    {
        return sALE->OnCanJoinLfg(player, roles, dungeons, comment);
    }

    void OnPlayerQuestRewardItem(Player* player, Item* item, uint32 count) override
    {
        sALE->OnQuestRewardItem(player, item, count);
    }

    void OnPlayerGroupRollRewardItem(Player* player, Item* item, uint32 count, RollVote voteType, Roll* roll) override
    {
        sALE->OnGroupRollRewardItem(player, item, count, voteType, roll);
    }

    void OnPlayerCreateItem(Player* player, Item* item, uint32 count) override
    {
        sALE->OnCreateItem(player, item, count);
    }

    void OnPlayerStoreNewItem(Player* player, Item* item, uint32 count) override
    {
        sALE->OnStoreNewItem(player, item, count);
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        sALE->OnPlayerCompleteQuest(player, quest);
    }

    bool OnPlayerCanGroupInvite(Player* player, std::string& memberName) override
    {
        return sALE->OnCanGroupInvite(player, memberName);
    }

    void OnPlayerBattlegroundDesertion(Player* player, const BattlegroundDesertionType type) override
    {
        sALE->OnBattlegroundDesertion(player, type);
    }

    void OnPlayerCreatureKilledByPet(Player* player, Creature* killed) override
    {
        sALE->OnCreatureKilledByPet(player, killed);
    }

    bool OnPlayerCanUpdateSkill(Player* player, uint32 skill_id) override
    {
        return sALE->OnPlayerCanUpdateSkill(player, skill_id);
    }

    void OnPlayerBeforeUpdateSkill(Player* player, uint32 skill_id, uint32& value, uint32 max, uint32 step) override
    {
        sALE->OnPlayerBeforeUpdateSkill(player, skill_id, value, max, step);
    }

    void OnPlayerUpdateSkill(Player* player, uint32 skill_id, uint32 value, uint32 max, uint32 step, uint32 new_value) override
    {
        sALE->OnPlayerUpdateSkill(player, skill_id, value, max, step, new_value);
    }
    
    bool OnPlayerCanResurrect(Player* player) override
    {
        return sALE->CanPlayerResurrect(player);
    }
};

class ALE_ServerScript : public ServerScript
{
public:
    ALE_ServerScript() : ServerScript("ALE_ServerScript", {
        SERVERHOOK_CAN_PACKET_SEND,
        SERVERHOOK_CAN_PACKET_RECEIVE
    }) { }

    bool CanPacketSend(WorldSession* session, WorldPacket& packet) override
    {
        if (!sALE->OnPacketSend(session, packet))
            return false;

        return true;
    }

    bool CanPacketReceive(WorldSession* session, WorldPacket& packet) override
    {
        if (!sALE->OnPacketReceive(session, packet))
            return false;

        return true;
    }
};

class ALE_SpellSC : public SpellSC
{
public:
    ALE_SpellSC() : SpellSC("ALE_SpellSC", {
        ALLSPELLHOOK_ON_DUMMY_EFFECT_GAMEOBJECT,
        ALLSPELLHOOK_ON_DUMMY_EFFECT_CREATURE,
        ALLSPELLHOOK_ON_DUMMY_EFFECT_ITEM,
        ALLSPELLHOOK_ON_CAST_CANCEL,
        ALLSPELLHOOK_ON_CAST,
        ALLSPELLHOOK_ON_PREPARE
    }) { }

    void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, GameObject* gameObjTarget) override
    {
        sALE->OnDummyEffect(caster, spellID, effIndex, gameObjTarget);
    }

    void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Creature* creatureTarget) override
    {
        sALE->OnDummyEffect(caster, spellID, effIndex, creatureTarget);
    }

    void OnDummyEffect(WorldObject* caster, uint32 spellID, SpellEffIndex effIndex, Item* itemTarget) override
    {
        sALE->OnDummyEffect(caster, spellID, effIndex, itemTarget);
    }

    void OnSpellCastCancel(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool bySelf) override
    {
        sALE->OnSpellCastCancel(caster, spell, spellInfo, bySelf);
    }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool skipCheck) override
    {
        sALE->OnSpellCast(caster, spell, spellInfo, skipCheck);
    }

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        sALE->OnSpellPrepare(caster, spell, spellInfo);
    }
};

class ALE_VehicleScript : public VehicleScript
{
public:
    ALE_VehicleScript() : VehicleScript("ALE_VehicleScript") { }

    void OnInstall(Vehicle* veh) override
    {
        sALE->OnInstall(veh);
    }

    void OnUninstall(Vehicle* veh) override
    {
        sALE->OnUninstall(veh);
    }

    void OnInstallAccessory(Vehicle* veh, Creature* accessory) override
    {
        sALE->OnInstallAccessory(veh, accessory);
    }

    void OnAddPassenger(Vehicle* veh, Unit* passenger, int8 seatId) override
    {
        sALE->OnAddPassenger(veh, passenger, seatId);
    }

    void OnRemovePassenger(Vehicle* veh, Unit* passenger) override
    {
        sALE->OnRemovePassenger(veh, passenger);
    }
};

class ALE_WorldObjectScript : public WorldObjectScript
{
public:
    ALE_WorldObjectScript() : WorldObjectScript("ALE_WorldObjectScript", {
        WORLDOBJECTHOOK_ON_WORLD_OBJECT_DESTROY,
        WORLDOBJECTHOOK_ON_WORLD_OBJECT_CREATE,
        WORLDOBJECTHOOK_ON_WORLD_OBJECT_SET_MAP,
        WORLDOBJECTHOOK_ON_WORLD_OBJECT_UPDATE
    }) { }

    void OnWorldObjectDestroy(WorldObject* object) override
    {
        delete object->ALEEvents;
        object->ALEEvents = nullptr;
    }

    void OnWorldObjectCreate(WorldObject* object) override
    {
        object->ALEEvents = nullptr;
    }

    void OnWorldObjectSetMap(WorldObject* object, Map* /*map*/) override
    {
        if (!object->ALEEvents)
            object->ALEEvents = new ALEEventProcessor(&ALE::GALE, object);
    }

    void OnWorldObjectUpdate(WorldObject* object, uint32 diff) override
    {
        object->ALEEvents->Update(diff);
    }
};

class ALE_WorldScript : public WorldScript
{
public:
    ALE_WorldScript() : WorldScript("ALE_WorldScript", {
        WORLDHOOK_ON_OPEN_STATE_CHANGE,
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD,
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        WORLDHOOK_ON_SHUTDOWN_INITIATE,
        WORLDHOOK_ON_SHUTDOWN_CANCEL,
        WORLDHOOK_ON_UPDATE,
        WORLDHOOK_ON_STARTUP,
        WORLDHOOK_ON_SHUTDOWN,
        WORLDHOOK_ON_AFTER_UNLOAD_ALL_MAPS,
        WORLDHOOK_ON_BEFORE_WORLD_INITIALIZED
    }) { }

    void OnOpenStateChange(bool open) override
    {
        sALE->OnOpenStateChange(open);
    }

    void OnBeforeConfigLoad(bool reload) override
    {
        ALEConfig::GetInstance().Initialize(reload);
        if (!reload)
        {
            ///- Initialize Lua Engine
            LOG_INFO("ALE", "Initialize ALE Lua Engine...");
            ALE::Initialize();
        }

        sALE->OnConfigLoad(reload, true);
    }

    void OnAfterConfigLoad(bool reload) override
    {
        sALE->OnConfigLoad(reload, false);
    }

    void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask) override
    {
        sALE->OnShutdownInitiate(code, mask);
    }

    void OnShutdownCancel() override
    {
        sALE->OnShutdownCancel();
    }

    void OnUpdate(uint32 diff) override
    {
        sALE->OnWorldUpdate(diff);
    }

    void OnStartup() override
    {
        sALE->OnStartup();
    }

    void OnShutdown() override
    {
        sALE->OnShutdown();
    }

    void OnAfterUnloadAllMaps() override
    {
        ALE::Uninitialize();
    }

    void OnBeforeWorldInitialized() override
    {
        ///- Run ALE scripts.
        // in multithread foreach: run scripts
        sALE->RunScripts();
        sALE->OnConfigLoad(false, false); // Must be done after ALE is initialized and scripts have run.
    }
};

class ALE_TicketScript : public TicketScript
{
public:
    ALE_TicketScript() : TicketScript("ALE_TicketScript", {
        TICKETHOOK_ON_TICKET_CREATE,
        TICKETHOOK_ON_TICKET_UPDATE_LAST_CHANGE,
        TICKETHOOK_ON_TICKET_CLOSE,
        TICKETHOOK_ON_TICKET_RESOLVE
    }) { }

    void OnTicketCreate(GmTicket* ticket) override
    {
        sALE->OnTicketCreate(ticket);
    }

    void OnTicketUpdateLastChange(GmTicket* ticket) override
    {
        sALE->OnTicketUpdateLastChange(ticket);
    }

    void OnTicketClose(GmTicket* ticket) override
    {
        sALE->OnTicketClose(ticket);
    }

    void OnTicketResolve(GmTicket* ticket) override
    {
        sALE->OnTicketResolve(ticket);
    }
};

class ALE_UnitScript : public UnitScript
{
public:
    ALE_UnitScript() : UnitScript("ALE_UnitScript") { }

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        if (!unit || !aura) return;

        if (unit->IsPlayer())
            sALE->OnPlayerAuraApply(unit->ToPlayer(), aura);

        if (unit->IsCreature())
            sALE->OnCreatureAuraApply(unit->ToCreature(), aura);
    }

    void OnHeal(Unit* healer, Unit* receiver, uint32& gain) override
    {
        if (!receiver || !healer) return;

        if (healer->IsPlayer())
            sALE->OnPlayerHeal(healer->ToPlayer(), receiver, gain);

        if (healer->IsCreature())
            sALE->OnCreatureHeal(healer->ToCreature(), receiver, gain);
    }

    void OnDamage(Unit* attacker, Unit* receiver, uint32& damage) override
    {
        if (!attacker || !receiver) return;

        if (attacker->IsPlayer())
            sALE->OnPlayerDamage(attacker->ToPlayer(), receiver, damage);

        if (attacker->IsCreature())
            sALE->OnCreatureDamage(attacker->ToCreature(), receiver, damage);
    }
};

// Group all custom scripts
void AddSC_ALE()
{
    new ALE_AllCreatureScript();
    new ALE_AllGameObjectScript();
    new ALE_AllItemScript();
    new ALE_AllMapScript();
    new ALE_AuctionHouseScript();
    new ALE_BGScript();
    new ALE_CommandSC();
    new ALE_ALEScript();
    new ALE_GameEventScript();
    new ALE_GroupScript();
    new ALE_GuildScript();
    new ALE_LootScript();
    new ALE_MiscScript();
    new ALE_PetScript();
    new ALE_PlayerScript();
    new ALE_ServerScript();
    new ALE_SpellSC();
    new ALE_TicketScript();
    new ALE_VehicleScript();
    new ALE_WorldObjectScript();
    new ALE_WorldScript();
    new ALE_UnitScript();
}
