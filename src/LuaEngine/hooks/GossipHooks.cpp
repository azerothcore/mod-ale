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

#define START_HOOK(BINDINGS, EVENT, ENTRY) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<GossipEvents>(EVENT, ENTRY);\
    if (!BINDINGS->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(BINDINGS, EVENT, ENTRY, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EntryKey<GossipEvents>(EVENT, ENTRY);\
    if (!BINDINGS->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

bool ALE::OnGossipHello(Player* pPlayer, GameObject* pGameObject)
{
    START_HOOK_WITH_RETVAL(GameObjectGossipBindings, GOSSIP_EVENT_ON_HELLO, pGameObject->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pGameObject);
    return CallAllFunctionsBool(GameObjectGossipBindings, key, true);
}

bool ALE::OnGossipSelect(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action)
{
    START_HOOK_WITH_RETVAL(GameObjectGossipBindings, GOSSIP_EVENT_ON_SELECT, pGameObject->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pGameObject);
    Push(sender);
    Push(action);
    return CallAllFunctionsBool(GameObjectGossipBindings, key, true);
}

bool ALE::OnGossipSelectCode(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action, const char* code)
{
    START_HOOK_WITH_RETVAL(GameObjectGossipBindings, GOSSIP_EVENT_ON_SELECT, pGameObject->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pGameObject);
    Push(sender);
    Push(action);
    Push(code);
    return CallAllFunctionsBool(GameObjectGossipBindings, key, true);
}

void ALE::HandleGossipSelectOption(Player* pPlayer, uint32 menuId, uint32 sender, uint32 action, const std::string& code)
{
    START_HOOK(PlayerGossipBindings, GOSSIP_EVENT_ON_SELECT, menuId);
    pPlayer->PlayerTalkClass->ClearMenus();

    Push(pPlayer); // receiver
    Push(pPlayer); // sender, just not to mess up the amount of args.
    Push(sender);
    Push(action);
    if (code.empty())
        Push();
    else
        Push(code);

    CallAllFunctions(PlayerGossipBindings, key);
}

bool ALE::OnItemGossip(Player* pPlayer, Item* pItem, SpellCastTargets const& /*targets*/)
{
    START_HOOK_WITH_RETVAL(ItemGossipBindings, GOSSIP_EVENT_ON_HELLO, pItem->GetEntry(), true);
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pItem);
    return CallAllFunctionsBool(ItemGossipBindings, key, true);
}

void ALE::HandleGossipSelectOption(Player* pPlayer, Item* pItem, uint32 sender, uint32 action, const std::string& code)
{
    START_HOOK(ItemGossipBindings, GOSSIP_EVENT_ON_SELECT, pItem->GetEntry());
    pPlayer->PlayerTalkClass->ClearMenus();

    Push(pPlayer);
    Push(pItem);
    Push(sender);
    Push(action);
    if (code.empty())
        Push();
    else
        Push(code);

    CallAllFunctions(ItemGossipBindings, key);
}

bool ALE::OnGossipHello(Player* pPlayer, Creature* pCreature)
{
    START_HOOK_WITH_RETVAL(CreatureGossipBindings, GOSSIP_EVENT_ON_HELLO, pCreature->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pCreature);
    return CallAllFunctionsBool(CreatureGossipBindings, key, true);
}

bool ALE::OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    START_HOOK_WITH_RETVAL(CreatureGossipBindings, GOSSIP_EVENT_ON_SELECT, pCreature->GetEntry(), false);
    auto originalMenu = *pPlayer->PlayerTalkClass;
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pCreature);
    Push(sender);
    Push(action);
    auto preventDefault = CallAllFunctionsBool(CreatureGossipBindings, key, true);
    if (!preventDefault) {
        *pPlayer->PlayerTalkClass = originalMenu;
    }
    return preventDefault;
}

bool ALE::OnGossipSelectCode(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* code)
{
    START_HOOK_WITH_RETVAL(CreatureGossipBindings, GOSSIP_EVENT_ON_SELECT, pCreature->GetEntry(), false);
    auto originalMenu = *pPlayer->PlayerTalkClass;
    pPlayer->PlayerTalkClass->ClearMenus();
    Push(pPlayer);
    Push(pCreature);
    Push(sender);
    Push(action);
    Push(code);
    auto preventDefault = CallAllFunctionsBool(CreatureGossipBindings, key, true);
    if (!preventDefault) {
        *pPlayer->PlayerTalkClass = originalMenu;
    }
    return preventDefault;
}
