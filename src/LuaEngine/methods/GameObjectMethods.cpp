/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Item.h"
#include "LootMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "SharedDefines.h"
#include "StringFormat.h"
#include "Unit.h"

/***
 * Represents a game object in the world, such as doors, chests, and other interactive objects.
 *
 * Inherits all methods from: [Object], [WorldObject]
 */
namespace LuaGameObject
{
    /**
     * Returns 'true' if the [GameObject] can give the specified [Quest]
     *
     * @param uint32 questId : quest entry Id to check
     * @return bool hasQuest
     */
    bool HasQuest(GameObject* go, uint32 questId)
    {
        return go->hasQuest(questId);
    }

    /**
     * Returns 'true' if the [GameObject] is spawned
     *
     * @return bool isSpawned
     */
    bool IsSpawned(GameObject* go)
    {
        return go->isSpawned();
    }

    /**
     * Returns 'true' if the [GameObject] is a transport
     *
     * @return bool isTransport
     */
    bool IsTransport(GameObject* go)
    {
        return go->IsTransport();
    }

    /**
     * Returns 'true' if the [GameObject] is active
     *
     * @return bool isActive
     */
    bool IsActive(GameObject* go)
    {
        return go->isActiveObject();
    }

    /*bool IsDestructible(GameObject* go) // TODO: Implementation core side
    {
        return go->IsDestructibleBuilding();
    }*/

    /**
     * Returns display ID of the [GameObject]
     *
     * @return uint32 displayId
     */
    uint32 GetDisplayId(GameObject* go)
    {
        return go->GetDisplayId();
    }

    /**
     * Returns the state of a [GameObject]
     * Below are client side [GOState]s off of 3.3.5a
     *
     * <pre>
     * enum GOState
     * {
     *     GO_STATE_ACTIVE             = 0,                        // show in world as used and not reset (closed door open)
     *     GO_STATE_READY              = 1,                        // show in world as ready (closed door close)
     *     GO_STATE_ACTIVE_ALTERNATIVE = 2                         // show in world as used in alt way and not reset (closed door open by cannon fire)
     * };
     * </pre>
     *
     * @return [GOState] goState
     */
    GOState GetGoState(GameObject* go)
    {
        return go->GetGoState();
    }

    /**
     * Returns the [LootState] of a [GameObject]
     * Below are [LootState]s off of 3.3.5a
     *
     * <pre>
     * enum LootState
     * {
     *     GO_NOT_READY = 0,
     *     GO_READY,                                               // can be ready but despawned, and then not possible activate until spawn
     *     GO_ACTIVATED,
     *     GO_JUST_DEACTIVATED
     * };
     * </pre>
     *
     * @return [LootState] lootState
     */
    LootState GetLootState(GameObject* go)
    {
        return go->getLootState();
    }

    /**
     * Returns the [Player] that can loot the [GameObject]
     *
     * Not the original looter and may be nil.
     *
     * @return [Player] player
     */
    Player* GetLootRecipient(GameObject* go)
    {
        return go->GetLootRecipient();
    }

    /**
     * Returns the [Group] that can loot the [GameObject]
     *
     * Not the original looter and may be nil.
     *
     * @return [Group] group
     */
    Group* GetLootRecipientGroup(GameObject* go)
    {
        return go->GetLootRecipientGroup();
    }

    /**
    * Returns the spawn ID for this [GameObject].
    *
    * @return uint32 spawnId
    */
    uint32 GetSpawnId(GameObject* go)
    {
        return go->GetSpawnId();
    }

