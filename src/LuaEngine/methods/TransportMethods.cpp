/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Transport.h"

/***
 * Represents a transport object in the world, such as boats and zeppelins.
 *
 * Inherits all methods from: [Object], [WorldObject], [GameObject]
 */
namespace LuaTransport
{
    /**
     * Returns a table of all passengers on the [Transport]
     *
     * @return table passengers
     */
    sol::table GetPassengers(Transport* transport, sol::this_state s)
    {
        sol::state_view lua(s);
        sol::table tbl = lua.create_table();

        Transport::PassengerSet const& passengers = transport->GetPassengers();
        int i = 1;
        for (WorldObject* passenger : passengers)
            tbl[i++] = ALEBind::ToLuaDynamic(lua, passenger);

        return tbl;
    }

    /**
     * Returns 'true' if the [Transport] is a MotionTransport (moving transport such as a boat or zeppelin)
     *
     * @return bool isMotionTransport
     */
    bool IsMotionTransport(Transport* transport)
    {
        return dynamic_cast<MotionTransport*>(transport) != nullptr;
    }

    /**
     * Adds a [WorldObject] as a passenger to the [Transport]
     *
     * @param [WorldObject] passenger : the object to add as a passenger
     * @param bool withAll = true : if true, also sets transport movement info on the passenger
     */
    void AddPassenger(Transport* transport, WorldObject* passenger, sol::optional<bool> withAll)
    {
        transport->AddPassenger(passenger, withAll.value_or(true));
    }

    /**
     * Removes a [WorldObject] passenger from the [Transport]
     *
     * @param [WorldObject] passenger : the object to remove
     * @param bool withAll = true : if true, also clears transport movement info from the passenger
     */
    void RemovePassenger(Transport* transport, WorldObject* passenger, sol::optional<bool> withAll)
    {
        transport->RemovePassenger(passenger, withAll.value_or(true));
    }

    /**
     * Enables or disables movement on the [Transport]
     *
     * Only works on MotionTransports where canBeStopped is set.
     *
     * @param bool enabled : true to enable movement, false to stop
     */
    void EnableMovement(Transport* transport, bool enabled)
    {
        MotionTransport* mt = dynamic_cast<MotionTransport*>(transport);
        if (mt)
            mt->EnableMovement(enabled);
    }
}

void RegisterTransportMethods(sol::state& lua)
{
    sol::usertype<TransportRef> type = ALEBind::NewHandleType<TransportRef, GameObjectRef, WorldObjectRef, ObjectRef>(lua, "Transport");

    type["GetPassengers"]     = ALEBind::Method(&LuaTransport::GetPassengers);
    type["IsMotionTransport"] = ALEBind::Method(&LuaTransport::IsMotionTransport);
    type["AddPassenger"]      = ALEBind::Method(&LuaTransport::AddPassenger);
    type["RemovePassenger"]   = ALEBind::Method(&LuaTransport::RemovePassenger);
    type["EnableMovement"]    = ALEBind::Method(&LuaTransport::EnableMovement);
}
