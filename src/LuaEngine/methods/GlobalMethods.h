/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ECLIPSE_GLOBAL_METHODS_H
#define _ECLIPSE_GLOBAL_METHODS_H

#include <sol/sol.hpp>
#include "EventManager.h"
#include "Common.h"

// Forward declarations
class Player;
class Creature;
class GameObject;

namespace Eclipse::Core
{
    class TimedEventManager;
}

namespace Eclipse::Methods
{
    /**
     * @class GlobalMethods
     * @brief Global Lua functions for Eclipse event system
     * 
     * **Purpose:**
     * Centralized namespace for ALL global Lua functions exposed by Eclipse.
     * Keeps global API clean and organized in one place.
     * 
     * **Exported Functions:**
     * - RegisterServerEvent(eventId, callback, shots)
     * - RegisterPlayerEvent(eventId, callback, shots)
     * - CancelEvent(handlerId)
     * - CancelServerEvent(eventId)
     * - CancelPlayerEvent(eventId)
     * 
     * **Architecture:**
     * - All functions are stateless (delegate to EventManager singleton)
     * - Thread-safe via EventManager's lock-free architecture
     * - Move semantics for callback functions (zero-copy)
     * 
     * **Usage in Lua:**
     * ```lua
     * local handlerId = RegisterPlayerEvent(2, function(eventId, player)
     *     print("Player logged in: " .. player:GetName())
     * end, 0)  -- 0 = infinite shots
     * 
     * CancelEvent(handlerId)  -- Cancel specific handler
     * CancelPlayerEvent(2)     -- Cancel all login handlers
     * ```
     * 
     * @note This namespace is registered to Lua via EventAPI::RegisterAPI()
     */
    namespace GlobalMethods
    {
        // ========================================================================
        // EVENT REGISTRATION
        // ========================================================================

        /**
         * @brief Register all global event functions to Lua state
         * 
         * Centralized registration for all state.set_function bindings.
         * Called by Methods::RegisterAllMethods().
         * 
         * Registers:
         * - RegisterServerEvent
         * - RegisterPlayerEvent
         * - CancelEvent
         * - CancelServerEvent
         * - CancelPlayerEvent
         * 
         * @param state Lua state to bind functions to
         * 
         * @performance ~50µs (5 function pointer assignments)
         */
        void RegisterGlobalEventFunctions(sol::state& state);

        /**
         * @brief Register a server/world event handler
         * 
         * Registers a Lua callback for world events (server start, config reload, etc.).
         * 
         * @param event Event ID (WorldEvent enum as uint32)
         * @param handler Lua callback function(eventId, ...)
         * @param shots Number of times to call (0 = infinite)
         * @return Handler ID for cancellation
         * 
         * @performance O(1) - Lock-free vector append
         * @thread-safety Thread-safe via EventManager
         * 
         * @example Lua:
         * ```lua
         * RegisterServerEvent(1, function(eventId)
         *     print("Server event triggered!")
         * end)
         * ```
         */
        uint64 RegisterServerEvent(uint32 event, sol::protected_function handler, sol::optional<uint32> shots);

        /**
         * @brief Register a player event handler
         * 
         * Registers a Lua callback for player events (login, logout, level up, etc.).
         * 
         * @param event Event ID (PlayerEvent enum as uint32)
         * @param handler Lua callback function(eventId, player, ...)
         * @param shots Number of times to call (0 = infinite)
         * @return Handler ID for cancellation
         *
         * @performance O(1) - Lock-free vector append
         * @thread-safety Thread-safe via EventManager
         *
         * @example Lua:
         * ```lua
         * RegisterPlayerEvent(2, function(eventId, player)
         *     print(player:GetName() .. " logged in!")
         * end)
         * ```
         */
        uint64 RegisterPlayerEvent(uint32 event, sol::protected_function handler, sol::optional<uint32> shots);

        // ========================================================================
        // EVENT CANCELLATION
        // ========================================================================