    /**
     * Sets the state of a [GameObject]
     *
     * <pre>
     * enum GOState
     * {
     *     GO_STATE_ACTIVE             = 0,                        // show in world as used and not reset (closed door open)
     *     GO_STATE_READY              = 1,                        // show in world as ready (closed door close)
     *     GO_STATE_ACTIVE_ALTERNATIVE = 2                         // show in world as used in alt way and not reset (closed door open by cannon fire)
     * };
     * </pre>
     *
     * @param [GOState] state : all available go states can be seen above
     */
    void SetGoState(GameObject* go, sol::optional<uint32> stateArg)
    {
        uint32 state = stateArg.value_or(0);

        if (state == 0)
            go->SetGoState(GO_STATE_ACTIVE);
        else if (state == 1)
            go->SetGoState(GO_STATE_READY);
        else if (state == 2)
            go->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
    }

    /**
     * Sets the [LootState] of a [GameObject]
     * Below are [LootState]s off of 3.3.5a
     *
     * <pre>
     * enum LootState
     * {
     *     GO_NOT_READY = 0,
     *     GO_READY,                                               // can be ready but despawned, and then not possible activate until spawn
     *     GO_ACTIVATED,
     *     GO_JUST_DEACTIVATED
     * };
     * </pre>
     *
     * @param [LootState] state : all available loot states can be seen above
     */
    void SetLootState(GameObject* go, sol::optional<uint32> stateArg)
    {
        uint32 state = stateArg.value_or(0);

        if (state == 0)
            go->SetLootState(GO_NOT_READY);
        else if (state == 1)
            go->SetLootState(GO_READY);
        else if (state == 2)
            go->SetLootState(GO_ACTIVATED);
        else if (state == 3)
            go->SetLootState(GO_JUST_DEACTIVATED);
    }

    /**
    * Adds an [Item] to the loot of a [GameObject]
    * Requires an gameobject with loot_template set to 0.
    *
    * @param uint32 entry : The entry of the [Item]
    * @param uint32 amount = 1 : amount of the [Item] to add to the loot
    * @return uint32 itemGUIDlow : low GUID of the [Item]
    */
    sol::variadic_results AddLoot(GameObject* go, sol::variadic_args args, sol::this_state s)
    {
        sol::state_view lua(s);
        sol::variadic_results results;

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        for (std::size_t i = 0; i + 1 < args.size(); i += 2)
        {
            uint32 entry = args[i].as<uint32>();
            uint32 amount = args[i + 1].as<uint32>();

            ItemTemplate const* item_proto = sObjectMgr->GetItemTemplate(entry);
            if (!item_proto)
                throw std::runtime_error(Acore::StringFormat("Item entry {} does not exist", entry));

            if (amount < 1 || (item_proto->MaxCount > 0 && amount > uint32(item_proto->MaxCount)))
                throw std::runtime_error(Acore::StringFormat("Item entry {} has invalid amount {}", entry, amount));

            if (Item* item = Item::CreateItem(entry, amount))
            {
                item->SaveToDB(trans);
                LootStoreItem storeItem(item->GetEntry(), 0, 100, 0, LOOT_MODE_DEFAULT, 0, item->GetCount(), item->GetCount());
                go->loot.AddItem(storeItem);
                results.push_back(sol::make_object(lua, item->GetGUID().GetCounter()));
            }
        }

        CharacterDatabase.CommitTransaction(trans);

        return results;
    }

    /**
     * Saves [GameObject] to the database
     *
     */
    void SaveToDB(GameObject* go)
    {
        go->SaveToDB();
    }

    /**
     * Removes [GameObject] from the world
     *
     * The object is no longer reachable after this and it is not respawned.
     *
     * @param bool deleteFromDB : if true, it will delete the [GameObject] from the database
     */
    void RemoveFromWorld(GameObject* go, sol::optional<bool> deleteFromDB)
    {
        bool deldb = deleteFromDB.value_or(false);

        // cs_gobject.cpp copy paste
        ObjectGuid ownerGuid = go->GetOwnerGUID();
        if (ownerGuid)
        {
            Unit* owner = ObjectAccessor::GetUnit(*go, ownerGuid);
            if (!owner || !ownerGuid.IsPlayer())
                return;

            owner->RemoveGameObject(go, false);
        }

        if (deldb)
            go->DeleteFromDB();

        go->SetRespawnTime(0);
        go->Delete();
    }

