/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "EventManager.h"
#include "StateManager.h"
#include "Log.h"

namespace ALE::Core
{
    /**
     * @brief Get EventManager singleton instance
     *
     * @return Reference to the global EventManager
     */
    EventManager& EventManager::GetInstance()
    {
        static EventManager instance;
        return instance;
    }

    /**
     * @brief Constructor - initializes state
     */
    EventManager::EventManager() : m_nextHandlerId(1), m_initialized(false) { }

    /**
     * @brief Destructor - cleanup all handlers
     */
    EventManager::~EventManager()
    {
        Shutdown();
    }

    /**
     * @brief Initialize event manager
     *
     * Called from ALEScriptLoader during server startup.
     *
     * @return true on success (always succeeds)
     */
    bool EventManager::Initialize()
    {
        if (m_initialized)
        {
            LOG_WARN("server.loading", "[ALE] EventManager - Already initialized");
            return true;
        }

        LOG_INFO("server.loading", "[ALE] EventManager - Initializing event manager");

        m_nextHandlerId = 1;
        m_initialized = true;

        LOG_INFO("server.loading", "[ALE] EventManager - Initialization complete");
        return true;
    }

    /**
     * @brief Shutdown event manager
     *
     * Cancels ALL registered handlers and logs final statistics.
     * Called from ALEScriptLoader during server shutdown.
     */
    void EventManager::Shutdown()
    {
        if (!m_initialized)
            return;

        CancelAllEvents();
        m_initialized = false;
    }

    /**
     * @brief Cancel a specific event handler by ID
     *
     * Searches all three storage maps (global, entry, unique).
     *
     * @param handlerId Handler ID returned by Register*Event
     * @return true if handler was found and removed
     */
    bool EventManager::CancelEvent(uint64 handlerId)
    {
        // Search in all three handler maps
        if (FindAndCancelHandler(m_globalHandlers, handlerId, "global"))
            return true;

        if (FindAndCancelHandler(m_entryHandlers, handlerId, "entry"))
            return true;

        if (FindAndCancelHandler(m_uniqueHandlers, handlerId, "unique"))
            return true;

        LOG_WARN("ale.events", "[ALE] EventManager - Handler {} not found for cancellation", handlerId);
        return false;
    }

    /**
     * @brief Cancel all handlers for a specific state
     *
     * Removes all handlers matching stateId across all event types.
     * Used when a Lua state is destroyed (future per-map cleanup).
     *
     * @param stateId State ID to cleanup
     */
    void EventManager::CancelStateEvents(int32 stateId)
    {
        size_t totalCancelled = 0;

        totalCancelled += CancelStateHandlersInMap(m_globalHandlers, stateId);
        totalCancelled += CancelStateHandlersInMap(m_entryHandlers, stateId);
        totalCancelled += CancelStateHandlersInMap(m_uniqueHandlers, stateId);

        LOG_DEBUG("ale.events", "[ALE] EventManager - Cancelled {} handlers for state {}", totalCancelled, stateId);
    }

    /**
     * @brief Cancel ALL registered events
     *
     * Complete cleanup of all event handlers.
     * Called during shutdown or Lua reload.
     */
    void EventManager::CancelAllEvents()
    {
        size_t total = m_globalHandlers.size() + m_entryHandlers.size() + m_uniqueHandlers.size();

        m_globalHandlers.clear();
        m_entryHandlers.clear();
        m_uniqueHandlers.clear();

        LOG_DEBUG("ale.events", "[ALE] EventManager - Cancelled all event handlers (total: {})", total);
    }

} // namespace ALE::Core
