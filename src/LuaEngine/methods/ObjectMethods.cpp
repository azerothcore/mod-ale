/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Object.h"
#include "UpdateFields.h"

/***
 * A basic game object (either an [Item] or a [WorldObject]).
 *
 * Objects in MaNGOS/Trinity are stored an a giant block of "values".
 * Subclasses of Object, like [WorldObject], extend the block with more data specific to that subclass.
 * Further subclasses, like [Player], extend it even further.
 *
 * A detailed map of all the fields in this data block can be found in the UpdateFields.h file of your emulator
 *   (it varies depending on the expansion supported).
 *
 * The GetValue methods in this class (e.g. [Object:GetInt32Value]) provide low-level access to the data block.
 * Other methods, like [Object:HasFlag] and [Object:GetScale], merely wrap the GetValue methods and provide a simpler interface.
 *
 * Inherits all methods from: none
 */
namespace LuaObject
{
    /**
     * Returns `true` if the specified flag is set, otherwise `false`.
     *
     * @param uint16 index : the index of the flags data in the [Object]
     * @param uint32 flag : the flag to check for in the flags data
     * @return bool hasFlag
     */
    bool HasFlag(Object* obj, uint16 index, uint32 flag)
    {
        return obj->HasFlag(index, flag);
    }

    /**
     * Returns `true` if the [Object] has been added to its [Map], otherwise `false`.
     *
     * @return bool inWorld
     */
    bool IsInWorld(Object* obj)
    {
        return obj->IsInWorld();
    }

    /**
     * Returns 'true' if the [Object] is a player, 'false' otherwise.
     *
     * @return bool IsPlayer
     */
    bool IsPlayer(Object* obj)
    {
        return obj->IsPlayer();
    }

    /**
     * Returns the data at the specified index, casted to a signed 32-bit integer.
     *
     * @param uint16 index
     * @return int32 value
     */
    int32 GetInt32Value(Object* obj, uint16 index)
    {
        return obj->GetInt32Value(index);
    }

    /**
     * Returns the data at the specified index, casted to a unsigned 32-bit integer.
     *
     * @param uint16 index
     * @return uint32 value
     */
    uint32 GetUInt32Value(Object* obj, uint16 index)
    {
        return obj->GetUInt32Value(index);
    }

    /**
     * Returns the data at the specified index, casted to a single-precision floating point value.
     *
     * @param uint16 index
     * @return float value
     */
    float GetFloatValue(Object* obj, uint16 index)
    {
        return obj->GetFloatValue(index);
    }

    /**
     * Returns the data at the specified index and offset, casted to an unsigned 8-bit integer.
     *
     * E.g. if you want the second byte at index 10, you would pass in 1 as the offset.
     *
     * @param uint16 index
     * @param uint8 offset : should be 0, 1, 2, or 3
     * @return uint8 value
     */
    uint8 GetByteValue(Object* obj, uint16 index, uint8 offset)
    {
        return obj->GetByteValue(index, offset);
    }

    /**
     * Returns the data at the specified index and offset, casted to a signed 16-bit integer.
     *
     * E.g. if you want the second half-word at index 10, you would pass in 1 as the offset.
     *
     * @param uint16 index
     * @param uint8 offset : should be 0 or 1
     * @return uint16 value
     */
    uint16 GetUInt16Value(Object* obj, uint16 index, uint8 offset)
    {
        return obj->GetUInt16Value(index, offset);
    }

    /**
     * Returns the scale/size of the [Object].
     *
     * This affects the size of a [WorldObject] in-game, but [Item]s don't have a "scale".
     *
     * @return float scale
     */
    float GetScale(Object* obj)
    {
        return obj->GetFloatValue(OBJECT_FIELD_SCALE_X);
    }

    /**
     * Returns the entry of the [Object].
     *
     * [Player]s do not have an "entry".
     *
     * @return uint32 entry
     */
    uint32 GetEntry(Object* obj)
    {
        return obj->GetEntry();
    }

    /**
     * Returns the GUID of the [Object].
     *
     * GUID is an unique identifier for the object.
     *
     * However on MaNGOS and cMangos creatures and gameobjects inside different maps can share
     * the same GUID but not on the same map.
     *
     * On TrinityCore this value is unique across all maps
     *
     * @return ObjectGuid guid
     */
    ObjectGuid GetGUID(Object* obj)
    {
        return obj->GetGUID();
    }

