/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEHandles.h"

#include "Corpse.h"
#include "GameObject.h"
#include "Group.h"
#include "GroupMgr.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Item.h"
#include "Map.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "Transport.h"

namespace ALEHandleEpoch
{
    namespace
    {
        // Starts above the default handle epoch (0) so a fresh handle never
        // accidentally matches the current call stack before its first resolve.
        uint64 currentEpoch = 1;
    }

    uint64 GetEpoch()
    {
        return currentEpoch;
    }

    void Bump()
    {
        ++currentEpoch;
    }
}

namespace
{
    // Captures where a world object lives so its handle can find the map again.
    void CaptureMap(WorldObject const* obj, uint32& mapId, uint32& instanceId)
    {
        if (Map const* map = obj->FindMap())
        {
            mapId = map->GetId();
            instanceId = map->GetInstanceId();
        }
    }
}

Object* ObjectRef::ResolveRaw() const
{
    if (!_guid)
        return nullptr;

    // Reuse the pointer resolved earlier in this same Lua call stack.
    if (_cached && _epoch == ALEHandleEpoch::GetEpoch())
        return _cached;

    Object* resolved = nullptr;

    // Players are reachable globally; items live in their owner's inventory;
    // everything else is looked up on the map it was captured on.
    if (_guid.IsPlayer())
        resolved = ObjectAccessor::FindPlayer(_guid);
    else if (_guid.IsItem())
    {
        if (Player* owner = ObjectAccessor::FindPlayer(_ownerGuid))
            resolved = owner->GetItemByGuid(_guid);
    }
    else if (Map* map = sMapMgr->FindMap(_mapId, _instanceId))
    {
        if (_guid.IsCreatureOrVehicle())
            resolved = map->GetCreature(_guid);
        else if (_guid.IsPet())
            resolved = map->GetPet(_guid);
        else if (_guid.IsGameObject() || _guid.IsTransport())
            resolved = map->GetGameObject(_guid);
        else if (_guid.IsMOTransport())
            resolved = map->GetTransport(_guid);
        else if (_guid.IsCorpse())
            resolved = map->GetCorpse(_guid);
    }

    if (resolved)
    {
        _cached = resolved;
        _epoch = ALEHandleEpoch::GetEpoch();
    }

    return resolved;
}

Object* ObjectRef::Resolve() const
{
    return ResolveRaw();
}

Object* ObjectRef::Require() const
{
    if (Object* obj = ResolveRaw())
        return obj;

    throw ALEStaleObjectError("Object");
}

WorldObjectRef::WorldObjectRef(WorldObject const* obj) :
    ObjectRef(obj->GetGUID(), 0, 0)
{
    CaptureMap(obj, _mapId, _instanceId);
}

WorldObject* WorldObjectRef::Resolve() const
{
    // Every guid type handled by ResolveRaw except items is a WorldObject.
    if (_guid.IsItem())
        return nullptr;

    return static_cast<WorldObject*>(ResolveRaw());
}

WorldObject* WorldObjectRef::Require() const
{
    if (WorldObject* obj = Resolve())
        return obj;

    throw ALEStaleObjectError("WorldObject");
}

UnitRef::UnitRef(Unit const* unit) :
    WorldObjectRef(unit)
{
}

Unit* UnitRef::Resolve() const
{
    if (Object* obj = ResolveRaw())
        return obj->ToUnit();

    return nullptr;
}

Unit* UnitRef::Require() const
{
    if (Unit* unit = Resolve())
        return unit;

    throw ALEStaleObjectError("Unit");
}

PlayerRef::PlayerRef(Player const* player) :
    UnitRef(player)
{
}

Player* PlayerRef::Resolve() const
{
    if (Object* obj = ResolveRaw())
        return obj->ToPlayer();

    return nullptr;
}

Player* PlayerRef::Require() const
{
    if (Player* player = Resolve())
        return player;

    throw ALEStaleObjectError("Player");
}

CreatureRef::CreatureRef(Creature const* creature) :
    UnitRef(creature)
{
}

Creature* CreatureRef::Resolve() const
{
    if (Object* obj = ResolveRaw())
        return obj->ToCreature();

    return nullptr;
}

Creature* CreatureRef::Require() const
{
    if (Creature* creature = Resolve())
        return creature;

    throw ALEStaleObjectError("Creature");
}

GameObjectRef::GameObjectRef(GameObject const* go) :
    WorldObjectRef(go)
{
}

GameObject* GameObjectRef::Resolve() const
{
    if (Object* obj = ResolveRaw())
        return obj->ToGameObject();

    return nullptr;
}

GameObject* GameObjectRef::Require() const
{
    if (GameObject* go = Resolve())
        return go;

    throw ALEStaleObjectError("GameObject");
}

TransportRef::TransportRef(Transport const* transport) :
    GameObjectRef(transport)
{
}

Transport* TransportRef::Resolve() const
{
    if (GameObject* go = GameObjectRef::Resolve())
        return go->ToTransport();

    return nullptr;
}

Transport* TransportRef::Require() const
{
    if (Transport* transport = Resolve())
        return transport;

    throw ALEStaleObjectError("Transport");
}

CorpseRef::CorpseRef(Corpse const* corpse) :
    WorldObjectRef(corpse)
{
}

Corpse* CorpseRef::Resolve() const
{
    if (Object* obj = ResolveRaw())
        return obj->ToCorpse();

    return nullptr;
}

Corpse* CorpseRef::Require() const
{
    if (Corpse* corpse = Resolve())
        return corpse;

    throw ALEStaleObjectError("Corpse");
}

ItemRef::ItemRef(Item const* item) :
    ObjectRef(item->GetGUID(), 0, 0, item->GetOwnerGUID())
{
}

Item* ItemRef::Resolve() const
{
    if (!_guid.IsItem())
        return nullptr;

    // The item resolution path in ResolveRaw only ever returns Item pointers.
    return static_cast<Item*>(ResolveRaw());
}

Item* ItemRef::Require() const
{
    if (Item* item = Resolve())
        return item;

    throw ALEStaleObjectError("Item");
}

MapRef::MapRef(Map const* map) :
    _mapId(map->GetId()), _instanceId(map->GetInstanceId())
{
}

Map* MapRef::Resolve() const
{
    return sMapMgr->FindMap(_mapId, _instanceId);
}

Map* MapRef::Require() const
{
    if (Map* map = Resolve())
        return map;

    throw ALEStaleObjectError("Map");
}

GroupRef::GroupRef(Group const* group) :
    _guid(group->GetGUID())
{
}

Group* GroupRef::Resolve() const
{
    return sGroupMgr->GetGroupByGUID(_guid.GetCounter());
}

Group* GroupRef::Require() const
{
    if (Group* group = Resolve())
        return group;

    throw ALEStaleObjectError("Group");
}

GuildRef::GuildRef(Guild const* guild) :
    _guildId(guild->GetId())
{
}

Guild* GuildRef::Resolve() const
{
    return sGuildMgr->GetGuildById(_guildId);
}

Guild* GuildRef::Require() const
{
    if (Guild* guild = Resolve())
        return guild;

    throw ALEStaleObjectError("Guild");
}
