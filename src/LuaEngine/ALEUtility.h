/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_UTIL_H
#define _ALE_UTIL_H

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <memory>
#include "Common.h"
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "Database/QueryResult.h"
#include "Log.h"

typedef QueryResult ALEQuery;

#define ALE_LOG_INFO(...)     LOG_INFO("ALE", __VA_ARGS__);
#define ALE_LOG_ERROR(...)    LOG_ERROR("ALE", __VA_ARGS__);
#define ALE_LOG_DEBUG(...)    LOG_DEBUG("ALE", __VA_ARGS__);

class Unit;
class WorldObject;
struct FactionTemplateEntry;

namespace ALEUtil
{
    uint32 GetCurrTime();

    uint32 GetTimeDiff(uint32 oldMSTime);

    class ObjectGUIDCheck
    {
    public:
        ObjectGUIDCheck(ObjectGuid guid);
        bool operator()(WorldObject* object);

        ObjectGuid _guid;
    };

    // Binary predicate to sort WorldObjects based on the distance to a reference WorldObject
    class ObjectDistanceOrderPred
    {
    public:
        ObjectDistanceOrderPred(WorldObject const* pRefObj, bool ascending = true);
        bool operator()(WorldObject const* pLeft, WorldObject const* pRight) const;

        WorldObject const* m_refObj;
        const bool m_ascending;
    };

    // Doesn't get self
    class WorldObjectInRangeCheck
    {
    public:
        WorldObjectInRangeCheck(bool nearest, WorldObject const* obj, float range,
            uint16 typeMask = 0, uint32 entry = 0, uint32 hostile = 0, uint32 dead = 0);
        WorldObject const& GetFocusObject() const;
        bool operator()(WorldObject* u);

        WorldObject const* const i_obj;
        Unit const* i_obj_unit;
        FactionTemplateEntry const* i_obj_fact;
        uint32 const i_hostile; // 0 both, 1 hostile, 2 friendly
        uint32 const i_entry;
        float i_range;
        uint16 const i_typeMask;
        uint32 const i_dead; // 0 both, 1 alive, 2 dead
        bool const i_nearest;
    };

    /*
     * Usage:
     * Inherit this class, then when needing lock, use
     * Guard guard(GetLock());
     *
     * The lock is automatically released at end of scope
     */
    class Lockable
    {
    public:
        typedef std::mutex LockType;
        typedef std::lock_guard<LockType> Guard;

        LockType& GetLock() { return _lock; }

    private:
        LockType _lock;
    };

    /*
     * Encodes `data` in Base-64 and store the result in `output`.
     */
    void EncodeData(const unsigned char* data, size_t input_length, std::string& output);

    /*
     * Decodes `data` from Base-64 and returns a pointer to the result, or `NULL` on error.
     *
     * The returned result buffer must be `delete[]`ed by the caller.
     */
    unsigned char* DecodeData(const char* data, size_t *output_length);
};

#endif