    /**
     * Returns the low-part of the [Object]'s GUID.
     *
     * On TrinityCore all low GUIDs are different for all objects of the same type.
     * For example creatures in instances are assigned new GUIDs when the Map is created.
     *
     * On MaNGOS and cMaNGOS low GUIDs are unique only on the same map.
     * For example creatures in instances use the same low GUID assigned for that spawn in the database.
     * This is why to identify a creature you have to know the instanceId and low GUID. See [Map:GetIntstanceId]
     *
     * @return uint32 guidLow
     */
    uint32 GetGUIDLow(Object* obj)
    {
        return obj->GetGUID().GetCounter();
    }

    /**
     * Returns the TypeId of the [Object].
     *
     *     enum TypeID
     *     {
     *         TYPEID_OBJECT        = 0,
     *         TYPEID_ITEM          = 1,
     *         TYPEID_CONTAINER     = 2,
     *         TYPEID_UNIT          = 3,
     *         TYPEID_PLAYER        = 4,
     *         TYPEID_GAMEOBJECT    = 5,
     *         TYPEID_DYNAMICOBJECT = 6,
     *         TYPEID_CORPSE        = 7
     *     };
     *
     * @return uint8 typeID
     */
    uint8 GetTypeId(Object* obj)
    {
        return obj->GetTypeId();
    }

    /**
     * Returns the data at the specified index, casted to an unsigned 64-bit integer.
     *
     * @param uint16 index
     * @return uint64 value
     */
    uint64 GetUInt64Value(Object* obj, uint16 index)
    {
        return obj->GetUInt64Value(index);
    }

    /**
     * Sets the specified flag in the data value at the specified index.
     *
     * If the flag was already set, it remains set.
     *
     * To remove a flag, use [Object:RemoveFlag].
     *
     * @param uint16 index
     * @param uint32 value
     */
    void SetFlag(Object* obj, uint16 index, uint32 flag)
    {
        obj->SetFlag(index, flag);
    }

    /**
     * Sets the data at the specified index to the given value, converted to a signed 32-bit integer.
     *
     * @param uint16 index
     * @param int32 value
     */
    void SetInt32Value(Object* obj, uint16 index, int32 value)
    {
        obj->SetInt32Value(index, value);
    }

    /**
     * Sets the data at the specified index to the given value, converted to an unsigned 32-bit integer.
     *
     * @param uint16 index
     * @param uint32 value
     */
    void SetUInt32Value(Object* obj, uint16 index, uint32 value)
    {
        obj->SetUInt32Value(index, value);
    }

    /**
     * Sets the data at the specified index to the given value, converted to an unsigned 32-bit integer.
     *
     * @param uint16 index
     * @param uint32 value
     */
    void UpdateUInt32Value(Object* obj, uint16 index, uint32 value)
    {
        obj->UpdateUInt32Value(index, value);
    }

    /**
     * Sets the data at the specified index to the given value, converted to a single-precision floating point value.
     *
     * @param uint16 index
     * @param float value
     */
    void SetFloatValue(Object* obj, uint16 index, float value)
    {
        obj->SetFloatValue(index, value);
    }

    /**
     * Sets the data at the specified index and offset to the given value, converted to an unsigned 8-bit integer.
     *
     * @param uint16 index
     * @param uint8 offset : should be 0, 1, 2, or 3
     * @param uint8 value
     */
    void SetByteValue(Object* obj, uint16 index, uint8 offset, uint8 value)
    {
        obj->SetByteValue(index, offset, value);
    }

    /**
     * Sets the data at the specified index to the given value, converted to an unsigned 16-bit integer.
     *
     * @param uint16 index
     * @param uint8 offset : should be 0 or 1
     * @param uint16 value
     */
    void SetUInt16Value(Object* obj, uint16 index, uint8 offset, uint16 value)
    {
        obj->SetUInt16Value(index, offset, value);
    }

    /**
     * Sets the data at the specified index to the given value, converted to a signed 16-bit integer.
     *
     * @param uint16 index
     * @param uint8 offset : should be 0 or 1
     * @param int16 value
     */
    void SetInt16Value(Object* obj, uint16 index, uint8 offset, int16 value)
    {
        obj->SetInt16Value(index, offset, value);
    }

    /**
     * Sets the [Object]'s scale/size to the given value.
     *
     * @param float scale
     */
    void SetScale(Object* obj, float scale)
    {
        obj->SetObjectScale(scale);
    }

    /**
     * Sets the data at the specified index to the given value, converted to an unsigned 64-bit integer.
     *
     * @param uint16 index
     * @param uint64 value
     */
    void SetUInt64Value(Object* obj, uint16 index, uint64 value)
    {
        obj->SetUInt64Value(index, value);
    }

    /**
     * Removes a flag from the value at the specified index.
     *
     * @param uint16 index
     * @param uint32 flag
     */
    void RemoveFlag(Object* obj, uint16 index, uint32 flag)
    {
        obj->RemoveFlag(index, flag);
    }