        /**
         * @brief Cancel a specific event handler by ID
         * 
         * Removes a single handler registered via RegisterXEvent().
         * 
         * @param handlerId Handler ID returned by RegisterXEvent
         * @return true if handler was found and cancelled, false otherwise
         * 
         * @performance O(n) where n = number of handlers for that event
         * @thread-safety Thread-safe via EventManager
         * 
         * @example Lua:
         * ```lua
         * local id = RegisterPlayerEvent(2, callback)
         * CancelEvent(id)  -- Remove this specific handler
         * ```
         */
        bool CancelEvent(uint64 handlerId);

        /**
         * @brief Cancel all handlers for a server event
         * 
         * Removes ALL handlers registered for a specific server event ID.
         * 
         * @param event Event ID (WorldEvent enum as uint32)
         * 
         * @performance O(1) - Clears vector
         * @thread-safety Thread-safe via EventManager
         * 
         * @example Lua:
         * ```lua
         * CancelServerEvent(1)  -- Remove all handlers for event 1
         * ```
         */
        void CancelServerEvent(uint32 event);

        /**
         * @brief Cancel all handlers for a player event
         * 
         * Removes ALL handlers registered for a specific player event ID.
         * 
         * @param event Event ID (PlayerEvent enum as uint32)
         * 
         * @performance O(1) - Clears vector
         * @thread-safety Thread-safe via EventManager
         * 
         * @example Lua:
         * ```lua
         * CancelPlayerEvent(2)  -- Remove all login handlers
         * ```
         */
        void CancelPlayerEvent(uint32 event);

        // ========================================================================
        // HELPER FUNCTIONS (Internal use)
        // ========================================================================

        /**
         * @brief Convert server event ID to internal EventType
         * @param event Event ID from Lua (uint32)
         * @return Typed WorldEvent enum
         */
        Core::WorldEvent ServerEventToType(uint32 event);

        /**
         * @brief Convert player event ID to internal EventType
         * @param event Event ID from Lua (uint32)
         * @return Typed PlayerEvent enum
         */
        Core::PlayerEvent PlayerEventToType(uint32 event);

        // ========================================================================
        // TIMED EVENTS (Global)
        // ========================================================================

        /**
         * @brief Create a global timed event (delayed/repeating callback)
         * 
         * Registers a callback to execute after a delay, optionally repeating.
         * Uses TimedEventManager for precise timing.
         * 
         * @param mgr TimedEventManager for this state (injected by EventAPI)
         * @param callback Lua function to call
         * @param delay Delay in milliseconds before first execution
         * @param repeats Number of times to repeat (1 = once, 0 = infinite)
         * @return Event ID for cancellation
         * 
         * @performance ~100ns registration (lock-free vector append)
         * @thread-safety Thread-safe via per-state manager isolation
         * 
         * @example Lua:
         * ```lua
         * CreateLuaEvent(function()
         *     print("5 seconds elapsed!")
         * end, 5000, 1)  -- 5000ms delay, execute once
         * ```
         */
        void RegisterCreateLuaEvent(sol::state& state, Core::TimedEventManager* mgr);

        /**
         * @brief Remove a specific timed event by ID
         * 
         * @param mgr TimedEventManager for this state
         * @param eventId Event ID returned by CreateLuaEvent
         * @return true if event was found and removed
         */
        void RegisterRemoveTimedEvent(sol::state& state, Core::TimedEventManager* mgr);

        /**
         * @brief Remove all global timed events
         * 
         * Clears all events registered via CreateLuaEvent.
         * Does NOT affect player/creature/gameobject events.
         * 
         * @param mgr TimedEventManager for this state
         */
        void RegisterRemoveAllTimedEvents(sol::state& state, Core::TimedEventManager* mgr);

        /**
         * @brief Get count of active global timed events
         * 
         * @param mgr TimedEventManager for this state
         * @return Number of active global events
         */
        void RegisterGetTimedEventCount(sol::state& state, Core::TimedEventManager* mgr);

    } // namespace GlobalMethods

} // namespace Eclipse::Methods

#endif // _ECLIPSE_GLOBAL_METHODS_H
