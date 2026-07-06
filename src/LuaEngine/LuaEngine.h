/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _LUA_ENGINE_H
#define _LUA_ENGINE_H

#include "Common.h"
#include "SharedDefines.h"
#include "DBCEnums.h"

#include "Group.h"
#include "Item.h"
#include "Chat.h"
#include "Player.h"
#include "Weather.h"
#include "World.h"
#include "Hooks.h"
#include "ALEBind.h"
#include "ALEConfig.h"
#include "ALEFileWatcher.h"
#include "ALEHandles.h"
#include "ALEUtility.h"
#include "BindingMap.h"
#include "EventEmitter.h"
#include "HttpManager.h"
#include "LFG.h"
#include "LootMgr.h"
#include "TicketMgr.h"

#include <sol/sol.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

struct ItemTemplate;
typedef BattlegroundTypeId BattleGroundTypeId;
typedef AreaTrigger AreaTriggerEntry;
class AuctionHouseObject;
struct AuctionEntry;
class Battleground;
typedef Battleground BattleGround;
class Channel;
class Corpse;
class Creature;
class CreatureAI;
class GameObject;
class Guild;
class Group;
class InstanceScript;
typedef InstanceScript InstanceData;
class ALEInstanceAI;
class Item;
class Pet;
class Player;
class Quest;
class Spell;
class SpellCastTargets;
class TempSummon;
class Unit;
class Weather;
class WorldPacket;
class Vehicle;

class EventMgr;

struct LuaScript
{
    std::string fileext;
    std::string filename;
    std::string filepath;
    std::string modulepath;
};

#define LOCK_ALE ALE::Guard __guard(ALE::GetLock())
#define ALE_GAME_API AC_GAME_API

class ALE_GAME_API ALE
{
public:
    typedef std::list<LuaScript> ScriptList;

    typedef std::recursive_mutex LockType;
    typedef std::lock_guard<LockType> Guard;

    const std::string& GetRequirePath() const { return lua_requirepath; }
    const std::string& GetRequireCPath() const { return lua_requirecpath; }

private:
    static bool reload;
    static bool initialized;
    static LockType lock;
    static std::unique_ptr<ALEFileWatcher> fileWatcher;

    // Lua script locations
    static ScriptList lua_scripts;
    static ScriptList lua_extensions;

    // Lua script folder path
    static std::string lua_folderpath;
    // lua path variables for the require() function
    static std::string lua_requirepath;
    static std::string lua_requirecpath;

    // Depth of nested event dispatches. When it drops back to 0 the engine
    // returns to C++ and the handle pointer caches are retired
    // (see ALEHandleEpoch in ALEHandles.h).
    uint32 event_level = 0;

    // Whether OpenLua() ran: a sol::state member always exists, but scripts
    // and bindings are only loaded into it while the engine is enabled.
    bool stateOpened = false;

    // Per-map instance data tables, kept alive by sol references.
    std::unordered_map<uint32, sol::table> instanceDataRefs;
    // Per-continent data tables (map id -> table).
    std::unordered_map<uint32, sol::table> continentDataRefs;

    ALE();
    ~ALE();

    // Prevent copy
    ALE(ALE const&) = delete;
    ALE& operator=(const ALE&) = delete;

    void OpenLua();
    void CloseLua();
    void DestroyBindStores();
    void CreateBindStores();

    // Use ReloadALE() to make ALE reload
    // This is called on world update to reload ALE
    static void _ReloadALE();
    static void LoadScriptPaths();
    static void GetScripts(std::string path);
    static void AddScriptPath(std::string filename, const std::string& fullpath);

    // Marks the end of one nested dispatch level; retires handle caches when
    // the outermost handler returns.
    void EnterDispatch() { ++event_level; }
    void LeaveDispatch()
    {
        if (--event_level == 0)
            ALEHandleEpoch::Bump();
    }

    // Reports a failed handler call to the log and the OnError emitter.
    void Report(sol::error const& error);

public:
    static ALE* GALE;

    // The Lua state. Bindings and hooks go through sol, never the raw C API.
    sol::state lua;

    EventMgr* eventMgr;
    HttpManager httpManager;
    QueryCallbackProcessor queryProcessor;
    EventEmitter<void(std::string)> OnError;

