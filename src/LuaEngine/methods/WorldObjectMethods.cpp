/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"
#include "ALEEventMgr.h"
#include "ALEUtility.h"

#include "CellImpl.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Object.h"
#include "Opcodes.h"
#include "Player.h"
#include "TemporarySummon.h"
#include "WorldPacket.h"

/***
 * Represents a [WorldObject] in the game world.
 *
 * Inherits all methods from: [Object]
 */
namespace LuaWorldObject
{
    /**
     * Returns the name of the [WorldObject]
     *
     * @return string name
     */
    std::string GetName(WorldObject* worldobject)
    {
        return worldobject->GetName();
    }

    /**
     * Returns the current [Map] object of the [WorldObject]
     *
     * @return [Map] mapObject
     */
    Map* GetMap(WorldObject* worldobject)
    {
        return worldobject->GetMap();
    }

    /**
     * Returns the current phase of the [WorldObject]
     *
     * @return uint32 phase
     */
    uint32 GetPhaseMask(WorldObject* worldobject)
    {
        return worldobject->GetPhaseMask();
    }

    /**
    * Sets the [WorldObject]'s phase mask.
    *
    * @param uint32 phaseMask
    * @param bool update = true : update visibility to nearby objects
    */
    void SetPhaseMask(WorldObject* worldobject, uint32 phaseMask, sol::optional<bool> updateArg)
    {
        bool update = updateArg.value_or(true);
        worldobject->SetPhaseMask(phaseMask, update);
    }

    /**
     * Returns the current instance ID of the [WorldObject]
     *
     * @return uint32 instanceId
     */
    uint32 GetInstanceId(WorldObject* worldobject)
    {
        return worldobject->GetInstanceId();
    }

    /**
     * Returns the current area ID of the [WorldObject]
     *
     * @return uint32 areaId
     */
    uint32 GetAreaId(WorldObject* worldobject)
    {
        return worldobject->GetAreaId();
    }

    /**
     * Returns the current zone ID of the [WorldObject]
     *
     * @return uint32 zoneId
     */
    uint32 GetZoneId(WorldObject* worldobject)
    {
        return worldobject->GetZoneId();
    }

    /**
     * Returns the current map ID of the [WorldObject]
     *
     * @return uint32 mapId
     */
    uint32 GetMapId(WorldObject* worldobject)
    {
        return worldobject->GetMapId();
    }

    /**
     * Returns the current X coordinate of the [WorldObject]
     *
     * @return float x
     */
    float GetX(WorldObject* worldobject)
    {
        return worldobject->GetPositionX();
    }

    /**
     * Returns the current Y coordinate of the [WorldObject]
     *
     * @return float y
     */
    float GetY(WorldObject* worldobject)
    {
        return worldobject->GetPositionY();
    }

    /**
     * Returns the current Z coordinate of the [WorldObject]
     *
     * @return float z
     */
    float GetZ(WorldObject* worldobject)
    {
        return worldobject->GetPositionZ();
    }

    /**
     * Returns the current orientation of the [WorldObject]
     *
     * @return float orientation / facing
     */
    float GetO(WorldObject* worldobject)
    {
        return worldobject->GetOrientation();
    }

    /**
     * Returns the coordinates and orientation of the [WorldObject]
     *
     * @return float x : x coordinate of the [WorldObject]
     * @return float y : y coordinate of the [WorldObject]
     * @return float z : z coordinate (height) of the [WorldObject]
     * @return float o : facing / orientation of  the [WorldObject]
     */
    std::tuple<float, float, float, float> GetLocation(WorldObject* worldobject)
    {
        return std::tuple<float, float, float, float>(worldobject->GetPositionX(), worldobject->GetPositionY(), worldobject->GetPositionZ(), worldobject->GetOrientation());
    }

    /**
     * Returns the nearest [Player] object in sight of the [WorldObject] or within the given range
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return [Player] nearestPlayer
     */
    Unit* GetNearestPlayer(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint32> hostileArg, sol::optional<uint32> deadArg)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint32 hostile = hostileArg.value_or(0);
        uint32 dead = deadArg.value_or(1);

        Unit* target = nullptr;
        ALEUtil::WorldObjectInRangeCheck checker(true, worldobject, range, TYPEMASK_PLAYER, 0, hostile, dead);

