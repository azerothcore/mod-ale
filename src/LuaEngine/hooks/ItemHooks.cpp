/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEIncludes.h"
#include "ALETemplate.h"

using namespace Hooks;

#define START_HOOK(EVENT, ENTRY) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<ItemEvents>(EVENT, ENTRY);\
    if (!ItemEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, ENTRY, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EntryKey<ItemEvents>(EVENT, ENTRY);\
    if (!ItemEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

void ALE::OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, Item* pTarget)
{
    START_HOOK(ITEM_EVENT_ON_DUMMY_EFFECT, pTarget->GetEntry());
    Push(pCaster);
    Push(spellId);
    Push(effIndex);
    Push(pTarget);
    CallAllFunctions(ItemEventBindings, key);
}

bool ALE::OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest)
{
    START_HOOK_WITH_RETVAL(ITEM_EVENT_ON_QUEST_ACCEPT, pItem->GetEntry(), false);
    Push(pPlayer);
    Push(pItem);
    Push(pQuest);
    return CallAllFunctionsBool(ItemEventBindings, key);
}

bool ALE::OnUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets)
{
    ObjectGuid guid = pItem->GET_GUID();
    bool castSpell = true;

    if (!OnItemUse(pPlayer, pItem, targets))
        castSpell = false;

    pItem = pPlayer->GetItemByGuid(guid);
    if (pItem)
    {
        if (!OnItemGossip(pPlayer, pItem, targets))
            castSpell = false;
        pItem = pPlayer->GetItemByGuid(guid);
    }

    if (pItem && castSpell)
        return true;

    // Send equip error that shows no message
    // This is a hack fix to stop spell casting visual bug when a spell is not cast on use
    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, 18);
    data << uint8(59); // EQUIP_ERR_NONE / EQUIP_ERR_CANT_BE_DISENCHANTED
    data << guid;
    data << ObjectGuid(uint64(0));
    data << uint8(0);
    pPlayer->GetSession()->SendPacket(&data);
    return false;
}

bool ALE::OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets)
{
    START_HOOK_WITH_RETVAL(ITEM_EVENT_ON_USE, pItem->GetEntry(), true);
    Push(pPlayer);
    Push(pItem);

    if (GameObject* target = targets.GetGOTarget())
        Push(target);
    else if (Item* target = targets.GetItemTarget())
        Push(target);
    else if (Corpse* target = targets.GetCorpseTarget())
        Push(target);
    else if (Unit* target = targets.GetUnitTarget())
        Push(target);
    else if (WorldObject* target = targets.GetObjectTarget())
        Push(target);
    else
        Push();

    return CallAllFunctionsBool(ItemEventBindings, key, true);
}

bool ALE::OnExpire(Player* pPlayer, ItemTemplate const* pProto)
{
    START_HOOK_WITH_RETVAL(ITEM_EVENT_ON_EXPIRE, pProto->ItemId, false);
    Push(pPlayer);
    Push(pProto->ItemId);
    return CallAllFunctionsBool(ItemEventBindings, key);
}

bool ALE::OnRemove(Player* pPlayer, Item* pItem)
{
    START_HOOK_WITH_RETVAL(ITEM_EVENT_ON_REMOVE, pItem->GetEntry(), false);
    Push(pPlayer);
    Push(pItem);
    return CallAllFunctionsBool(ItemEventBindings, key);
}