    std::unique_ptr<BindingMap<EventKey<Hooks::ServerEvents>>>      ServerEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::PlayerEvents>>>      PlayerEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::GuildEvents>>>       GuildEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::GroupEvents>>>       GroupEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::VehicleEvents>>>     VehicleEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::BGEvents>>>          BGEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::TicketEvents>>>      TicketEventBindings;
    std::unique_ptr<BindingMap<EventKey<Hooks::AllCreatureEvents>>> AllCreatureEventBindings;

    std::unique_ptr<BindingMap<EntryKey<Hooks::PacketEvents>>>      PacketEventBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::CreatureEvents>>>    CreatureEventBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::GossipEvents>>>      CreatureGossipBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::GameObjectEvents>>>  GameObjectEventBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::GossipEvents>>>      GameObjectGossipBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::ItemEvents>>>        ItemEventBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::GossipEvents>>>      ItemGossipBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::GossipEvents>>>      PlayerGossipBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::InstanceEvents>>>    MapEventBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::InstanceEvents>>>    InstanceEventBindings;
    std::unique_ptr<BindingMap<EntryKey<Hooks::SpellEvents>>>       SpellEventBindings;

    std::unique_ptr<BindingMap<UniqueObjectKey<Hooks::CreatureEvents>>> CreatureUniqueBindings;

    static void Initialize();
    static void Uninitialize();
    // This function is used to make ALE reload
    static void ReloadALE() { LOCK_ALE; reload = true; }
    static LockType& GetLock() { return lock; };
    static bool IsInitialized() { return initialized; }

    void RunScripts();
    bool ShouldReload() const { return reload; }
    bool HasLuaState() const { return stateOpened; }

    /*
     * Registers a Lua handler for an event. Called from the Register* global
     * functions exposed to scripts. Returns a callable that cancels the
     * registration, or raises a Lua error on invalid arguments.
     */
    sol::object Register(uint8 regtype, uint32 entry, ObjectGuid guid, uint32 instanceId,
        uint32 event_id, sol::protected_function callback, uint32 shots);

    // -----------------------------------------------------------------
    // Event dispatch
    //
    // Every game object argument is automatically wrapped into its safe
    // handle (see ALEBind.h); plain values pass through unchanged.
    // -----------------------------------------------------------------

    // Calls one handler with (event_id, args...); reports errors.
    // Returns an invalid result when the handler raised an error.
    template<typename E, typename... Args>
    sol::protected_function_result Call(sol::protected_function const& callback, E event_id, Args&&... args)
    {
        EnterDispatch();
        sol::protected_function_result result = callback(event_id, ALEBind::ToLua(lua, std::forward<Args>(args))...);
        LeaveDispatch();

        if (!result.valid())
            Report(sol::error(result));

        return result;
    }

    // Calls every handler bound to `key`, ignoring their results.
    template<typename K, typename... Args>
    void CallAll(BindingMap<K>& bindings, K const& key, Args&&... args)
    {
        for (sol::protected_function const& callback : bindings.GetCallbacksFor(key))
            Call(callback, key.event_id, args...);
    }

    // Same, for events bound in two maps (creature entry + unique creature).
    template<typename K1, typename K2, typename... Args>
    void CallAll(BindingMap<K1>& bindings1, BindingMap<K2>& bindings2, K1 const& key1, K2 const& key2, Args&&... args)
    {
        CallAll(bindings1, key1, args...);
        CallAll(bindings2, key2, args...);
    }

    /*
     * Calls every handler bound to `key` and folds their first return value:
     * returns `default_value` if every handler returned it (or nothing),
     * the opposite as soon as one handler disagrees.
     *
     * With default_value = false this implements the usual "return true to
     * override default behaviour" hook contract.
     */
    template<typename K, typename... Args>
    bool CallAllBool(BindingMap<K>& bindings, K const& key, bool default_value, Args&&... args)
    {
        bool result = default_value;

        for (sol::protected_function const& callback : bindings.GetCallbacksFor(key))
        {
            sol::protected_function_result callResult = Call(callback, key.event_id, args...);
            if (!callResult.valid())
                continue;

            if (sol::optional<bool> value = callResult.get<sol::optional<bool>>(0))
                if (*value != default_value)
                    result = !default_value;
        }

        return result;
    }

