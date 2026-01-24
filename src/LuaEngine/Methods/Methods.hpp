/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_METHODS_HPP
#define _ALE_METHODS_HPP

#include <sol/sol.hpp>
#include "GlobalMethods.hpp"
#include "UnitMethods.hpp"
#include "PlayerMethods.hpp"
#include "Log.h"

namespace ALE::Methods
{
    /**
     * @brief Register ALL Lua methods and functions to a state
     * 
     * @param state Lua state to bind all functions to
     * @param mapId Map ID for this state (-1 = master, 0+ = specific map)
     * @param timedEventMgr TimedEventManager for this state (per-map isolation)
     * 
     * @note CRITICAL ORDER: Base classes MUST be registered BEFORE derived classes!
     *       1. Unit      (base class)
     *       2. Player    (inherits Unit via sol::bases<Unit>())
     *       3. Creature  (future - will also inherit Unit)
     *       4. Global functions & timed events
     * 
     * @note This is the ONLY place where Lua bindings should be registered.
     *       Adding a new global function? Add it to GlobalMethods.
     *       Adding a new Unit method? Add it to UnitMethods.
     *       Adding a new Player method? Add it to PlayerMethods.
     */
    inline void RegisterAllMethods(sol::state& state, Core::TimedEventManager* timedEventMgr)
    {
        // Unit MUST be registered BEFORE Player!
        // Player uses sol::bases<Unit>() which requires Unit to be registered first
        RegisterUnitMethods(state);

        // Player inherits all Unit methods automatically
        RegisterPlayerMethods(state);

        // Global functions (_G namespace) - includes:
        //   - Event system (RegisterServerEvent, RegisterPlayerEvent, etc.)
        //   - Global timed events (CreateLuaEvent, RemoveTimedEvent, etc.)
        //   - Object timed event methods (Player:RegisterEvent, Creature:RegisterEvent, etc.)
        RegisterGlobalMethods(state, timedEventMgr);
    }

} // namespace ALE::Methods

#endif // _ALE_METHODS_HPP