        Acore::UnitLastSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, target, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        return target;
    }

    /**
     * Returns the nearest [GameObject] object in sight of the [WorldObject] or within the given range and/or with a specific entry ID
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 entryId = 0 : optionally set entry ID of game object to find
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     *
     * @return [GameObject] nearestGameObject
     */
    GameObject* GetNearestGameObject(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint32> entryArg, sol::optional<uint32> hostileArg)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint32 entry = entryArg.value_or(0);
        uint32 hostile = hostileArg.value_or(0);

        GameObject* target = nullptr;
        ALEUtil::WorldObjectInRangeCheck checker(true, worldobject, range, TYPEMASK_GAMEOBJECT, entry, hostile);

        Acore::GameObjectLastSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, target, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        return target;
    }

    /**
     * Returns the nearest [Creature] object in sight of the [WorldObject] or within the given range and/or with a specific entry ID
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 entryId = 0 : optionally set entry ID of creature to find
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return [Creature] nearestCreature
     */
    Creature* GetNearestCreature(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint32> entryArg, sol::optional<uint32> hostileArg, sol::optional<uint32> deadArg)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint32 entry = entryArg.value_or(0);
        uint32 hostile = hostileArg.value_or(0);
        uint32 dead = deadArg.value_or(1);

        Creature* target = nullptr;
        ALEUtil::WorldObjectInRangeCheck checker(true, worldobject, range, TYPEMASK_UNIT, entry, hostile, dead);

        Acore::CreatureLastSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, target, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        return target;
    }

    /**
     * Returns a table of [Player] objects in sight of the [WorldObject] or within the given range
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return table playersInRange : table of [Player]s
     */
    sol::table GetPlayersInRange(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint32> hostileArg, sol::optional<uint32> deadArg, sol::this_state s)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint32 hostile = hostileArg.value_or(0);
        uint32 dead = deadArg.value_or(1);

        std::list<Player*> list;
        ALEUtil::WorldObjectInRangeCheck checker(false, worldobject, range, TYPEMASK_PLAYER, 0, hostile, dead);

        Acore::PlayerListSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, list, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (std::list<Player*>::const_iterator it = list.begin(); it != list.end(); ++it)
            tbl[++i] = PlayerRef(*it);

        return tbl;
    }

    /**
     * Returns a table of [Creature] objects in sight of the [WorldObject] or within the given range and/or with a specific entry ID
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 entryId = 0 : optionally set entry ID of creatures to find
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return table creaturesInRange : table of [Creature]s
     */
    sol::table GetCreaturesInRange(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint32> entryArg, sol::optional<uint32> hostileArg, sol::optional<uint32> deadArg, sol::this_state s)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint32 entry = entryArg.value_or(0);
        uint32 hostile = hostileArg.value_or(0);
        uint32 dead = deadArg.value_or(1);

        std::list<Creature*> list;
        ALEUtil::WorldObjectInRangeCheck checker(false, worldobject, range, TYPEMASK_UNIT, entry, hostile, dead);

        Acore::CreatureListSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, list, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (std::list<Creature*>::const_iterator it = list.begin(); it != list.end(); ++it)
            tbl[++i] = CreatureRef(*it);

        return tbl;
    }

    /**
     * Returns a table of [GameObject] objects in sight of the [WorldObject] or within the given range and/or with a specific entry ID
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param uint32 entryId = 0 : optionally set entry ID of game objects to find
     * @param uint32 hostile = 0 : 0 both, 1 hostile, 2 friendly
     *
     * @return table gameObjectsInRange : table of [GameObject]s
     */
    sol::table GetGameObjectsInRange(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint32> entryArg, sol::optional<uint32> hostileArg, sol::this_state s)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint32 entry = entryArg.value_or(0);
        uint32 hostile = hostileArg.value_or(0);

        std::list<GameObject*> list;
        ALEUtil::WorldObjectInRangeCheck checker(false, worldobject, range, TYPEMASK_GAMEOBJECT, entry, hostile);

        Acore::GameObjectListSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, list, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        sol::table tbl = sol::state_view(s).create_table();
        uint32 i = 0;

        for (std::list<GameObject*>::const_iterator it = list.begin(); it != list.end(); ++it)
            tbl[++i] = GameObjectRef(*it);

        return tbl;
    }

    /**
     * Returns nearest [WorldObject] in sight of the [WorldObject].
     * The distance, type, entry and hostility requirements the [WorldObject] must match can be passed.
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param [TypeMask] type = 0 : the [TypeMask] that the [WorldObject] must be. This can contain multiple types. 0 will be ingored
     * @param uint32 entry = 0 : the entry of the [WorldObject], 0 will be ingored
     * @param uint32 hostile = 0 : specifies whether the [WorldObject] needs to be 1 hostile, 2 friendly or 0 either
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return [WorldObject] worldObject
     */
    WorldObject* GetNearObject(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint16> typeArg, sol::optional<uint32> entryArg, sol::optional<uint32> hostileArg, sol::optional<uint32> deadArg)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint16 type = typeArg.value_or(0); // TypeMask
        uint32 entry = entryArg.value_or(0);
        uint32 hostile = hostileArg.value_or(0); // 0 none, 1 hostile, 2 friendly
        uint32 dead = deadArg.value_or(1); // 0 both, 1 alive, 2 dead

        float x, y, z;
        worldobject->GetPosition(x, y, z);
        ALEUtil::WorldObjectInRangeCheck checker(true, worldobject, range, type, entry, hostile, dead);

        WorldObject* target = nullptr;

        Acore::WorldObjectLastSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, target, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        return target;
    }

    /**
     * Returns a table of [WorldObject]s in sight of the [WorldObject].
     * The distance, type, entry and hostility requirements the [WorldObject] must match can be passed.
     *
     * @param float range = 533.33333 : optionally set range. Default range is grid size
     * @param [TypeMask] type = 0 : the [TypeMask] that the [WorldObject] must be. This can contain multiple types. 0 will be ingored
     * @param uint32 entry = 0 : the entry of the [WorldObject], 0 will be ingored
     * @param uint32 hostile = 0 : specifies whether the [WorldObject] needs to be 1 hostile, 2 friendly or 0 either
     * @param uint32 dead = 1 : 0 both, 1 alive, 2 dead
     *
     * @return table worldObjectList : table of [WorldObject]s
     */
    sol::table GetNearObjects(WorldObject* worldobject, sol::optional<float> rangeArg, sol::optional<uint16> typeArg, sol::optional<uint32> entryArg, sol::optional<uint32> hostileArg, sol::optional<uint32> deadArg, sol::this_state s)
    {
        float range = rangeArg.value_or(SIZE_OF_GRIDS);
        uint16 type = typeArg.value_or(0); // TypeMask
        uint32 entry = entryArg.value_or(0);
        uint32 hostile = hostileArg.value_or(0); // 0 none, 1 hostile, 2 friendly
        uint32 dead = deadArg.value_or(1); // 0 both, 1 alive, 2 dead

        float x, y, z;
        worldobject->GetPosition(x, y, z);
        ALEUtil::WorldObjectInRangeCheck checker(false, worldobject, range, type, entry, hostile, dead);

        std::list<WorldObject*> list;

        Acore::WorldObjectListSearcher<ALEUtil::WorldObjectInRangeCheck> searcher(worldobject, list, checker);
        Cell::VisitObjects(worldobject, searcher, range);

        sol::state_view lua(s);
        sol::table tbl = lua.create_table();
        uint32 i = 0;

        for (std::list<WorldObject*>::const_iterator it = list.begin(); it != list.end(); ++it)
            tbl[++i] = ALEBind::ToLuaDynamic(lua, *it);

        return tbl;
    }

    /**
     * Returns the distance from this [WorldObject] to another [WorldObject], or from this [WorldObject] to a point in 3d space.
     *
     * The function takes into account the given object sizes. See also [WorldObject:GetExactDistance], [WorldObject:GetDistance2d]
     *
     * @proto dist = (obj)
     * @proto dist = (x, y, z)
     *
     * @param [WorldObject] obj
     * @param float x : the X-coordinate of the point
     * @param float y : the Y-coordinate of the point
     * @param float z : the Z-coordinate of the point
     *
     * @return float dist : the distance in yards
     */
    float GetDistance(WorldObject* worldobject, sol::object first, sol::optional<float> yArg, sol::optional<float> zArg)
    {
        if (first.is<WorldObjectRef>())
            if (WorldObject* target = first.as<WorldObjectRef>().Resolve())
                return worldobject->GetDistance(target);

        if (!first.is<float>() || !yArg || !zArg)
            throw std::invalid_argument("WorldObject or coordinates (x, y, z) expected");

        float X = first.as<float>();
        float Y = *yArg;
        float Z = *zArg;
        return worldobject->GetDistance(X, Y, Z);
    }

    /**
     * Returns the distance from this [WorldObject] to another [WorldObject], or from this [WorldObject] to a point in 3d space.
     *
     * The function does not take into account the given object sizes, which means only the object coordinates are compared. See also [WorldObject:GetDistance], [WorldObject:GetDistance2d]
     *
     * @proto dist = (obj)
     * @proto dist = (x, y, z)
     *
     * @param [WorldObject] obj
     * @param float x : the X-coordinate of the point
     * @param float y : the Y-coordinate of the point
     * @param float z : the Z-coordinate of the point
     *
     * @return float dist : the distance in yards
     */
    float GetExactDistance(WorldObject* worldobject, sol::object first, sol::optional<float> yArg, sol::optional<float> zArg)
    {
        float x, y, z;
        worldobject->GetPosition(x, y, z);

        WorldObject* target = first.is<WorldObjectRef>() ? first.as<WorldObjectRef>().Resolve() : nullptr;
        if (target)
        {
            float x2, y2, z2;
            target->GetPosition(x2, y2, z2);
            x -= x2;
            y -= y2;
            z -= z2;
        }
        else
        {
            if (!first.is<float>() || !yArg || !zArg)
                throw std::invalid_argument("WorldObject or coordinates (x, y, z) expected");

            x -= first.as<float>();
            y -= *yArg;
            z -= *zArg;
        }

        return std::sqrt(x*x + y*y + z*z);
    }

    /**
     * Returns the distance from this [WorldObject] to another [WorldObject], or from this [WorldObject] to a point in 2d space.
     *
     * The function takes into account the given object sizes. See also [WorldObject:GetDistance], [WorldObject:GetExactDistance2d]
     *
     * @proto dist = (obj)
     * @proto dist = (x, y)
     *
     * @param [WorldObject] obj
     * @param float x : the X-coordinate of the point
     * @param float y : the Y-coordinate of the point
     *
     * @return float dist : the distance in yards
     */
    float GetDistance2d(WorldObject* worldobject, sol::object first, sol::optional<float> yArg)
    {
        if (first.is<WorldObjectRef>())
            if (WorldObject* target = first.as<WorldObjectRef>().Resolve())
                return worldobject->GetDistance2d(target);

        if (!first.is<float>() || !yArg)
            throw std::invalid_argument("WorldObject or coordinates (x, y) expected");

        float X = first.as<float>();
        float Y = *yArg;
        return worldobject->GetDistance2d(X, Y);
    }

    /**
     * Returns the distance from this [WorldObject] to another [WorldObject], or from this [WorldObject] to a point in 2d space.
     *
     * The function does not take into account the given object sizes, which means only the object coordinates are compared. See also [WorldObject:GetDistance], [WorldObject:GetDistance2d]
     *
     * @proto dist = (obj)
     * @proto dist = (x, y)
     *
     * @param [WorldObject] obj
     * @param float x : the X-coordinate of the point
     * @param float y : the Y-coordinate of the point
     *
     * @return float dist : the distance in yards
     */
    float GetExactDistance2d(WorldObject* worldobject, sol::object first, sol::optional<float> yArg)
    {
        float x, y, z;
        worldobject->GetPosition(x, y, z);

        WorldObject* target = first.is<WorldObjectRef>() ? first.as<WorldObjectRef>().Resolve() : nullptr;
        if (target)
        {
            float x2, y2, z2;
            target->GetPosition(x2, y2, z2);
            x -= x2;
            y -= y2;
        }
        else
        {
            if (!first.is<float>() || !yArg)
                throw std::invalid_argument("WorldObject or coordinates (x, y) expected");

            x -= first.as<float>();
            y -= *yArg;
        }

        return std::sqrt(x*x + y*y);
    }

    /**
     * Returns the x, y and z of a point dist away from the [WorldObject].
     *
     * @param float distance : specifies the distance of the point from the [WorldObject] in yards
     * @param float angle : specifies the angle of the point relative to the orientation / facing of the [WorldObject] in radians
     *
     * @return float x
     * @return float y
     * @return float z
     */
    std::tuple<float, float, float> GetRelativePoint(WorldObject* worldobject, float dist, float rad)
    {
        float x, y, z;
        worldobject->GetClosePoint(x, y, z, 0.0f, dist, rad);

        return std::tuple<float, float, float>(x, y, z);
    }

    /**
     * Returns the angle between this [WorldObject] and another [WorldObject] or a point.
     *
     * The angle is the angle between two points and orientation will be ignored.
     *
     * @proto dist = (obj)
     * @proto dist = (x, y)
     *
     * @param [WorldObject] object
     * @param float x
     * @param float y
     *
     * @return float angle : angle in radians in range 0..2*pi
     */
    float GetAngle(WorldObject* worldobject, sol::object first, sol::optional<float> yArg)
    {
        if (first.is<WorldObjectRef>())
            if (WorldObject* target = first.as<WorldObjectRef>().Resolve())
                return worldobject->GetAbsoluteAngle(target);

        if (!first.is<float>() || !yArg)
            throw std::invalid_argument("WorldObject or coordinates (x, y) expected");

        float x = first.as<float>();
        float y = *yArg;
        return worldobject->GetAbsoluteAngle(x, y);
    }

    /**
     * Returns the transport the [WorldObject] is on, or nil if not on a transport
     *
     * @return [Transport] transport
     */
    Transport* GetTransport(WorldObject* worldobject)
    {
        return static_cast<Transport*>(worldobject->GetTransport());
    }

    /**
     * Sends a [WorldPacket] to [Player]s in sight of the [WorldObject].
     *
     * @param [WorldPacket] packet
     */
    void SendPacket(WorldObject* worldobject, WorldPacket* data)
    {
        worldobject->SendMessageToSet(data, true);
    }

    /**
     * Spawns a [GameObject] at specified location.
     *
     * @param uint32 entry : [GameObject] entry ID
     * @param float x
     * @param float y
     * @param float z
     * @param float o
     * @param uint32 respawnDelay = 30 : respawn time in seconds
     * @return [GameObject] gameObject
     */
    GameObject* SummonGameObject(WorldObject* worldobject, uint32 entry, float x, float y, float z, float o, sol::optional<uint32> respawnDelayArg)
    {
        uint32 respawnDelay = respawnDelayArg.value_or(30);

        return worldobject->SummonGameObject(entry, x, y, z, o, 0, 0, 0, 0, respawnDelay);
    }

    /**
     * Spawns the creature at specified location.
     *
     *     enum TempSummonType
     *     {
     *         TEMPSUMMON_TIMED_OR_DEAD_DESPAWN       = 1, // despawns after a specified time OR when the creature disappears
     *         TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN     = 2, // despawns after a specified time OR when the creature dies
     *         TEMPSUMMON_TIMED_DESPAWN               = 3, // despawns after a specified time
     *         TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT = 4, // despawns after a specified time after the creature is out of combat
     *         TEMPSUMMON_CORPSE_DESPAWN              = 5, // despawns instantly after death
     *         TEMPSUMMON_CORPSE_TIMED_DESPAWN        = 6, // despawns after a specified time after death
     *         TEMPSUMMON_DEAD_DESPAWN                = 7, // despawns when the creature disappears
     *         TEMPSUMMON_MANUAL_DESPAWN              = 8, // despawns when UnSummon() is called
     *         TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN = 9, // despawns after a specified time (OOC) OR when the creature dies
     *         TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN   = 10 // despawns after a specified time (OOC) OR when the creature disappears
     *     };
     *
     * @param uint32 entry : [Creature]'s entry ID
     * @param float x
     * @param float y
     * @param float z
     * @param float o
     * @param [TempSummonType] spawnType = MANUAL_DESPAWN : defines how and when the creature despawns
     * @param uint32 despawnTimer = 0 : despawn time in milliseconds
     * @return [Creature] spawnedCreature
     */
    Creature* SpawnCreature(WorldObject* worldobject, uint32 entry, float x, float y, float z, float o, sol::optional<uint32> spawnTypeArg, sol::optional<uint32> despawnTimerArg)
    {
        uint32 spawnType = spawnTypeArg.value_or(8);
        uint32 despawnTimer = despawnTimerArg.value_or(0);

        TempSummonType type;
        switch (spawnType)
        {
            case 1:
                type = TEMPSUMMON_TIMED_OR_DEAD_DESPAWN;
                break;
            case 2:
                type = TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN;
                break;
            case 3:
                type = TEMPSUMMON_TIMED_DESPAWN;
                break;
            case 4:
                type = TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT;
                break;
            case 5:
                type = TEMPSUMMON_CORPSE_DESPAWN;
                break;
            case 6:
                type = TEMPSUMMON_CORPSE_TIMED_DESPAWN;
                break;
            case 7:
                type = TEMPSUMMON_DEAD_DESPAWN;
                break;
            case 8:
                type = TEMPSUMMON_MANUAL_DESPAWN;
                break;
            default:
                throw std::invalid_argument("valid SpawnType expected");
        }

        return worldobject->SummonCreature(entry, x, y, z, o, type, despawnTimer);
    }

    /**
     * Registers a timed event to the [WorldObject]
     * When the passed function is called, the parameters `(eventId, delay, repeats, worldobject)` are passed to it.
     * Repeats will decrease on each call if the event does not repeat indefinitely
     *
     * Note that for [Creature] and [GameObject] the timed event timer ticks only if the creature is in sight of someone
     * For all [WorldObject]s the timed events are removed when the object is destoryed. This means that for example a [Player]'s events are removed on logout.
     *
     *     local function Timed(eventid, delay, repeats, worldobject)
     *         print(worldobject:GetName())
     *     end
     *     worldobject:RegisterEvent(Timed, 1000, 5) -- do it after 1 second 5 times
     *     worldobject:RegisterEvent(Timed, {1000, 10000}, 0) -- do it after 1 to 10 seconds forever
     *
     * @proto eventId = (function, delay)
     * @proto eventId = (function, delaytable)
     * @proto eventId = (function, delay, repeats)
     * @proto eventId = (function, delaytable, repeats)
     *
     * @param function function : function to trigger when the time has passed
     * @param uint32 delay : set time in milliseconds for the event to trigger
     * @param table delaytable : a table `{min, max}` containing the minimum and maximum delay time
     * @param uint32 repeats = 1 : how many times for the event to repeat, 0 is infinite
     * @return int eventId : unique ID for the timed event used to cancel it or nil
     */
    uint64 RegisterEvent(WorldObject* worldobject, sol::protected_function function, sol::object delay, sol::optional<uint32> repeatsArg)
    {
        uint32 min, max;
        if (delay.is<sol::table>())
        {
            sol::table delayTable = delay.as<sol::table>();
            min = delayTable.get<uint32>(1);
            max = delayTable.get<uint32>(2);
        }
        else
        {
            sol::optional<uint32> delayValue = delay.as<sol::optional<uint32>>();
            if (!delayValue)
                throw std::invalid_argument("number or table expected");

            min = max = *delayValue;
        }
        uint32 repeats = repeatsArg.value_or(1);

        if (min > max)
            throw std::invalid_argument("min is bigger than max delay");

        return worldobject->ALEEvents->AddEvent(function, min, max, repeats);
    }

    /**
     * Removes the timed event from a [WorldObject] by the specified event ID
     *
     * @param int eventId : event Id to remove
     */
    void RemoveEventById(WorldObject* worldobject, uint64 eventId)
    {
        worldobject->ALEEvents->SetState(eventId, LUAEVENT_STATE_ABORT);
    }

    /**
     * Removes all timed events from a [WorldObject]
     *
     */
    void RemoveEvents(WorldObject* worldobject)
    {
        worldobject->ALEEvents->SetStates(LUAEVENT_STATE_ABORT);
    }

    /**
     * Returns true if the given [WorldObject] or coordinates are in the [WorldObject]'s line of sight
     *
     * @proto isInLoS = (worldobject)
     * @proto isInLoS = (x, y, z)
     *
     * @param [WorldObject] worldobject
     * @param float x
     * @param float y
     * @param float z
     * @return bool isInLoS
     */
    bool IsWithinLoS(WorldObject* worldobject, sol::object first, sol::optional<float> yArg, sol::optional<float> zArg)
    {
        if (first.is<WorldObjectRef>())
            if (WorldObject* target = first.as<WorldObjectRef>().Resolve())
                return worldobject->IsWithinLOSInMap(target);

        if (!first.is<float>() || !yArg || !zArg)
            throw std::invalid_argument("WorldObject or coordinates (x, y, z) expected");

        float x = first.as<float>();
        float y = *yArg;
        float z = *zArg;
        return worldobject->IsWithinLOS(x, y, z);
    }

    /**
     * Returns true if the [WorldObject]s are on the same map
     *
     * @param [WorldObject] worldobject
     * @return bool isInMap
     */
    bool IsInMap(WorldObject* worldobject, WorldObject* target)
    {
        return worldobject->IsInMap(target);
    }

    /**
     * Returns true if the point is in the given distance of the [WorldObject]
     *
     * Notice that the distance is measured from the edge of the [WorldObject].
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float distance
     * @return bool isInDistance
     */
    bool IsWithinDist3d(WorldObject* worldobject, float x, float y, float z, float dist)
    {
        return worldobject->IsWithinDist3d(x, y, z, dist);
    }

    /**
     * Returns true if the point is in the given distance of the [WorldObject]
     *
     * The distance is measured only in x,y coordinates.
     * Notice that the distance is measured from the edge of the [WorldObject].
     *
     * @param float x
     * @param float y
     * @param float distance
     * @return bool isInDistance
     */
    bool IsWithinDist2d(WorldObject* worldobject, float x, float y, float dist)
    {
        return worldobject->IsWithinDist2d(x, y, dist);
    }

    /**
     * Returns true if the target is in the given distance of the [WorldObject]
     *
     * Notice that the distance is measured from the edge of the [WorldObject]s.
     *
     * @param [WorldObject] target
     * @param float distance
     * @param bool is3D = true : if false, only x,y coordinates used for checking
     * @return bool isInDistance
     */
    bool IsWithinDist(WorldObject* worldobject, WorldObject* target, float distance, sol::optional<bool> is3DArg)
    {
        bool is3D = is3DArg.value_or(true);
        return worldobject->IsWithinDist(target, distance, is3D);
    }

    /**
     * Returns true if the [WorldObject] is on the same map and within given distance
     *
     * Notice that the distance is measured from the edge of the [WorldObject]s.
     *
     * @param [WorldObject] target
     * @param float distance
     * @param bool is3D = true : if false, only x,y coordinates used for checking
     * @return bool isInDistance
     */
    bool IsWithinDistInMap(WorldObject* worldobject, WorldObject* target, float distance, sol::optional<bool> is3DArg)
    {
        bool is3D = is3DArg.value_or(true);

        return worldobject->IsWithinDistInMap(target, distance, is3D);
    }

    /**
     * Returns true if the target is within given range
     *
     * Notice that the distance is measured from the edge of the [WorldObject]s.
     *
     * @param [WorldObject] target
     * @param float minrange
     * @param float maxrange
     * @param bool is3D = true : if false, only x,y coordinates used for checking
     * @return bool isInDistance
     */
    bool IsInRange(WorldObject* worldobject, WorldObject* target, float minrange, float maxrange, sol::optional<bool> is3DArg)
    {
        bool is3D = is3DArg.value_or(true);

        return worldobject->IsInRange(target, minrange, maxrange, is3D);
    }

    /**
     * Returns true if the point is within given range
     *
     * Notice that the distance is measured from the edge of the [WorldObject].
     *
     * @param float x
     * @param float y
     * @param float minrange
     * @param float maxrange
     * @return bool isInDistance
     */
    bool IsInRange2d(WorldObject* worldobject, float x, float y, float minrange, float maxrange)
    {
        return worldobject->IsInRange2d(x, y, minrange, maxrange);
    }

    /**
     * Returns true if the point is within given range
     *
     * Notice that the distance is measured from the edge of the [WorldObject].
     *
     * @param float x
     * @param float y
     * @param float z
     * @param float minrange
     * @param float maxrange
     * @return bool isInDistance
     */
    bool IsInRange3d(WorldObject* worldobject, float x, float y, float z, float minrange, float maxrange)
    {
        return worldobject->IsInRange3d(x, y, z, minrange, maxrange);
    }

    /**
     * Returns true if the target is in the given arc in front of the [WorldObject]
     *
     * @param [WorldObject] target
     * @param float arc = pi
     * @return bool isInFront
     */
    bool IsInFront(WorldObject* worldobject, WorldObject* target, sol::optional<float> arcArg)
    {
        float arc = arcArg.value_or(static_cast<float>(M_PI));

        return worldobject->isInFront(target, arc);
    }

    /**
     * Returns true if the target is in the given arc behind the [WorldObject]
     *
     * @param [WorldObject] target
     * @param float arc = pi
     * @return bool isInBack
     */
    bool IsInBack(WorldObject* worldobject, WorldObject* target, sol::optional<float> arcArg)
    {
        float arc = arcArg.value_or(static_cast<float>(M_PI));

        return worldobject->isInBack(target, arc);
    }

    /**
     * The [WorldObject] plays music to a [Player]
     *
     * If no [Player] provided it will play the music to everyone near.
     * This method does not interrupt previously played music.
     *
     * See also [WorldObject:PlayDistanceSound], [WorldObject:PlayDirectSound]
     *
     * @param uint32 music : entry of a music
     * @param [Player] player = nil : [Player] to play the music to
     */
    void PlayMusic(WorldObject* worldobject, uint32 musicid, sol::optional<PlayerRef> playerArg)
    {
        Player* player = playerArg ? playerArg->Resolve() : nullptr;

        WorldPacket data(SMSG_PLAY_MUSIC, 4);
        data << uint32(musicid);
        if (player)
            player->SendDirectMessage(&data);
        else
            worldobject->SendMessageToSet(&data, true);
    }

    /**
     * The [WorldObject] plays a sound to a [Player]
     *
     * If no [Player] provided it will play the sound to everyone near.
     * This method will play sound and does not interrupt prvious sound.
     *
     * See also [WorldObject:PlayDistanceSound], [WorldObject:PlayMusic]
     *
     * @param uint32 sound : entry of a sound
     * @param [Player] player = nil : [Player] to play the sound to
     */
    void PlayDirectSound(WorldObject* worldobject, uint32 soundId, sol::optional<PlayerRef> playerArg)
    {
        Player* player = playerArg ? playerArg->Resolve() : nullptr;
        if (!sSoundEntriesStore.LookupEntry(soundId))
            return;

        if (player)
            worldobject->PlayDirectSound(soundId, player);
        else
            worldobject->PlayDirectSound(soundId);
    }

    /**
     * The [WorldObject] plays a sound to a [Player]
     *
     * If no [Player] it will play the sound to everyone near.
     * Sound will fade the further you are from the [WorldObject].
     * This method interrupts previously playing sound.
     *
     * See also [WorldObject:PlayDirectSound], [WorldObject:PlayMusic]
     *
     * @param uint32 sound : entry of a sound
     * @param [Player] player = nil : [Player] to play the sound to
     */
    void PlayDistanceSound(WorldObject* worldobject, uint32 soundId, sol::optional<PlayerRef> playerArg)
    {
        Player* player = playerArg ? playerArg->Resolve() : nullptr;
        if (!sSoundEntriesStore.LookupEntry(soundId))
            return;

        if (player)
            worldobject->PlayDistanceSound(soundId, player);
        else
            worldobject->PlayDistanceSound(soundId);
    }
}

