/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"

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

namespace
{
    // Gossip codes are passed as nil when the option had no input box.
    sol::object CodeOrNil(sol::state_view lua, std::string const& code)
    {
        if (code.empty())
            return sol::make_object(lua, sol::nil);

        return sol::make_object(lua, code);
    }
}

bool ALE::OnGossipHello(Player* pPlayer, GameObject* pGameObject)
{
    START_HOOK_WITH_RETVAL(GameObjectGossipBindings, GOSSIP_EVENT_ON_HELLO, pGameObject->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return CallAllBool(*GameObjectGossipBindings, key, true, pPlayer, pGameObject);
}

bool ALE::OnGossipSelect(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action)
{
    START_HOOK_WITH_RETVAL(GameObjectGossipBindings, GOSSIP_EVENT_ON_SELECT, pGameObject->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return CallAllBool(*GameObjectGossipBindings, key, true, pPlayer, pGameObject, sender, action);
}

bool ALE::OnGossipSelectCode(Player* pPlayer, GameObject* pGameObject, uint32 sender, uint32 action, const char* code)
{
    START_HOOK_WITH_RETVAL(GameObjectGossipBindings, GOSSIP_EVENT_ON_SELECT, pGameObject->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return CallAllBool(*GameObjectGossipBindings, key, true, pPlayer, pGameObject, sender, action, code);
}

void ALE::HandleGossipSelectOption(Player* pPlayer, uint32 menuId, uint32 sender, uint32 action, const std::string& code)
{
    START_HOOK(PlayerGossipBindings, GOSSIP_EVENT_ON_SELECT, menuId);
    pPlayer->PlayerTalkClass->ClearMenus();

    // The player is passed twice: as the receiver and as the sender,
    // to keep the same argument layout as the other gossip events.
    CallAll(*PlayerGossipBindings, key, pPlayer, pPlayer, sender, action, CodeOrNil(lua, code));
}

bool ALE::OnItemGossip(Player* pPlayer, Item* pItem, SpellCastTargets const& /*targets*/)
{
    START_HOOK_WITH_RETVAL(ItemGossipBindings, GOSSIP_EVENT_ON_HELLO, pItem->GetEntry(), true);
    pPlayer->PlayerTalkClass->ClearMenus();
    return CallAllBool(*ItemGossipBindings, key, true, pPlayer, pItem);
}

void ALE::HandleGossipSelectOption(Player* pPlayer, Item* pItem, uint32 sender, uint32 action, const std::string& code)
{
    START_HOOK(ItemGossipBindings, GOSSIP_EVENT_ON_SELECT, pItem->GetEntry());
    pPlayer->PlayerTalkClass->ClearMenus();
    CallAll(*ItemGossipBindings, key, pPlayer, pItem, sender, action, CodeOrNil(lua, code));
}

bool ALE::OnGossipHello(Player* pPlayer, Creature* pCreature)
{
    START_HOOK_WITH_RETVAL(CreatureGossipBindings, GOSSIP_EVENT_ON_HELLO, pCreature->GetEntry(), false);
    pPlayer->PlayerTalkClass->ClearMenus();
    return CallAllBool(*CreatureGossipBindings, key, true, pPlayer, pCreature);
}

bool ALE::OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    START_HOOK_WITH_RETVAL(CreatureGossipBindings, GOSSIP_EVENT_ON_SELECT, pCreature->GetEntry(), false);

    // Restore the menu if no handler overrode the default behaviour.
    auto originalMenu = *pPlayer->PlayerTalkClass;
    pPlayer->PlayerTalkClass->ClearMenus();

    bool preventDefault = CallAllBool(*CreatureGossipBindings, key, true, pPlayer, pCreature, sender, action);
    if (!preventDefault)
        *pPlayer->PlayerTalkClass = originalMenu;

    return preventDefault;
}

bool ALE::OnGossipSelectCode(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action, const char* code)
{
    START_HOOK_WITH_RETVAL(CreatureGossipBindings, GOSSIP_EVENT_ON_SELECT, pCreature->GetEntry(), false);

    // Restore the menu if no handler overrode the default behaviour.
    auto originalMenu = *pPlayer->PlayerTalkClass;
    pPlayer->PlayerTalkClass->ClearMenus();

    bool preventDefault = CallAllBool(*CreatureGossipBindings, key, true, pPlayer, pCreature, sender, action, code);
    if (!preventDefault)
        *pPlayer->PlayerTalkClass = originalMenu;

    return preventDefault;
}
