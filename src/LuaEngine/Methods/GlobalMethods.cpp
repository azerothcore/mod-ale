/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "GlobalMethods.h"
#include "StateManager.h"
#include "TimedEventManager.h"
#include "Log.h"

namespace ALE::Methods::GlobalMethods
{
    /**
     * @brief Convert uint32 event ID to WorldEvent enum
     *
     * Server events use range 1-50 in EventManager.
     * Direct cast from uint32 to enum for type safety.
     */
    Core::WorldEvent ServerEventToType(uint32 event)
    {
        return static_cast<Core::WorldEvent>(event);
    }

    /**
     * @brief Convert uint32 event ID to PlayerEvent enum
     *
     * Player events start at 100 in EventManager.
     * Direct cast from uint32 to enum for type safety.
     */
    Core::PlayerEvent PlayerEventToType(uint32 event)
    {
        return static_cast<Core::PlayerEvent>(event);
    }

    // ========================================================================
    // GLOBAL EVENT FUNCTION REGISTRATION
    // ========================================================================

    /**
     * @brief Register all global event functions to Lua state
     * 
     * Centralizes all state.set_function calls for event-related global functions.
     * Keeps EventAPI.cpp clean by moving binding logic here.
     */
    void RegisterGlobalEventFunctions(sol::state& state)
    {
        state.set_function("RegisterServerEvent", &RegisterServerEvent);
        state.set_function("RegisterPlayerEvent", &RegisterPlayerEvent);

        state.set_function("CancelEvent", &CancelEvent);
        state.set_function("CancelServerEvent", &CancelServerEvent);
        state.set_function("CancelPlayerEvent", &CancelPlayerEvent);

        LOG_DEBUG("ale.methods", "[ALE] GlobalMethods - Registered global event functions");
    }

    // ========================================================================
    // EVENT REGISTRATION IMPLEMENTATIONS
    // ========================================================================

    /**
     * @brief Register a server/world event handler
     *
     * Workflow:
     * 1. Convert event ID to typed enum (ServerEventToType)
     * 2. Move callback to EventManager (zero-copy)
     * 3. Store in lock-free event handler vector
     * 4. Return unique handler ID for cancellation
     *
     */
    uint64 RegisterServerEvent(uint32 event, sol::protected_function handler, sol::optional<uint32> shots)
    {
        auto eventType = ServerEventToType(event);
        return Core::EventManager::GetInstance().RegisterGlobalEvent(
            eventType,
            std::move(handler),
            shots.value_or(0)    // Default: infinite shots
        );
    }

    /**
     * @brief Register a player event handler
     *
     * Workflow:
     * 1. Convert event ID to typed enum (PlayerEventToType)
     * 2. Move callback to EventManager (zero-copy)
     * 3. Store in lock-free event handler vector
     * 4. Return unique handler ID for cancellation
     */
    uint64 RegisterPlayerEvent(uint32 event, sol::protected_function handler, sol::optional<uint32> shots)
    {
        auto eventType = PlayerEventToType(event);
        return Core::EventManager::GetInstance().RegisterGlobalEvent(
            eventType,
            std::move(handler),
            shots.value_or(0)    // Default: infinite shots
        );
    }

    // ========================================================================
    // EVENT CANCELLATION FUNCTIONS
    // ========================================================================

    /**
     * @brief Cancel a specific event handler by ID
     * 
     * Searches all event vectors for matching handler ID.
     * Removes first match found (handler IDs are globally unique).
     */
    bool CancelEvent(uint64 handlerId)
    {
        return Core::EventManager::GetInstance().CancelEvent(handlerId);
    }

    /**
     * @brief Cancel all handlers for a server event
     *
     * Clears the entire handler vector for this event type.
     */
    void CancelServerEvent(uint32 event)
    {
        auto eventType = ServerEventToType(event);
        Core::EventManager::GetInstance().CancelGlobalEvent(eventType);
    }

    /**
     * @brief Cancel all handlers for a player event
     *
     * Clears the entire handler vector for this event type.
     */
    void CancelPlayerEvent(uint32 event)
    {
        auto eventType = PlayerEventToType(event);
        Core::EventManager::GetInstance().CancelGlobalEvent(eventType);
    }

    /**
     * @brief Register CreateLuaEvent global function
     *
     * Lambda captures TimedEventManager pointer for per-state isolation.
     */
    void RegisterCreateLuaEvent(sol::state& state, Core::TimedEventManager* mgr)
    {
        state["CreateLuaEvent"] = [mgr](sol::function callback, uint32 delay, sol::optional<uint32> repeats) -> uint64 {
            uint32 repeatCount = repeats.value_or(1);
            sol::protected_function pf(callback);
            return mgr->RegisterGlobalEvent(std::move(pf), delay, repeatCount);
        };
    }

    /**
     * @brief Register RemoveTimedEvent global function
     */
    void RegisterRemoveTimedEvent(sol::state& state, Core::TimedEventManager* mgr)
    {
        state["RemoveTimedEvent"] = [mgr](uint64 eventId) -> bool {
            return mgr->RemoveEvent(eventId);
        };
    }

    /**
     * @brief Register RemoveAllTimedEvents global function
     */
    void RegisterRemoveAllTimedEvents(sol::state& state, Core::TimedEventManager* mgr)
    {
        state["RemoveAllTimedEvents"] = [mgr]() {
            mgr->RemoveAllGlobalEvents();
        };
    }

    /**
     * @brief Register GetTimedEventCount global function
     */
    void RegisterGetTimedEventCount(sol::state& state, Core::TimedEventManager* mgr)
    {
        state["GetTimedEventCount"] = [mgr]() -> uint32 {
            return mgr->GetGlobalEventCount();
        };
    }

} // namespace ALE::Methods::GlobalMethods
