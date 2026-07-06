/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "Channel.h"

#include <algorithm>

using namespace Hooks;

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<PlayerEvents>(EVENT);\
    if (!PlayerEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EventKey<PlayerEvents>(EVENT);\
    if (!PlayerEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

void ALE::OnLearnTalents(Player* pPlayer, uint32 talentId, uint32 talentRank, uint32 spellid)
{
    START_HOOK(PLAYER_EVENT_ON_LEARN_TALENTS);
    CallAll(*PlayerEventBindings, key, pPlayer, talentId, talentRank, spellid);
}

bool ALE::OnCommand(ChatHandler& handler, const char* text)
{
    Player* player = handler.IsConsole() ? nullptr : handler.GetSession()->GetPlayer();
    // If from console, player is NULL
    if (!player || player->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR)
    {
        std::string reload = text;
        std::transform(reload.begin(), reload.end(), reload.begin(), ::tolower);
        if (reload.find("reload ale") == 0)
        {
            ReloadALE();
            return false;
        }
    }

    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_COMMAND, true);
    return CallAllBool(*PlayerEventBindings, key, true, player, text, &handler);
}

void ALE::OnLootItem(Player* pPlayer, Item* pItem, uint32 count, ObjectGuid guid)
{
    START_HOOK(PLAYER_EVENT_ON_LOOT_ITEM);
    CallAll(*PlayerEventBindings, key, pPlayer, pItem, count, guid);
}

void ALE::OnLootMoney(Player* pPlayer, uint32 amount)
{
    START_HOOK(PLAYER_EVENT_ON_LOOT_MONEY);
    CallAll(*PlayerEventBindings, key, pPlayer, amount);
}

void ALE::OnFirstLogin(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_FIRST_LOGIN);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnRepop(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_REPOP);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnResurrect(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_RESURRECT);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnQuestAbandon(Player* pPlayer, uint32 questId)
{
    START_HOOK(PLAYER_EVENT_ON_QUEST_ABANDON);
    CallAll(*PlayerEventBindings, key, pPlayer, questId);
}

void ALE::OnEquip(Player* pPlayer, Item* pItem, uint8 bag, uint8 slot)
{
    START_HOOK(PLAYER_EVENT_ON_EQUIP);
    CallAll(*PlayerEventBindings, key, pPlayer, pItem, bag, slot);
}

InventoryResult ALE::OnCanUseItem(const Player* pPlayer, uint32 itemEntry)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_USE_ITEM, EQUIP_ERR_OK);

    // A handler that returns a number overrides the equip result.
    uint32 result = CallAllFold(*PlayerEventBindings, key, static_cast<uint32>(EQUIP_ERR_OK),
        [&](auto const& callback, uint32 /*current*/)
    {
        return Call(callback, key.event_id, pPlayer, itemEntry);
    });

    return static_cast<InventoryResult>(result);
}

void ALE::OnPlayerEnterCombat(Player* pPlayer, Unit* pEnemy)
{
    START_HOOK(PLAYER_EVENT_ON_ENTER_COMBAT);
    CallAll(*PlayerEventBindings, key, pPlayer, pEnemy);
}

void ALE::OnPlayerLeaveCombat(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_LEAVE_COMBAT);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnPVPKill(Player* pKiller, Player* pKilled)
{
    START_HOOK(PLAYER_EVENT_ON_KILL_PLAYER);
    CallAll(*PlayerEventBindings, key, pKiller, pKilled);
}

void ALE::OnCreatureKill(Player* pKiller, Creature* pKilled)
{
    START_HOOK(PLAYER_EVENT_ON_KILL_CREATURE);
    CallAll(*PlayerEventBindings, key, pKiller, pKilled);
}

void ALE::OnPlayerKilledByCreature(Creature* pKiller, Player* pKilled)
{
    START_HOOK(PLAYER_EVENT_ON_KILLED_BY_CREATURE);
    CallAll(*PlayerEventBindings, key, pKiller, pKilled);
}

void ALE::OnLevelChanged(Player* pPlayer, uint8 oldLevel)
{
    START_HOOK(PLAYER_EVENT_ON_LEVEL_CHANGE);
    CallAll(*PlayerEventBindings, key, pPlayer, oldLevel);
}

