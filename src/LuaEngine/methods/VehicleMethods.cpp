/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Unit.h"
#include "Vehicle.h"

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
    bool IsOnBoard(Vehicle* vehicle, Unit* passenger)
    {
        return passenger->IsOnVehicle(vehicle->GetBase());
    }

    /**
     * Returns the [Vehicle]'s owner
     *
     * @return [Unit] owner
     */
    Unit* GetOwner(Vehicle* vehicle)
    {
        return vehicle->GetBase();
    }

    /**
     * Returns the [Vehicle]'s entry
     *
     * @return uint32 entry
     */
    uint32 GetEntry(Vehicle* vehicle)
    {
        return vehicle->GetVehicleInfo()->m_ID;
    }

    /**
     * Returns the [Vehicle]'s passenger in the specified seat
     *
     * @param int8 seat
     * @return [Unit] passenger
     */
    Unit* GetPassenger(Vehicle* vehicle, int8 seatId)
    {
        return vehicle->GetPassenger(seatId);
    }

    /**
     * Adds [Unit] passenger to a specified seat in the [Vehicle]
     *
     * @param [Unit] passenger
     * @param int8 seat
     */
    void AddPassenger(Vehicle* vehicle, Unit* passenger, int8 seatId)
    {
        vehicle->AddPassenger(passenger, seatId);
    }

    /**
     * Removes [Unit] passenger from the [Vehicle]
     *
     * @param [Unit] passenger
     */
    void RemovePassenger(Vehicle* vehicle, Unit* passenger)
    {
        vehicle->RemovePassenger(passenger);
    }
}

void RegisterVehicleMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<Vehicle>> type = ALEBind::NewHandleType<ScopedRef<Vehicle>>(lua, "Vehicle");

    type["IsOnBoard"]       = ALEBind::Method(&LuaVehicle::IsOnBoard);
    type["GetOwner"]        = ALEBind::Method(&LuaVehicle::GetOwner);
    type["GetEntry"]        = ALEBind::Method(&LuaVehicle::GetEntry);
    type["GetPassenger"]    = ALEBind::Method(&LuaVehicle::GetPassenger);
    type["AddPassenger"]    = ALEBind::Method(&LuaVehicle::AddPassenger);
    type["RemovePassenger"] = ALEBind::Method(&LuaVehicle::RemovePassenger);
}