    /**
     * Attempts to convert the [Object] to a [Corpse].
     *
     * If the [Object] is not a [Corpse], returns `nil`.
     *
     * @return [Corpse] corpse : the [Object] as a [Corpse], or `nil`
     */
    Corpse* ToCorpse(Object* obj)
    {
        return obj->ToCorpse();
    }

    /**
     * Attempts to convert the [Object] to a [GameObject].
     *
     * If the [Object] is not a [GameObject], returns `nil`.
     *
     * @return [GameObject] gameObject : the [Object] as a [GameObject], or `nil`
     */
    GameObject* ToGameObject(Object* obj)
    {
        return obj->ToGameObject();
    }

    /**
     * Attempts to convert the [Object] to a [Unit].
     *
     * If the [Object] is not a [Unit], returns `nil`.
     *
     * @return [Unit] unit : the [Object] as a [Unit], or `nil`
     */
    Unit* ToUnit(Object* obj)
    {
        return obj->ToUnit();
    }

    /**
     * Attempts to convert the [Object] to a [Creature].
     *
     * If the [Object] is not a [Creature], returns `nil`.
     *
     * @return [Creature] creature : the [Object] as a [Creature], or `nil`
     */
    Creature* ToCreature(Object* obj)
    {
        return obj->ToCreature();
    }

    /**
     * Attempts to convert the [Object] to a [Player].
     *
     * If the [Object] is not a [Player], returns `nil`.
     *
     * @return [Player] player : the [Object] as a [Player], or `nil`
     */
    Player* ToPlayer(Object* obj)
    {
        return obj->ToPlayer();
    }
}

void RegisterObjectMethods(sol::state& lua)
{
    sol::usertype<ObjectRef> type = ALEBind::NewHandleType<ObjectRef>(lua, "Object");

    type["HasFlag"]           = ALEBind::Method(&LuaObject::HasFlag);
    type["IsInWorld"]         = ALEBind::Method(&LuaObject::IsInWorld);
    type["IsPlayer"]          = ALEBind::Method(&LuaObject::IsPlayer);
    type["GetInt32Value"]     = ALEBind::Method(&LuaObject::GetInt32Value);
    type["GetUInt32Value"]    = ALEBind::Method(&LuaObject::GetUInt32Value);
    type["GetFloatValue"]     = ALEBind::Method(&LuaObject::GetFloatValue);
    type["GetByteValue"]      = ALEBind::Method(&LuaObject::GetByteValue);
    type["GetUInt16Value"]    = ALEBind::Method(&LuaObject::GetUInt16Value);
    type["GetScale"]          = ALEBind::Method(&LuaObject::GetScale);
    type["GetEntry"]          = ALEBind::Method(&LuaObject::GetEntry);
    type["GetGUID"]           = ALEBind::Method(&LuaObject::GetGUID);
    type["GetGUIDLow"]        = ALEBind::Method(&LuaObject::GetGUIDLow);
    type["GetTypeId"]         = ALEBind::Method(&LuaObject::GetTypeId);
    type["GetUInt64Value"]    = ALEBind::Method(&LuaObject::GetUInt64Value);
    type["SetFlag"]           = ALEBind::Method(&LuaObject::SetFlag);
    type["SetInt32Value"]     = ALEBind::Method(&LuaObject::SetInt32Value);
    type["SetUInt32Value"]    = ALEBind::Method(&LuaObject::SetUInt32Value);
    type["UpdateUInt32Value"] = ALEBind::Method(&LuaObject::UpdateUInt32Value);
    type["SetFloatValue"]     = ALEBind::Method(&LuaObject::SetFloatValue);
    type["SetByteValue"]      = ALEBind::Method(&LuaObject::SetByteValue);
    type["SetUInt16Value"]    = ALEBind::Method(&LuaObject::SetUInt16Value);
    type["SetInt16Value"]     = ALEBind::Method(&LuaObject::SetInt16Value);
    type["SetScale"]          = ALEBind::Method(&LuaObject::SetScale);
    type["SetUInt64Value"]    = ALEBind::Method(&LuaObject::SetUInt64Value);
    type["RemoveFlag"]        = ALEBind::Method(&LuaObject::RemoveFlag);
    type["ToCorpse"]          = ALEBind::Method(&LuaObject::ToCorpse);
    type["ToGameObject"]      = ALEBind::Method(&LuaObject::ToGameObject);
    type["ToUnit"]            = ALEBind::Method(&LuaObject::ToUnit);
    type["ToCreature"]        = ALEBind::Method(&LuaObject::ToCreature);
    type["ToPlayer"]          = ALEBind::Method(&LuaObject::ToPlayer);
}