void ALE::OnFreeTalentPointsChanged(Player* pPlayer, uint32 newPoints)
{
    START_HOOK(PLAYER_EVENT_ON_TALENTS_CHANGE);
    CallAll(*PlayerEventBindings, key, pPlayer, newPoints);
}

void ALE::OnTalentsReset(Player* pPlayer, bool noCost)
{
    START_HOOK(PLAYER_EVENT_ON_TALENTS_RESET);
    CallAll(*PlayerEventBindings, key, pPlayer, noCost);
}

void ALE::OnMoneyChanged(Player* pPlayer, int32& amount)
{
    START_HOOK(PLAYER_EVENT_ON_MONEY_CHANGE);
    amount = CallAllFold(*PlayerEventBindings, key, amount, [&](auto const& callback, int32 current)
    {
        return Call(callback, key.event_id, pPlayer, current);
    });
}

void ALE::OnGiveXP(Player* pPlayer, uint32& amount, Unit* pVictim, uint8 xpSource)
{
    START_HOOK(PLAYER_EVENT_ON_GIVE_XP);
    amount = CallAllFold(*PlayerEventBindings, key, amount, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, pPlayer, current, pVictim, xpSource);
    });
}

bool ALE::OnReputationChange(Player* pPlayer, uint32 factionID, int32& standing, bool incremental)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_REPUTATION_CHANGE, true);
    bool result = true;

    // A handler that returns a number changes the new standing;
    // returning -1 blocks the change entirely.
    for (sol::protected_function const& callback : PlayerEventBindings->GetCallbacksFor(key))
    {
        sol::protected_function_result callResult = Call(callback, key.event_id, pPlayer, factionID, standing, incremental);
        if (!callResult.valid())
            continue;

        if (sol::optional<int32> newStanding = callResult.get<sol::optional<int32>>(0))
        {
            standing = *newStanding;
            if (standing == -1)
                result = false;
        }
    }

    return result;
}

void ALE::OnDuelRequest(Player* pTarget, Player* pChallenger)
{
    START_HOOK(PLAYER_EVENT_ON_DUEL_REQUEST);
    CallAll(*PlayerEventBindings, key, pTarget, pChallenger);
}

void ALE::OnDuelStart(Player* pStarter, Player* pChallenger)
{
    START_HOOK(PLAYER_EVENT_ON_DUEL_START);
    CallAll(*PlayerEventBindings, key, pStarter, pChallenger);
}

void ALE::OnDuelEnd(Player* pWinner, Player* pLoser, DuelCompleteType type)
{
    START_HOOK(PLAYER_EVENT_ON_DUEL_END);
    CallAll(*PlayerEventBindings, key, pWinner, pLoser, type);
}

void ALE::OnEmote(Player* pPlayer, uint32 emote)
{
    START_HOOK(PLAYER_EVENT_ON_EMOTE);
    CallAll(*PlayerEventBindings, key, pPlayer, emote);
}

void ALE::OnTextEmote(Player* pPlayer, uint32 textEmote, uint32 emoteNum, ObjectGuid guid)
{
    START_HOOK(PLAYER_EVENT_ON_TEXT_EMOTE);
    CallAll(*PlayerEventBindings, key, pPlayer, textEmote, emoteNum, guid);
}

void ALE::OnPlayerSpellCast(Player* pPlayer, Spell* pSpell, bool skipCheck)
{
    START_HOOK(PLAYER_EVENT_ON_SPELL_CAST);
    CallAll(*PlayerEventBindings, key, pPlayer, pSpell, skipCheck);
}

void ALE::OnLogin(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_LOGIN);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnLogout(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_LOGOUT);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnCreate(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_CHARACTER_CREATE);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnDelete(uint32 guidlow)
{
    START_HOOK(PLAYER_EVENT_ON_CHARACTER_DELETE);
    CallAll(*PlayerEventBindings, key, guidlow);
}

void ALE::OnSave(Player* pPlayer)
{
    START_HOOK(PLAYER_EVENT_ON_SAVE);
    CallAll(*PlayerEventBindings, key, pPlayer);
}

