/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "TimedEventManager.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "Log.h"

namespace Eclipse::Core
{
    TimedEventManager::TimedEventManager(int32 mapId) : m_nextEventId(1), m_mapId(mapId)
    {
        m_globalEvents.reserve(16);
        m_eventsToRemove.reserve(8);
    }

    TimedEventManager::~TimedEventManager()
    {
        Clear();
    }

    uint64 TimedEventManager::RegisterGlobalEvent(sol::protected_function&& callback, uint32 delay, uint32 repeats)
    {
        uint64 eventId = m_nextEventId++;

        m_events.emplace(eventId, TimedEvent(eventId, std::move(callback), delay, repeats));
        m_globalEvents.push_back(eventId);

        LOG_DEBUG("scripts.eclipse", "Registered global event {} (delay={}ms, repeats={})",
                  eventId, delay, repeats);

        return eventId;
    }

    uint64 TimedEventManager::RegisterObjectEvent(ObjectGuid objectGuid, sol::protected_function&& callback, uint32 delay, uint32 repeats, TimedEventObjectType objectType)
    {
        uint64 eventId = m_nextEventId++;

        m_events.emplace(eventId, TimedEvent(eventId, std::move(callback), delay, repeats, objectGuid, objectType));

        m_objectEvents[objectGuid].push_back(eventId);

        LOG_DEBUG("scripts.eclipse", "Registered object event {} for GUID {} (type={}, delay={}ms, repeats={})",
                  eventId, objectGuid.ToString(), static_cast<int>(objectType), delay, repeats);

        return eventId;
    }

    bool TimedEventManager::RemoveEvent(uint64 eventId)
    {
        auto eventIt = m_events.find(eventId);
        if (eventIt == m_events.end())
            return false;  // Event not found

        const TimedEvent& event = eventIt->second;

        // Remove from appropriate index
        if (event.objectType == TimedEventObjectType::GLOBAL)
        {
            // Remove from global events list
            auto it = std::find(m_globalEvents.begin(), m_globalEvents.end(), eventId);
            if (it != m_globalEvents.end())
            {
                std::swap(*it, m_globalEvents.back());
                m_globalEvents.pop_back();
            }
        }
        else
        {
            // Remove from object events index
            auto objIt = m_objectEvents.find(event.objectGuid);
            if (objIt != m_objectEvents.end())
            {
                auto& eventList = objIt->second;
                auto it = std::find(eventList.begin(), eventList.end(), eventId);
                if (it != eventList.end())
                {
                    std::swap(*it, eventList.back());
                    eventList.pop_back();
                }

                if (eventList.empty())
                    m_objectEvents.erase(objIt);
            }
        }

        // Remove event from main storage
        m_events.erase(eventIt);

        LOG_DEBUG("scripts.eclipse", "Removed event {}", eventId);
        return true;
    }

    void TimedEventManager::RemoveObjectEvents(ObjectGuid objectGuid)
    {
        auto it = m_objectEvents.find(objectGuid);
        if (it == m_objectEvents.end())
            return;  // No events for this object

        // Remove all events for this object from main storage
        for (uint64 eventId : it->second)
            m_events.erase(eventId);

        LOG_DEBUG("scripts.eclipse", "Removed {} events for object {}", it->second.size(), objectGuid.ToString());

        // Remove object entry from index
        m_objectEvents.erase(it);
    }

    void TimedEventManager::RemoveAllGlobalEvents()
    {
        // Remove all global events from main storage
        for (uint64 eventId : m_globalEvents)
            m_events.erase(eventId);

        LOG_DEBUG("scripts.eclipse", "Removed {} global events", m_globalEvents.size());

        m_globalEvents.clear();
    }