    // Same, for events bound in two maps (creature entry + unique creature).
    template<typename K1, typename K2, typename... Args>
    bool CallAllBool(BindingMap<K1>& bindings1, BindingMap<K2>& bindings2, K1 const& key1, K2 const& key2,
        bool default_value, Args&&... args)
    {
        bool result1 = CallAllBool(bindings1, key1, default_value, args...);
        bool result2 = CallAllBool(bindings2, key2, default_value, args...);
        return (result1 != default_value || result2 != default_value) ? !default_value : default_value;
    }

    /*
     * Calls every handler bound to `key` and threads `value` through them:
     * a handler that returns a value of type V replaces it for the handlers
     * after it. Returns the final value.
     *
     * `invoke` receives (handler, current value) and performs the call, so
     * the caller decides where the value sits among the arguments:
     *
     *     damage = CallAllFold(*bindings, key, damage, [&](auto const& callback, uint32 current)
     *     {
     *         return Call(callback, key.event_id, me, target, current, spellInfo);
     *     });
     */
    template<typename K, typename V, typename Invoker>
    V CallAllFold(BindingMap<K>& bindings, K const& key, V value, Invoker&& invoke)
    {
        return FoldCallbacks(bindings.GetCallbacksFor(key), value, std::forward<Invoker>(invoke));
    }

    // Same fold over an explicit handler snapshot, for events bound in two
    // maps (see GetCreatureCallbacks).
    template<typename V, typename Invoker>
    V FoldCallbacks(std::vector<sol::protected_function> const& callbacks, V value, Invoker&& invoke)
    {
        for (sol::protected_function const& callback : callbacks)
        {
            sol::protected_function_result result = invoke(callback, value);
            if (!result.valid())
                continue;

            if (sol::optional<V> newValue = result.get<sol::optional<V>>(0))
                value = *newValue;
        }

        return value;
    }

    /*
     * Returns the merged handler snapshot for a creature event bound by entry
     * and/or by unique spawn, for hooks that need to inspect each handler's
     * results themselves (modified damage, multiple return values, ...).
     */
    std::vector<sol::protected_function> GetCreatureCallbacks(
        EntryKey<Hooks::CreatureEvents> const& entryKey, UniqueObjectKey<Hooks::CreatureEvents> const& uniqueKey)
    {
        std::vector<sol::protected_function> callbacks = CreatureEventBindings->GetCallbacksFor(entryKey);
        std::vector<sol::protected_function> unique = CreatureUniqueBindings->GetCallbacksFor(uniqueKey);
        callbacks.insert(callbacks.end(), unique.begin(), unique.end());
        return callbacks;
    }

    /*
     * Returns `true` if ALE has instance data for `map`.
     */
    bool HasInstanceData(Map const* map);

    /*
     * Stores `data` as the instance data table for `map`.
     */
    void CreateInstanceData(Map const* map, sol::table data);

    /*
     * Retrieves the instance data table for the `Map` scripted by `ai`.
     *
     * An `ALEInstanceAI` is needed because the instance data might not exist
     * (i.e. ALE has been reloaded). In that case the AI is "reloaded" (a new
     * instance data table is created and loaded with the last known save
     * state, and `Load`/`Initialize` hooks are called).
     */
    sol::table GetInstanceData(ALEInstanceAI* ai);

    CreatureAI* GetAI(Creature* creature);
    InstanceData* GetInstanceData(Map* map);
    void FreeInstanceId(uint32 instanceId);

