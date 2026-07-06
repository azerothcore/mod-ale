/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEEventMgr.h"

using namespace Hooks;

#define START_HOOK(EVENT, ENTRY) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<GameObjectEvents>(EVENT, ENTRY);\
    if (!GameObjectEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, ENTRY, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EntryKey<GameObjectEvents>(EVENT, ENTRY);\
    if (!GameObjectEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

void ALE::OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, GameObject* pTarget)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DUMMY_EFFECT, pTarget->GetEntry());
    CallAll(*GameObjectEventBindings, key, pCaster, spellId, effIndex, pTarget);
}

void ALE::UpdateAI(GameObject* pGameObject, uint32 diff)
{
    pGameObject->ALEEvents->Update(diff);
    START_HOOK(GAMEOBJECT_EVENT_ON_AIUPDATE, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject, diff);
}

bool ALE::OnQuestAccept(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest)
{
    START_HOOK_WITH_RETVAL(GAMEOBJECT_EVENT_ON_QUEST_ACCEPT, pGameObject->GetEntry(), false);
    return CallAllBool(*GameObjectEventBindings, key, false, pPlayer, pGameObject, pQuest);
}

bool ALE::OnQuestReward(Player* pPlayer, GameObject* pGameObject, Quest const* pQuest, uint32 opt)
{
    START_HOOK_WITH_RETVAL(GAMEOBJECT_EVENT_ON_QUEST_REWARD, pGameObject->GetEntry(), false);
    return CallAllBool(*GameObjectEventBindings, key, false, pPlayer, pGameObject, pQuest, opt);
}

void ALE::GetDialogStatus(const Player* pPlayer, const GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DIALOG_STATUS, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pPlayer, pGameObject);
}

void ALE::OnDestroyed(GameObject* pGameObject, WorldObject* attacker)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DESTROYED, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject, attacker);
}

void ALE::OnDamaged(GameObject* pGameObject, WorldObject* attacker)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_DAMAGED, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject, attacker);
}

void ALE::OnLootStateChanged(GameObject* pGameObject, uint32 state)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_LOOT_STATE_CHANGE, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject, state);
}

void ALE::OnGameObjectStateChanged(GameObject* pGameObject, uint32 state)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_GO_STATE_CHANGED, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject, state);
}

void ALE::OnSpawn(GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_SPAWN, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject);
}

void ALE::OnAddToWorld(GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_ADD, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject);
}

void ALE::OnRemoveFromWorld(GameObject* pGameObject)
{
    START_HOOK(GAMEOBJECT_EVENT_ON_REMOVE, pGameObject->GetEntry());
    CallAll(*GameObjectEventBindings, key, pGameObject);
}

bool ALE::OnGameObjectUse(Player* pPlayer, GameObject* pGameObject)
{
    START_HOOK_WITH_RETVAL(GAMEOBJECT_EVENT_ON_USE, pGameObject->GetEntry(), false);
    return CallAllBool(*GameObjectEventBindings, key, false, pGameObject, pPlayer);
}
