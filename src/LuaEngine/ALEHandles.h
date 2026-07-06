/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_HANDLES_H
#define _ALE_HANDLES_H

#include "ObjectGuid.h"

#include <stdexcept>
#include <string>

class Corpse;
class GameObject;
class Item;
class Object;
class Player;
class Unit;
class WorldObject;
class Creature;

/*
 * Safe references ("handles") to game objects exposed to Lua.
 *
 * Lua scripts can keep a value alive for as long as they want, but the game
 * object behind it can despawn, log out or be unloaded at any moment. Handing
 * raw pointers to Lua would therefore be a use-after-free waiting to happen.
 *
 * Instead, every world object crossing the C++/Lua boundary is wrapped in a
 * handle that stores its ObjectGuid (plus enough context to find it again) and
 * is re-resolved through the core accessors on every use:
 *
 *   - resolving is cheap: the resolved pointer is cached for the duration of
 *     the current Lua call stack (see ALEHandleEpoch below), so repeated method
 *     calls inside one event handler cost a single lookup;
 *   - a handle kept across calls stays *safe*: if the object is gone, methods
 *     raise a regular Lua error ("object no longer exists") instead of
 *     dereferencing freed memory.
 *
 * The handle classes mirror the game class hierarchy (PlayerRef is a UnitRef
 * is a WorldObjectRef is an ObjectRef) so that sol can pass a Player value to
 * any function expecting a Unit or WorldObject.
 */

namespace ALEHandleEpoch
{
    /*
     * Identifier of the current Lua call stack.
     *
     * The engine bumps it whenever the outermost event handler returns to C++.
     * Handles use it to know whether their cached pointer belongs to the
     * current call stack (safe to reuse) or to a previous one (must re-resolve).
     */
    uint64 GetEpoch();
    void Bump();
}

/*
 * Error thrown by Require() when a handle no longer resolves to a live object.
 * sol converts it into a regular Lua error, catchable with pcall.
 */
class ALEStaleObjectError : public std::runtime_error
{
public:
    explicit ALEStaleObjectError(char const* typeName) :
        std::runtime_error(std::string(typeName) + " no longer exists (despawned, logged out or unloaded)")
    {
    }
};

/*
 * Base handle: identity and resolution logic shared by every handle type.
 *
 * Resolution dispatches on the guid's HighGuid, so a UnitRef holding a player
 * guid correctly resolves through ObjectAccessor while a creature guid goes
 * through its owning map.
 */
class ObjectRef
{
public:
    ObjectRef() = default;

    ObjectGuid GetGuid() const { return _guid; }

    // Returns whether the referenced object is currently reachable.
    bool IsValid() const { return ResolveRaw() != nullptr; }

    // Handles compare by identity: same guid means same game object.
    bool operator==(ObjectRef const& other) const { return _guid == other._guid; }

    Object* Resolve() const;
    Object* Require() const;

protected:
    ObjectRef(ObjectGuid guid, uint32 mapId, uint32 instanceId, ObjectGuid ownerGuid = ObjectGuid::Empty) :
        _guid(guid), _ownerGuid(ownerGuid), _mapId(mapId), _instanceId(instanceId)
    {
    }

    // Looks the object up through the appropriate core accessor,
    // reusing the pointer cached for the current Lua call stack when possible.
    Object* ResolveRaw() const;

    ObjectGuid _guid;
    // Owning player's guid, only used to resolve items (an item lives in its owner's inventory).
    ObjectGuid _ownerGuid;
    uint32 _mapId = 0;
    uint32 _instanceId = 0;

private:
    // Pointer cache, only trusted while _epoch matches the current call stack.
    mutable Object* _cached = nullptr;
    mutable uint64 _epoch = 0;
};

class WorldObjectRef : public ObjectRef
{
public:
    WorldObjectRef() = default;
    explicit WorldObjectRef(WorldObject const* obj);

    WorldObject* Resolve() const;
    WorldObject* Require() const;

protected:
    using ObjectRef::ObjectRef;
};

class UnitRef : public WorldObjectRef
{
public:
    UnitRef() = default;
    explicit UnitRef(Unit const* unit);

    Unit* Resolve() const;
    Unit* Require() const;

protected:
    using WorldObjectRef::WorldObjectRef;
};

class PlayerRef : public UnitRef
{
public:
    PlayerRef() = default;
    explicit PlayerRef(Player const* player);

    Player* Resolve() const;
    Player* Require() const;
};

class CreatureRef : public UnitRef
{
public:
    CreatureRef() = default;
    explicit CreatureRef(Creature const* creature);

    Creature* Resolve() const;
    Creature* Require() const;
};

class GameObjectRef : public WorldObjectRef
{
public:
    GameObjectRef() = default;
    explicit GameObjectRef(GameObject const* go);

    GameObject* Resolve() const;
    GameObject* Require() const;
};

class CorpseRef : public WorldObjectRef
{
public:
    CorpseRef() = default;
    explicit CorpseRef(Corpse const* corpse);

    Corpse* Resolve() const;
    Corpse* Require() const;
};

class ItemRef : public ObjectRef
{
public:
    ItemRef() = default;
    explicit ItemRef(Item const* item);

    Item* Resolve() const;
    Item* Require() const;
};

#endif // _ALE_HANDLES_H
