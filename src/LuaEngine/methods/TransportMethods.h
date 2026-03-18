/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef TRANSPORTMETHODS_H
#define TRANSPORTMETHODS_H

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
    int GetPassengers(lua_State* L, Transport* transport)
    {
        Transport::PassengerSet const& passengers = transport->GetPassengers();
        lua_createtable(L, static_cast<int>(passengers.size()), 0);
        int i = 1;
        for (WorldObject* passenger : passengers)
        {
            ALE::Push(L, passenger);
            lua_rawseti(L, -2, i++);
        }
        return 1;
    }

    /**
     * Returns 'true' if the [Transport] is a MotionTransport (moving transport such as a boat or zeppelin)
     *
     * @return bool isMotionTransport
     */
    int IsMotionTransport(lua_State* L, Transport* transport)
    {
        ALE::Push(L, dynamic_cast<MotionTransport*>(transport) != nullptr);
        return 1;
    }

    /**
     * Adds a [WorldObject] as a passenger to the [Transport]
     *
     * @param [WorldObject] passenger : the object to add as a passenger
     * @param bool withAll = true : if true, also sets transport movement info on the passenger
     */
    int AddPassenger(lua_State* L, Transport* transport)
    {
        WorldObject* passenger = ALE::CHECKOBJ<WorldObject>(L, 2);
        bool withAll = ALE::CHECKVAL<bool>(L, 3, true);
        transport->AddPassenger(passenger, withAll);
        return 0;
    }

    /**
     * Removes a [WorldObject] passenger from the [Transport]
     *
     * @param [WorldObject] passenger : the object to remove
     * @param bool withAll = true : if true, also clears transport movement info from the passenger
     */
    int RemovePassenger(lua_State* L, Transport* transport)
    {
        WorldObject* passenger = ALE::CHECKOBJ<WorldObject>(L, 2);
        bool withAll = ALE::CHECKVAL<bool>(L, 3, true);
        transport->RemovePassenger(passenger, withAll);
        return 0;
    }

    /**
     * Enables or disables movement on the [Transport]
     *
     * Only works on MotionTransports where canBeStopped is set.
     *
     * @param bool enabled : true to enable movement, false to stop
     */
    int EnableMovement(lua_State* L, Transport* transport)
    {
        bool enabled = ALE::CHECKVAL<bool>(L, 2);
        MotionTransport* mt = dynamic_cast<MotionTransport*>(transport);
        if (mt)
            mt->EnableMovement(enabled);
        return 0;
    }
}

#endif