void ALE::OnBindToInstance(Player* pPlayer, Difficulty difficulty, uint32 mapid, bool permanent)
{
    START_HOOK(PLAYER_EVENT_ON_BIND_TO_INSTANCE);
    CallAll(*PlayerEventBindings, key, pPlayer, difficulty, mapid, permanent);
}

void ALE::OnUpdateArea(Player* pPlayer, uint32 oldArea, uint32 newArea)
{
    START_HOOK(PLAYER_EVENT_ON_UPDATE_AREA);
    CallAll(*PlayerEventBindings, key, pPlayer, oldArea, newArea);
}

void ALE::OnUpdateZone(Player* pPlayer, uint32 newZone, uint32 newArea)
{
    START_HOOK(PLAYER_EVENT_ON_UPDATE_ZONE);
    CallAll(*PlayerEventBindings, key, pPlayer, newZone, newArea);
}

void ALE::OnMapChanged(Player* player)
{
    START_HOOK(PLAYER_EVENT_ON_MAP_CHANGE);
    CallAll(*PlayerEventBindings, key, player);
}

/*
 * Shared implementation of the chat events: handlers return
 * (false to block the message, new message text to rewrite it).
 * `extra` is the chat context (group, guild, channel id, receiver, ...).
 */
bool ALE::DispatchChatEvent(EventKey<Hooks::PlayerEvents> const& key, Player* pPlayer, std::string& msg,
    uint32 type, uint32 lang, sol::optional<sol::object> extra)
{
    bool result = true;

    for (sol::protected_function const& callback : PlayerEventBindings->GetCallbacksFor(key))
    {
        sol::protected_function_result callResult = extra
            ? Call(callback, key.event_id, pPlayer, msg, type, lang, *extra)
            : Call(callback, key.event_id, pPlayer, msg, type, lang);
        if (!callResult.valid())
            continue;

        if (callResult.get<sol::optional<bool>>(0) == sol::optional<bool>(false))
            result = false;

        if (sol::optional<std::string> newMessage = callResult.get<sol::optional<std::string>>(1))
            msg = *newMessage;
    }

    return result;
}

bool ALE::OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg)
{
    if (lang == LANG_ADDON)
        return OnAddonMessage(pPlayer, type, msg, nullptr, nullptr, nullptr, nullptr);

    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CHAT, true);
    return DispatchChatEvent(key, pPlayer, msg, type, lang, sol::nullopt);
}

bool ALE::OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Group* pGroup)
{
    if (lang == LANG_ADDON)
        return OnAddonMessage(pPlayer, type, msg, nullptr, nullptr, pGroup, nullptr);

    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_GROUP_CHAT, true);
    return DispatchChatEvent(key, pPlayer, msg, type, lang, sol::make_object(lua, GroupRef(pGroup)));
}

bool ALE::OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Guild* pGuild)
{
    if (lang == LANG_ADDON)
        return OnAddonMessage(pPlayer, type, msg, nullptr, pGuild, nullptr, nullptr);

    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_GUILD_CHAT, true);
    return DispatchChatEvent(key, pPlayer, msg, type, lang, sol::make_object(lua, GuildRef(pGuild)));
}

bool ALE::OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Channel* pChannel)
{
    if (lang == LANG_ADDON)
        return OnAddonMessage(pPlayer, type, msg, nullptr, nullptr, nullptr, pChannel);

    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CHANNEL_CHAT, true);

    // Built-in channels are passed by id, custom channels by negated DB id.
    int32 channelId = pChannel->IsConstant()
        ? static_cast<int32>(pChannel->GetChannelId())
        : -static_cast<int32>(pChannel->GetChannelDBId());
    return DispatchChatEvent(key, pPlayer, msg, type, lang, sol::make_object(lua, channelId));
}

bool ALE::OnChat(Player* pPlayer, uint32 type, uint32 lang, std::string& msg, Player* pReceiver)
{
    if (lang == LANG_ADDON)
        return OnAddonMessage(pPlayer, type, msg, pReceiver, nullptr, nullptr, nullptr);

    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_WHISPER, true);
    return DispatchChatEvent(key, pPlayer, msg, type, lang, sol::make_object(lua, PlayerRef(pReceiver)));
}

