/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef VEHICLEMETHODS_H
#define VEHICLEMETHODS_H

/***
 * Represents a vehicle in the game, which can carry passengers and provide special abilities or movement.
 *
 * Inherits all methods from: none
 */
namespace LuaVehicle
{
    /**
     * Returns true if the [Unit] passenger is on board
     *
     * @param [Unit] passenger
     * @return bool isOnBoard
     */
    int IsOnBoard(lua_State* L, Vehicle* vehicle)
    {
        Unit* passenger = ALE::CHECKOBJ<Unit>(L, 2);
        ALE::Push(L, passenger->IsOnVehicle(vehicle->GetBase()));
        return 1;
    }

    /**
     * Returns the [Vehicle]'s owner
     *
     * @return [Unit] owner
     */
    int GetOwner(lua_State* L, Vehicle* vehicle)
    {
        ALE::Push(L, vehicle->GetBase());
        return 1;
    }

    /**
     * Returns the [Vehicle]'s entry
     *
     * @return uint32 entry
     */
    int GetEntry(lua_State* L, Vehicle* vehicle)
    {
        ALE::Push(L, vehicle->GetVehicleInfo()->m_ID);
        return 1;
    }

    /**
     * Returns the [Vehicle]'s passenger in the specified seat
     *
     * @param int8 seat
     * @return [Unit] passenger
     */
    int GetPassenger(lua_State* L, Vehicle* vehicle)
    {
        int8 seatId = ALE::CHECKVAL<int8>(L, 2);
        ALE::Push(L, vehicle->GetPassenger(seatId));
        return 1;
    }

    /**
     * Adds [Unit] passenger to a specified seat in the [Vehicle]
     *
     * @param [Unit] passenger
     * @param int8 seat
     */
    int AddPassenger(lua_State* L, Vehicle* vehicle)
    {
        Unit* passenger = ALE::CHECKOBJ<Unit>(L, 2);
        int8 seatId = ALE::CHECKVAL<int8>(L, 3);

        vehicle->AddPassenger(passenger, seatId);
        return 0;
    }

    /**
     * Removes [Unit] passenger from the [Vehicle]
     *
     * @param [Unit] passenger
     */
    int RemovePassenger(lua_State* L, Vehicle* vehicle)
    {
        Unit* passenger = ALE::CHECKOBJ<Unit>(L, 2);
        vehicle->RemovePassenger(passenger);
        return 0;
    }
}

#endif // VEHICLEMETHODS_H