    void TimedEventManager::Update(uint32 diff)
    {
        // Early exit: No global events to update
        if (m_globalEvents.empty())
            return;

        // Clear removal buffer (reuse allocation from previous frame)
        m_eventsToRemove.clear();

        // Update only global events (object events updated via UpdateObjectEvents)
        for (uint64 eventId : m_globalEvents)
        {
            auto eventIt = m_events.find(eventId);
            if (eventIt == m_events.end())
                continue;  // Event was removed externally

            TimedEvent& event = eventIt->second;
            event.elapsed += diff;

            // Check if ready to execute
            if (event.elapsed >= event.delay)
            {
                ExecuteEvent(event, nullptr);  // nullptr = no object (global event)

                // Reset timer for next iteration
                event.elapsed = 0;

                // Handle repeat count
                if (event.repeats > 0)
                {
                    event.remainingRepeats--;
                    if (event.remainingRepeats == 0)
                        m_eventsToRemove.push_back(eventId);
                }
            }
        }

        // Batch remove expired events
        for (uint64 eventId : m_eventsToRemove)
            RemoveEvent(eventId);
    }

    void TimedEventManager::UpdateObjectEvents(WorldObject* obj, uint32 diff)
    {
        // Null check
        if (!obj)
            return;

        // Early exit: No events for this object
        ObjectGuid guid = obj->GetGUID();
        auto objEventsIt = m_objectEvents.find(guid);
        if (objEventsIt == m_objectEvents.end())
        {
            return;
        }

        // Clear removal buffer
        m_eventsToRemove.clear();

        // Update events for this specific object
        for (uint64 eventId : objEventsIt->second)
        {
            auto eventIt = m_events.find(eventId);
            if (eventIt == m_events.end())
            {
                continue;  // Event was removed externally
            }

            TimedEvent& event = eventIt->second;
            event.elapsed += diff;

            // Check if ready to execute
            if (event.elapsed >= event.delay)
            {
                ExecuteEvent(event, obj);  // Pass object to callback

                // Reset timer for next iteration
                event.elapsed = 0;

                // Handle repeat count
                if (event.repeats > 0)
                {
                    event.remainingRepeats--;
                    if (event.remainingRepeats == 0)
                    {
                        m_eventsToRemove.push_back(eventId);
                    }
                }
            }
        }

        // Batch remove expired events
        for (uint64 eventId : m_eventsToRemove)
        {
            RemoveEvent(eventId);
        }
    }

    void TimedEventManager::ExecuteEvent(TimedEvent& event, WorldObject* obj)
    {
        try
        {
            sol::protected_function_result result;

            // Dispatch based on object type
            switch (event.objectType)
            {
                case TimedEventObjectType::GLOBAL:
                    // Global event: callback(eventId, delay, repeats)
                    result = event.callback(event.id, event.delay, event.repeats);
                    break;

                case TimedEventObjectType::PLAYER:
                {
                    if (!obj)
                        return;

                    Player* player = obj->ToPlayer();
                    if (!player)
                        return;

                    // Player event: callback(eventId, delay, repeats, player)
                    result = event.callback(event.id, event.delay, event.repeats, player);
                    break;
                }

                case TimedEventObjectType::CREATURE:
                {
                    if (!obj)
                        return;

                    Creature* creature = obj->ToCreature();
                    if (!creature)
                        return;

                    // Creature event: callback(eventId, delay, repeats, creature)
                    result = event.callback(event.id, event.delay, event.repeats, creature);
                    break;
                }

                case TimedEventObjectType::GAMEOBJECT:
                {
                    if (!obj)
                        return;

                    GameObject* gameobject = obj->ToGameObject();
                    if (!gameobject)
                        return;

                    // GameObject event: callback(eventId, delay, repeats, gameobject)
                    result = event.callback(event.id, event.delay, event.repeats, gameobject);
                    break;
                }
            }

            // Check for Lua errors
            if (!result.valid())
            {
                sol::error err = result;
                LOG_ERROR("scripts.eclipse", "Event {} callback error: {}", event.id, err.what());
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("scripts.eclipse", "Event {} exception: {}", event.id, e.what());
        }
    }

    uint32 TimedEventManager::GetObjectEventCount(ObjectGuid objectGuid) const
    {
        auto it = m_objectEvents.find(objectGuid);
        return it != m_objectEvents.end() ? static_cast<uint32>(it->second.size()) : 0;
    }

    void TimedEventManager::Clear()
    {
        m_events.clear();
        m_objectEvents.clear();
        m_globalEvents.clear();
        m_eventsToRemove.clear();
        m_nextEventId = 1;

        LOG_DEBUG("scripts.eclipse", "Cleared all timed events (mapId={})", m_mapId);
    }

} // namespace Eclipse::Core