void RegisterWorldObjectMethods(sol::state& lua)
{
    sol::usertype<WorldObjectRef> type = ALEBind::NewHandleType<WorldObjectRef, ObjectRef>(lua, "WorldObject");

    type["GetName"]               = ALEBind::Method(&LuaWorldObject::GetName);
    type["GetMap"]                = ALEBind::Method(&LuaWorldObject::GetMap);
    type["GetPhaseMask"]          = ALEBind::Method(&LuaWorldObject::GetPhaseMask);
    type["SetPhaseMask"]          = ALEBind::Method(&LuaWorldObject::SetPhaseMask);
    type["GetInstanceId"]         = ALEBind::Method(&LuaWorldObject::GetInstanceId);
    type["GetAreaId"]             = ALEBind::Method(&LuaWorldObject::GetAreaId);
    type["GetZoneId"]             = ALEBind::Method(&LuaWorldObject::GetZoneId);
    type["GetMapId"]              = ALEBind::Method(&LuaWorldObject::GetMapId);
    type["GetX"]                  = ALEBind::Method(&LuaWorldObject::GetX);
    type["GetY"]                  = ALEBind::Method(&LuaWorldObject::GetY);
    type["GetZ"]                  = ALEBind::Method(&LuaWorldObject::GetZ);
    type["GetO"]                  = ALEBind::Method(&LuaWorldObject::GetO);
    type["GetLocation"]           = ALEBind::Method(&LuaWorldObject::GetLocation);
    type["GetNearestPlayer"]      = ALEBind::Method(&LuaWorldObject::GetNearestPlayer);
    type["GetNearestGameObject"]  = ALEBind::Method(&LuaWorldObject::GetNearestGameObject);
    type["GetNearestCreature"]    = ALEBind::Method(&LuaWorldObject::GetNearestCreature);
    type["GetPlayersInRange"]     = ALEBind::Method(&LuaWorldObject::GetPlayersInRange);
    type["GetCreaturesInRange"]   = ALEBind::Method(&LuaWorldObject::GetCreaturesInRange);
    type["GetGameObjectsInRange"] = ALEBind::Method(&LuaWorldObject::GetGameObjectsInRange);
    type["GetNearObject"]         = ALEBind::Method(&LuaWorldObject::GetNearObject);
    type["GetNearObjects"]        = ALEBind::Method(&LuaWorldObject::GetNearObjects);
    type["GetDistance"]           = ALEBind::Method(&LuaWorldObject::GetDistance);
    type["GetExactDistance"]      = ALEBind::Method(&LuaWorldObject::GetExactDistance);
    type["GetDistance2d"]         = ALEBind::Method(&LuaWorldObject::GetDistance2d);
    type["GetExactDistance2d"]    = ALEBind::Method(&LuaWorldObject::GetExactDistance2d);
    type["GetRelativePoint"]      = ALEBind::Method(&LuaWorldObject::GetRelativePoint);
    type["GetAngle"]              = ALEBind::Method(&LuaWorldObject::GetAngle);
    type["GetTransport"]          = ALEBind::Method(&LuaWorldObject::GetTransport);
    type["SendPacket"]            = ALEBind::Method(&LuaWorldObject::SendPacket);
    type["SummonGameObject"]      = ALEBind::Method(&LuaWorldObject::SummonGameObject);
    type["SpawnCreature"]         = ALEBind::Method(&LuaWorldObject::SpawnCreature);
    type["RegisterEvent"]         = ALEBind::Method(&LuaWorldObject::RegisterEvent);
    type["RemoveEventById"]       = ALEBind::Method(&LuaWorldObject::RemoveEventById);
    type["RemoveEvents"]          = ALEBind::Method(&LuaWorldObject::RemoveEvents);
    type["IsWithinLoS"]           = ALEBind::Method(&LuaWorldObject::IsWithinLoS);
    type["IsInMap"]               = ALEBind::Method(&LuaWorldObject::IsInMap);
    type["IsWithinDist3d"]        = ALEBind::Method(&LuaWorldObject::IsWithinDist3d);
    type["IsWithinDist2d"]        = ALEBind::Method(&LuaWorldObject::IsWithinDist2d);
    type["IsWithinDist"]          = ALEBind::Method(&LuaWorldObject::IsWithinDist);
    type["IsWithinDistInMap"]     = ALEBind::Method(&LuaWorldObject::IsWithinDistInMap);
    type["IsInRange"]             = ALEBind::Method(&LuaWorldObject::IsInRange);
    type["IsInRange2d"]           = ALEBind::Method(&LuaWorldObject::IsInRange2d);
    type["IsInRange3d"]           = ALEBind::Method(&LuaWorldObject::IsInRange3d);
    type["IsInFront"]             = ALEBind::Method(&LuaWorldObject::IsInFront);
    type["IsInBack"]              = ALEBind::Method(&LuaWorldObject::IsInBack);
    type["PlayMusic"]             = ALEBind::Method(&LuaWorldObject::PlayMusic);
    type["PlayDirectSound"]       = ALEBind::Method(&LuaWorldObject::PlayDirectSound);
    type["PlayDistanceSound"]     = ALEBind::Method(&LuaWorldObject::PlayDistanceSound);
}