    /**
     * Activates a door or a button/lever
     *
     * @param uint32 delay = 0 : cooldown time in seconds to restore the [GameObject] back to normal. 0 for infinite duration
     */
    void UseDoorOrButton(GameObject* go, sol::optional<uint32> delay)
    {
        go->UseDoorOrButton(delay.value_or(0));
    }

    /**
     * Despawns a [GameObject]
     *
     * The gameobject may be automatically respawned by the core
     */
    void Despawn(GameObject* go)
    {
        go->SetLootState(GO_JUST_DEACTIVATED);
    }

    /**
     * Respawns a [GameObject]
     */
    void Respawn(GameObject* go)
    {
        go->Respawn();
    }

    /**
     * Sets the respawn or despawn time for the gameobject.
     *
     * Respawn time is also used as despawn time depending on gameobject settings
     *
     * @param int32 delay = 0 : cooldown time in seconds to respawn or despawn the object. 0 means never
     */
    void SetRespawnTime(GameObject* go, int32 respawn)
    {
        go->SetRespawnTime(respawn);
    }

    /**
     * Sets the respawn or despawn time for the gameobject.
     *
     * Respawn time is also used as despawn time depending on gameobject settings
     *
     * @param int32 delay = 0 : cooldown time in seconds to respawn or despawn the object. 0 means never
     */
    void SetRespawnDelay(GameObject* go, int32 respawn)
    {
        go->SetRespawnDelay(respawn);
    }
}

void RegisterGameObjectMethods(sol::state& lua)
{
    sol::usertype<GameObjectRef> type = ALEBind::NewHandleType<GameObjectRef, WorldObjectRef, ObjectRef>(lua, "GameObject");

    type["HasQuest"]              = ALEBind::Method(&LuaGameObject::HasQuest);
    type["IsSpawned"]             = ALEBind::Method(&LuaGameObject::IsSpawned);
    type["IsTransport"]           = ALEBind::Method(&LuaGameObject::IsTransport);
    type["IsActive"]              = ALEBind::Method(&LuaGameObject::IsActive);
    type["GetDisplayId"]          = ALEBind::Method(&LuaGameObject::GetDisplayId);
    type["GetGoState"]            = ALEBind::Method(&LuaGameObject::GetGoState);
    type["GetLootState"]          = ALEBind::Method(&LuaGameObject::GetLootState);
    type["GetLootRecipient"]      = ALEBind::Method(&LuaGameObject::GetLootRecipient);
    type["GetLootRecipientGroup"] = ALEBind::Method(&LuaGameObject::GetLootRecipientGroup);
    type["GetSpawnId"]            = ALEBind::Method(&LuaGameObject::GetSpawnId);
    type["SetGoState"]            = ALEBind::Method(&LuaGameObject::SetGoState);
    type["SetLootState"]          = ALEBind::Method(&LuaGameObject::SetLootState);
    type["AddLoot"]               = ALEBind::Method(&LuaGameObject::AddLoot);
    type["SaveToDB"]              = ALEBind::Method(&LuaGameObject::SaveToDB);
    type["RemoveFromWorld"]       = ALEBind::Method(&LuaGameObject::RemoveFromWorld);
    type["UseDoorOrButton"]       = ALEBind::Method(&LuaGameObject::UseDoorOrButton);
    type["Despawn"]               = ALEBind::Method(&LuaGameObject::Despawn);
    type["Respawn"]               = ALEBind::Method(&LuaGameObject::Respawn);
    type["SetRespawnTime"]        = ALEBind::Method(&LuaGameObject::SetRespawnTime);
    type["SetRespawnDelay"]       = ALEBind::Method(&LuaGameObject::SetRespawnDelay);
}