    /* Custom */
    void OnTimedEvent(sol::protected_function const& callback, uint64 eventId, uint32 delay, uint32 calls, WorldObject* obj);
    bool OnCommand(ChatHandler& handler, const char* text);
    void OnWorldUpdate(uint32 diff);
    void OnLootItem(Player* pPlayer, Item* pItem, uint32 count, ObjectGuid guid);
    void OnLootMoney(Player* pPlayer, uint32 amount);
    void OnFirstLogin(Player* pPlayer);
    void OnEquip(Player* pPlayer, Item* pItem, uint8 bag, uint8 slot);
    void OnRepop(Player* pPlayer);
    void OnResurrect(Player* pPlayer);
    void OnQuestAbandon(Player* pPlayer, uint32 questId);
    void OnLearnTalents(Player* pPlayer, uint32 talentId, uint32 talentRank, uint32 spellid);
    InventoryResult OnCanUseItem(const Player* pPlayer, uint32 itemEntry);
    void OnLuaStateClose();
    void OnLuaStateOpen();
    bool OnAddonMessage(Player* sender, uint32 type, std::string& msg, Player* receiver, Guild* guild, Group* group, Channel* channel);
    void OnPetAddedToWorld(Player* player, Creature* pet);
    void OnQuestRewardItem(Player* player, Item* item, uint32 count);
    void OnCreateItem(Player* player, Item* item, uint32 count);
    void OnStoreNewItem(Player* player, Item* item, uint32 count);
    void OnPlayerCompleteQuest(Player* player, Quest const* quest);

