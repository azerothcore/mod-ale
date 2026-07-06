/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"

using namespace Hooks;

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<VehicleEvents>(EVENT);\
    if (!VehicleEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

void ALE::OnInstall(Vehicle* vehicle)
{
    START_HOOK(VEHICLE_EVENT_ON_INSTALL);
    CallAll(*VehicleEventBindings, key, vehicle);
}

void ALE::OnUninstall(Vehicle* vehicle)
{
    START_HOOK(VEHICLE_EVENT_ON_UNINSTALL);
    CallAll(*VehicleEventBindings, key, vehicle);
}

void ALE::OnInstallAccessory(Vehicle* vehicle, Creature* accessory)
{
    START_HOOK(VEHICLE_EVENT_ON_INSTALL_ACCESSORY);
    CallAll(*VehicleEventBindings, key, vehicle, accessory);
}

void ALE::OnAddPassenger(Vehicle* vehicle, Unit* passenger, int8 seatId)
{
    START_HOOK(VEHICLE_EVENT_ON_ADD_PASSENGER);
    CallAll(*VehicleEventBindings, key, vehicle, passenger, seatId);
}

void ALE::OnRemovePassenger(Vehicle* vehicle, Unit* passenger)
{
    START_HOOK(VEHICLE_EVENT_ON_REMOVE_PASSENGER);
    CallAll(*VehicleEventBindings, key, vehicle, passenger);
}
