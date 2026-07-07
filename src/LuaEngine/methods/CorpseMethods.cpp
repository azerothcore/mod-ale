/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Corpse.h"

/***
 * The remains of a [Player] that has died.
 *
 * Inherits all methods from: [Object], [WorldObject]
 */
namespace LuaCorpse
{
    /**
     * Returns the GUID of the [Player] that left the [Corpse] behind.
     *
     * @return ObjectGuid ownerGUID
     */
    ObjectGuid GetOwnerGUID(Corpse* corpse)
    {
        return corpse->GetOwnerGUID();
    }

    /**
     * Returns the time when the [Player] became a ghost and spawned this [Corpse].
     *
     * @return uint32 ghostTime
     */
    uint32 GetGhostTime(Corpse* corpse)
    {
        return corpse->GetGhostTime();
    }

    /**
     * Returns the [CorpseType] of a [Corpse].
     *
     *     enum CorpseType
     *     {
     *         CORPSE_BONES             = 0,
     *         CORPSE_RESURRECTABLE_PVE = 1,
     *         CORPSE_RESURRECTABLE_PVP = 2
     *     };
     *
     * @return [CorpseType] corpseType
     */
    CorpseType GetType(Corpse* corpse)
    {
        return corpse->GetType();
    }

    /**
     * Sets the "ghost time" to the current time.
     *
     * See [Corpse:GetGhostTime].
     */
    void ResetGhostTime(Corpse* corpse)
    {
        corpse->ResetGhostTime();
    }

    /**
     * Saves the [Corpse] to the database.
     */
    void SaveToDB(Corpse* corpse)
    {
        corpse->SaveToDB();
    }
}

void RegisterCorpseMethods(sol::state& lua)
{
    sol::usertype<CorpseRef> type = ALEBind::NewHandleType<CorpseRef, WorldObjectRef, ObjectRef>(lua, "Corpse");

    type["GetOwnerGUID"]   = ALEBind::Method(&LuaCorpse::GetOwnerGUID);
    type["GetGhostTime"]   = ALEBind::Method(&LuaCorpse::GetGhostTime);
    type["GetType"]        = ALEBind::Method(&LuaCorpse::GetType);
    type["ResetGhostTime"] = ALEBind::Method(&LuaCorpse::ResetGhostTime);
    type["SaveToDB"]       = ALEBind::Method(&LuaCorpse::SaveToDB);
}