    /* Item */
    void OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, Item* pTarget);
    bool OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest);
    bool OnUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
    bool OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
    bool OnItemGossip(Player* pPlayer, Item* pItem, SpellCastTargets const& targets);
    bool OnExpire(Player* pPlayer, ItemTemplate const* pProto);
    bool OnRemove(Player* pPlayer, Item* item);
    void HandleGossipSelectOption(Player* pPlayer, Item* item, uint32 sender, uint32 action, const std::string& code);

    /* Creature */
    void OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, Creature* pTarget);
    bool OnGossipHello(Player* pPlayer, Creature* pCreature);
    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action);
    bool OnGossipSelectCode(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* code);
    bool OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest);
    bool OnQuestReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest, uint32 opt);
    void GetDialogStatus(const Player* pPlayer, const Creature* pCreature);

    bool OnSummoned(Creature* creature, Unit* summoner);
    bool UpdateAI(Creature* me, const uint32 diff);
    bool EnterCombat(Creature* me, Unit* target);
    bool DamageTaken(Creature* me, Unit* attacker, uint32& damage);
    bool JustDied(Creature* me, Unit* killer);
    bool KilledUnit(Creature* me, Unit* victim);
    bool JustSummoned(Creature* me, Creature* summon);
    bool SummonedCreatureDespawn(Creature* me, Creature* summon);
    bool MovementInform(Creature* me, uint32 type, uint32 id);
    bool AttackStart(Creature* me, Unit* target);
    bool EnterEvadeMode(Creature* me);
    bool JustRespawned(Creature* me);
    bool JustReachedHome(Creature* me);
    bool ReceiveEmote(Creature* me, Player* player, uint32 emoteId);
    bool CorpseRemoved(Creature* me, uint32& respawnDelay);
    bool MoveInLineOfSight(Creature* me, Unit* who);
    bool SpellHit(Creature* me, WorldObject* caster, SpellInfo const* spell);
    bool SpellHitTarget(Creature* me, WorldObject* target, SpellInfo const* spell);
    bool SummonedCreatureDies(Creature* me, Creature* summon, Unit* killer);
    bool OwnerAttackedBy(Creature* me, Unit* attacker);
    bool OwnerAttacked(Creature* me, Unit* target);
    void On_Reset(Creature* me);
    void OnCreatureAuraApply(Creature* me, Aura* aura);
    void OnCreatureHeal(Creature* me, Unit* target, uint32& gain);
    void OnCreatureDamage(Creature* me, Unit* target, uint32& gain);
    void OnCreatureAuraRemove(Creature* me, Aura* aura, AuraRemoveMode mode);
    void OnCreatureModifyPeriodicDamageAurasTick(Creature* me, Unit* target, uint32& damage, SpellInfo const* spellInfo);
    void OnCreatureModifyMeleeDamage(Creature* me, Unit* target, uint32& damage);
    void OnCreatureModifySpellDamageTaken(Creature* me, Unit* target, int32& damage, SpellInfo const* spellInfo);
    void OnCreatureModifyHealReceived(Creature* me, Unit* target, uint32& heal, SpellInfo const* spellInfo);
    uint32 OnCreatureDealDamage(Creature* me, Unit* pVictim, uint32 damage, DamageEffectType damagetype);

    /* GameObject */
    void OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, GameObject* pTarget);
    bool OnGameObjectUse(Player* pPlayer, GameObject* pGameObject);
    bool OnGossipHello(Player* pPlayer, GameObject* pGameObject);
    bool OnGossipSelect(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action);
    bool OnGossipSelectCode(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action, const char* code);
    bool OnQuestAccept(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest);
    bool OnQuestReward(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest, uint32 opt);
    void GetDialogStatus(const Player* pPlayer, const GameObject* pGameObject);
    void OnDestroyed(GameObject* pGameObject, WorldObject* attacker);
    void OnDamaged(GameObject* pGameObject, WorldObject* attacker);
    void OnLootStateChanged(GameObject* pGameObject, uint32 state);
    void OnGameObjectStateChanged(GameObject* pGameObject, uint32 state);
    void UpdateAI(GameObject* pGameObject, uint32 diff);
    void OnSpawn(GameObject* gameobject);

    /* Packet */
    bool OnPacketSend(WorldSession* session, const WorldPacket& packet);
    void OnPacketSendAny(Player* player, const WorldPacket& packet, bool& result);
    void OnPacketSendOne(Player* player, const WorldPacket& packet, bool& result);
    bool OnPacketReceive(WorldSession* session, WorldPacket const& packet);
    void OnPacketReceiveAny(Player* player, WorldPacket const& packet, bool& result);
    void OnPacketReceiveOne(Player* player, WorldPacket const& packet, bool& result);

    /* Player */
    void OnPlayerEnterCombat(Player* pPlayer, Unit* pEnemy);
    void OnPlayerLeaveCombat(Player* pPlayer);
    void OnPVPKill(Player* pKiller, Player* pKilled);
    void OnCreatureKill(Player* pKiller, Creature* pKilled);
    void OnPlayerKilledByCreature(Creature* pKiller, Player* pKilled);
    void OnLevelChanged(Player* pPlayer, uint8 oldLevel);
    void OnFreeTalentPointsChanged(Player* pPlayer, uint32 newPoints);
    void OnTalentsReset(Player* pPlayer, bool noCost);
    void OnMoneyChanged(Player* pPlayer, int32& amount);
    void OnGiveXP(Player* pPlayer, uint32& amount, Unit* pVictim, uint8 xpSource);
    bool OnReputationChange(Player* pPlayer, uint32 factionID, int32& standing, bool incremental);
    void OnDuelRequest(Player* pTarget, Player* pChallenger);
    void OnDuelStart(Player* pStarter, Player* pChallenger);
    void OnDuelEnd(Player* pWinner, Player* pLoser, DuelCompleteType type);
    bool OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg);
    bool OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Group* pGroup);
    bool OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Guild* pGuild);
    bool OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Channel* pChannel);
    bool OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Player* pReceiver);
    bool DispatchChatEvent(EventKey<Hooks::PlayerEvents> const& key, Player* pPlayer, std::string& msg,
        uint32 type, uint32 lang, sol::optional<sol::object> extra);
    void OnEmote(Player* pPlayer, uint32 emote);
    void OnTextEmote(Player* pPlayer, uint32 textEmote, uint32 emoteNum, ObjectGuid guid);
    void OnPlayerSpellCast(Player* pPlayer, Spell* pSpell, bool skipCheck);
    void OnLogin(Player* pPlayer);
    void OnLogout(Player* pPlayer);
    void OnCreate(Player* pPlayer);
    void OnDelete(uint32 guid);
    void OnSave(Player* pPlayer);
    void OnBindToInstance(Player* pPlayer, Difficulty difficulty, uint32 mapid, bool permanent);
    void OnUpdateArea(Player* pPlayer, uint32 oldArea, uint32 newArea);
    void OnUpdateZone(Player* pPlayer, uint32 newZone, uint32 newArea);
    void OnMapChanged(Player* pPlayer);
    void HandleGossipSelectOption(Player* pPlayer, uint32 menuId, uint32 sender, uint32 action, const std::string& code);
    void OnLearnSpell(Player* player, uint32 spellId);
    void OnAchiComplete(Player* player, AchievementEntry const* achievement);
    void OnFfaPvpStateUpdate(Player* player, bool hasFfaPvp);
    bool OnCanInitTrade(Player* player, Player* target);
    bool OnCanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint32 money, uint32 cod, Item* item);
    bool OnCanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, const std::string& comment);
    bool OnCanGroupInvite(Player* player, std::string& memberName);
    void OnGroupRollRewardItem(Player* player, Item* item, uint32 count, RollVote voteType, Roll* roll);
    void OnBattlegroundDesertion(Player* player, const BattlegroundDesertionType type);
    void OnCreatureKilledByPet(Player* player, Creature* killed);
    bool OnPlayerCanUpdateSkill(Player* player, uint32 skill_id);
    void OnPlayerBeforeUpdateSkill(Player* player, uint32 skill_id, uint32& value, uint32 max, uint32 step);
    void OnPlayerUpdateSkill(Player* player, uint32 skill_id, uint32 value, uint32 max, uint32 step, uint32 new_value);
    bool CanPlayerResurrect(Player* player);
    void OnPlayerQuestAccept(Player* player, Quest const* quest);
    void OnPlayerAuraApply(Player* player, Aura* aura);
    void OnPlayerAuraRemove(Player* player, Aura* aura, AuraRemoveMode mode);
    void OnPlayerHeal(Player* player, Unit* target, uint32& gain);
    void OnPlayerDamage(Player* player, Unit* target, uint32& damage);
    void OnPlayerModifyPeriodicDamageAurasTick(Player* player, Unit* target, uint32& damage, SpellInfo const* spellInfo);
    void OnPlayerModifyMeleeDamage(Player* player, Unit* target, uint32& damage);
    void OnPlayerModifySpellDamageTaken(Player* player, Unit* target, int32& damage, SpellInfo const* spellInfo);
    void OnPlayerModifyHealReceived(Player* player, Unit* target, uint32& heal, SpellInfo const* spellInfo);
    uint32 OnPlayerDealDamage(Player* player, Unit* pVictim, uint32 damage, DamageEffectType damagetype);
    void OnPlayerReleasedGhost(Player* player);

    /* Vehicle */
    void OnInstall(Vehicle* vehicle);
    void OnUninstall(Vehicle* vehicle);
    void OnInstallAccessory(Vehicle* vehicle, Creature* accessory);
    void OnAddPassenger(Vehicle* vehicle, Unit* passenger, int8 seatId);
    void OnRemovePassenger(Vehicle* vehicle, Unit* passenger);

    /* AreaTrigger */
    bool OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* pTrigger);

    /* Weather */
    void OnChange(Weather* weather, uint32 zone, WeatherState state, float grade);

    /* Auction House */
    void OnAdd(AuctionHouseObject* ah, AuctionEntry* entry);
    void OnRemove(AuctionHouseObject* ah, AuctionEntry* entry);
    void OnSuccessful(AuctionHouseObject* ah, AuctionEntry* entry);
    void OnExpire(AuctionHouseObject* ah, AuctionEntry* entry);

    /* Guild */
    void OnAddMember(Guild* guild, Player* player, uint32 plRank);
    void OnRemoveMember(Guild* guild, Player* player, bool isDisbanding);
    void OnMOTDChanged(Guild* guild, const std::string& newMotd);
    void OnInfoChanged(Guild* guild, const std::string& newInfo);
    void OnCreate(Guild* guild, Player* leader, const std::string& name);
    void OnDisband(Guild* guild);
    void OnMemberWitdrawMoney(Guild* guild, Player* player, uint32& amount, bool isRepair);
    void OnMemberDepositMoney(Guild* guild, Player* player, uint32& amount);
    void OnItemMove(Guild* guild, Player* player, Item* pItem, bool isSrcBank, uint8 srcContainer, uint8 srcSlotId, bool isDestBank, uint8 destContainer, uint8 destSlotId);
    void OnEvent(Guild* guild, uint8 eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank);
    void OnBankEvent(Guild* guild, uint8 eventType, uint8 tabId, uint32 playerGuid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId);

    /* Group */
    void OnAddMember(Group* group, ObjectGuid guid);
    void OnInviteMember(Group* group, ObjectGuid guid);
    void OnRemoveMember(Group* group, ObjectGuid guid, uint8 method);
    void OnChangeLeader(Group* group, ObjectGuid newLeaderGuid, ObjectGuid oldLeaderGuid);
    void OnDisband(Group* group);
    void OnCreate(Group* group, ObjectGuid leaderGuid, GroupType groupType);

    /* Map */
    void OnCreate(Map* map);
    void OnDestroy(Map* map);
    void OnPlayerEnter(Map* map, Player* player);
    void OnPlayerLeave(Map* map, Player* player);
    void OnUpdate(Map* map, uint32 diff);
    void OnAddToWorld(Creature* creature);
    void OnRemoveFromWorld(Creature* creature);
    void OnAddToWorld(GameObject* gameobject);
    void OnRemoveFromWorld(GameObject* gameobject);
    void OnRemove(Creature* creature);
    void OnRemove(GameObject* gameobject);

    /* Instance */
    void OnInitialize(ALEInstanceAI* ai);
    void OnLoad(ALEInstanceAI* ai);
    void OnUpdateInstance(ALEInstanceAI* ai, uint32 diff);
    void OnPlayerEnterInstance(ALEInstanceAI* ai, Player* player);
    void OnCreatureCreate(ALEInstanceAI* ai, Creature* creature);
    void OnGameObjectCreate(ALEInstanceAI* ai, GameObject* gameobject);
    bool OnCheckEncounterInProgress(ALEInstanceAI* ai);

    /* World */
    void OnOpenStateChange(bool open);
    void OnConfigLoad(bool reload, bool isBefore);
    void OnShutdownInitiate(ShutdownExitCode code, ShutdownMask mask);
    void OnShutdownCancel();
    void OnStartup();
    void OnShutdown();
    void OnGameEventStart(uint32 eventid);
    void OnGameEventStop(uint32 eventid);

    /* Battle Ground */
    void OnBGStart(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId);
    void OnBGEnd(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId, TeamId winner);
    void OnBGCreate(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId);
    void OnBGDestroy(BattleGround* bg, BattleGroundTypeId bgId, uint32 instanceId);

    /* Ticket */
    void OnTicketCreate(GmTicket* ticket);
    void OnTicketClose(GmTicket* ticket);
    void OnTicketUpdateLastChange(GmTicket* ticket);
    void OnTicketResolve(GmTicket* ticket);

    /* Spell */
    void OnSpellPrepare(Unit* caster, Spell* spell, SpellInfo const* spellInfo);
    void OnSpellCast(Unit* caster, Spell* spell, SpellInfo const* spellInfo, bool skipCheck);
    void OnSpellCastCancel(Unit* caster, Spell* spell, SpellInfo const* spellInfo, bool bySelf);

    /* AllCreature */
    void OnAllCreatureAddToWorld(Creature* creature);
    void OnAllCreatureRemoveFromWorld(Creature* creature);
    void OnAllCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature);
    void OnAllCreatureBeforeSelectLevel(const CreatureTemplate* cinfo, Creature* creature, uint8& level);
    void OnAllCreatureAuraApply(Creature* me, Aura* aura);
    void OnAllCreatureHeal(Creature* me, Unit* target, uint32& gain);
    void OnAllCreatureDamage(Creature* me, Unit* target, uint32& gain);
    void OnAllCreatureAuraRemove(Creature* me, Aura* aura, AuraRemoveMode mode);
    void OnAllCreatureModifyPeriodicDamageAurasTick(Creature* me, Unit* target, uint32& damage, SpellInfo const* spellInfo);
    void OnAllCreatureModifyMeleeDamage(Creature* me, Unit* target, uint32& damage);
    void OnAllCreatureModifySpellDamageTaken(Creature* me, Unit* target, int32& damage, SpellInfo const* spellInfo);
    void OnAllCreatureModifyHealReceived(Creature* me, Unit* target, uint32& heal, SpellInfo const* spellInfo);
    uint32 OnAllCreatureDealDamage(Creature* me, Unit* pVictim, uint32 damage, DamageEffectType damagetype);
};

#define sALE ALE::GALE

#endif // _LUA_ENGINE_H