void ALE::OnPetAddedToWorld(Player* player, Creature* pet)
{
    START_HOOK(PLAYER_EVENT_ON_PET_ADDED_TO_WORLD);
    CallAll(*PlayerEventBindings, key, player, pet);
}

void ALE::OnLearnSpell(Player* player, uint32 spellId)
{
    START_HOOK(PLAYER_EVENT_ON_LEARN_SPELL);
    CallAll(*PlayerEventBindings, key, player, spellId);
}

void ALE::OnAchiComplete(Player* player, AchievementEntry const* achievement)
{
    START_HOOK(PLAYER_EVENT_ON_ACHIEVEMENT_COMPLETE);
    CallAll(*PlayerEventBindings, key, player, achievement);
}

void ALE::OnFfaPvpStateUpdate(Player* player, bool hasFfaPvp)
{
    START_HOOK(PLAYER_EVENT_ON_FFAPVP_CHANGE);
    CallAll(*PlayerEventBindings, key, player, hasFfaPvp);
}

bool ALE::OnCanInitTrade(Player* player, Player* target)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_INIT_TRADE, true);
    return CallAllBool(*PlayerEventBindings, key, true, player, target);
}

bool ALE::OnCanSendMail(Player* player, ObjectGuid receiverGuid, ObjectGuid mailbox, std::string& subject, std::string& body, uint32 money, uint32 cod, Item* item)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_SEND_MAIL, true);
    return CallAllBool(*PlayerEventBindings, key, true, player, receiverGuid, mailbox, subject, body, money, cod, item);
}

bool ALE::OnCanJoinLfg(Player* player, uint8 roles, lfg::LfgDungeonSet& dungeons, const std::string& comment)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_JOIN_LFG, true);

    // The dungeon set is passed as an array-like table.
    sol::table dungeonTable = lua.create_table();
    uint32 counter = 1;
    for (uint32 dungeon : dungeons)
        dungeonTable[counter++] = dungeon;

    return CallAllBool(*PlayerEventBindings, key, true, player, roles, dungeonTable, comment);
}

void ALE::OnQuestRewardItem(Player* player, Item* item, uint32 count)
{
    START_HOOK(PLAYER_EVENT_ON_QUEST_REWARD_ITEM);
    CallAll(*PlayerEventBindings, key, player, item, count);
}

void ALE::OnCreateItem(Player* player, Item* item, uint32 count)
{
    START_HOOK(PLAYER_EVENT_ON_CREATE_ITEM);
    CallAll(*PlayerEventBindings, key, player, item, count);
}

void ALE::OnStoreNewItem(Player* player, Item* item, uint32 count)
{
    START_HOOK(PLAYER_EVENT_ON_STORE_NEW_ITEM);
    CallAll(*PlayerEventBindings, key, player, item, count);
}

void ALE::OnPlayerCompleteQuest(Player* player, Quest const* quest)
{
    START_HOOK(PLAYER_EVENT_ON_COMPLETE_QUEST);
    CallAll(*PlayerEventBindings, key, player, quest);
}

bool ALE::OnCanGroupInvite(Player* player, std::string& memberName)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_GROUP_INVITE, true);
    return CallAllBool(*PlayerEventBindings, key, true, player, memberName);
}

void ALE::OnGroupRollRewardItem(Player* player, Item* item, uint32 count, RollVote voteType, Roll* roll)
{
    START_HOOK(PLAYER_EVENT_ON_GROUP_ROLL_REWARD_ITEM);
    CallAll(*PlayerEventBindings, key, player, item, count, voteType, roll);
}

void ALE::OnBattlegroundDesertion(Player* player, const BattlegroundDesertionType type)
{
    START_HOOK(PLAYER_EVENT_ON_BG_DESERTION);
    CallAll(*PlayerEventBindings, key, player, type);
}

void ALE::OnCreatureKilledByPet(Player* player, Creature* killed)
{
    START_HOOK(PLAYER_EVENT_ON_PET_KILL);
    CallAll(*PlayerEventBindings, key, player, killed);
}

bool ALE::OnPlayerCanUpdateSkill(Player* player, uint32 skill_id)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_UPDATE_SKILL, true);
    return CallAllBool(*PlayerEventBindings, key, true, player, skill_id);
}

void ALE::OnPlayerBeforeUpdateSkill(Player* player, uint32 skill_id, uint32& value, uint32 max, uint32 step)
{
    START_HOOK(PLAYER_EVENT_ON_BEFORE_UPDATE_SKILL);
    value = CallAllFold(*PlayerEventBindings, key, value, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, skill_id, current, max, step);
    });
}

void ALE::OnPlayerUpdateSkill(Player* player, uint32 skill_id, uint32 value, uint32 max, uint32 step, uint32 new_value)
{
    START_HOOK(PLAYER_EVENT_ON_UPDATE_SKILL);
    CallAll(*PlayerEventBindings, key, player, skill_id, value, max, step, new_value);
}

bool ALE::CanPlayerResurrect(Player* player)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_CAN_RESURRECT, true);
    return CallAllBool(*PlayerEventBindings, key, true, player);
}

void ALE::OnPlayerReleasedGhost(Player* player)
{
    START_HOOK(PLAYER_EVENT_ON_RELEASED_GHOST);
    CallAll(*PlayerEventBindings, key, player);
}

void ALE::OnPlayerQuestAccept(Player* player, Quest const* quest)
{
    START_HOOK(PLAYER_EVENT_ON_QUEST_ACCEPT);
    CallAll(*PlayerEventBindings, key, player, quest);
}

void ALE::OnPlayerAuraApply(Player* player, Aura* aura)
{
    START_HOOK(PLAYER_EVENT_ON_AURA_APPLY);
    CallAll(*PlayerEventBindings, key, player, aura);
}

void ALE::OnPlayerHeal(Player* player, Unit* target, uint32& gain)
{
    START_HOOK(PLAYER_EVENT_ON_HEAL);
    gain = CallAllFold(*PlayerEventBindings, key, gain, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, target, current);
    });
}

void ALE::OnPlayerDamage(Player* player, Unit* target, uint32& damage)
{
    START_HOOK(PLAYER_EVENT_ON_DAMAGE);
    damage = CallAllFold(*PlayerEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, target, current);
    });
}

void ALE::OnPlayerAuraRemove(Player* player, Aura* aura, AuraRemoveMode mode)
{
    START_HOOK(PLAYER_EVENT_ON_AURA_REMOVE);
    CallAll(*PlayerEventBindings, key, player, aura, mode);
}

void ALE::OnPlayerModifyPeriodicDamageAurasTick(Player* player, Unit* target, uint32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(PLAYER_EVENT_ON_MODIFY_PERIODIC_DAMAGE_AURAS_TICK);
    damage = CallAllFold(*PlayerEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, target, current, spellInfo);
    });
}

void ALE::OnPlayerModifyMeleeDamage(Player* player, Unit* target, uint32& damage)
{
    START_HOOK(PLAYER_EVENT_ON_MODIFY_MELEE_DAMAGE);
    damage = CallAllFold(*PlayerEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, target, current);
    });
}

void ALE::OnPlayerModifySpellDamageTaken(Player* player, Unit* target, int32& damage, SpellInfo const* spellInfo)
{
    START_HOOK(PLAYER_EVENT_ON_MODIFY_SPELL_DAMAGE_TAKEN);
    damage = CallAllFold(*PlayerEventBindings, key, damage, [&](auto const& callback, int32 current)
    {
        return Call(callback, key.event_id, player, target, current, spellInfo);
    });
}

void ALE::OnPlayerModifyHealReceived(Player* player, Unit* target, uint32& heal, SpellInfo const* spellInfo)
{
    START_HOOK(PLAYER_EVENT_ON_MODIFY_HEAL_RECEIVED);
    heal = CallAllFold(*PlayerEventBindings, key, heal, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, target, current, spellInfo);
    });
}

uint32 ALE::OnPlayerDealDamage(Player* player, Unit* target, uint32 damage, DamageEffectType damagetype)
{
    START_HOOK_WITH_RETVAL(PLAYER_EVENT_ON_DEAL_DAMAGE, damage);
    return CallAllFold(*PlayerEventBindings, key, damage, [&](auto const& callback, uint32 current)
    {
        return Call(callback, key.event_id, player, target, current, damagetype);
    });
}
