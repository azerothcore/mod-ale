/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "TimedEventManager.h"
#include "LuaGuard.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "Log.h"

namespace ALE::Core
{
    TimedEventManager::TimedEventManager(int32 mapId) : m_nextEventId(1), m_mapId(mapId)
    {
    }

    TimedEventManager::~TimedEventManager()
    {
        Clear();
    }

    uint64 TimedEventManager::RegisterGlobalEvent(sol::protected_function&& callback, uint32 delay, uint32 repeats)
    {
        uint64 eventId = m_nextEventId++;

        m_events.emplace(eventId, TimedEvent(eventId, std::move(callback), delay, repeats));
        m_globalEvents.insert(eventId);

        LOG_DEBUG("scripts.ale", "Registered global event {} (delay={}ms, repeats={})",
                  eventId, delay, repeats);

        return eventId;
    }

    uint64 TimedEventManager::RegisterObjectEvent(ObjectGuid objectGuid, sol::protected_function&& callback, uint32 delay, uint32 repeats, TimedEventObjectType objectType)
    {
        uint64 eventId = m_nextEventId++;

        m_events.emplace(eventId, TimedEvent(eventId, std::move(callback), delay, repeats, objectGuid, objectType));

        m_objectEvents[objectGuid].insert(eventId);

        LOG_DEBUG("scripts.ale", "Registered object event {} for GUID {} (type={}, delay={}ms, repeats={})",
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
            m_globalEvents.erase(eventId);
        }
        else
        {
            auto objIt = m_objectEvents.find(event.objectGuid);
            if (objIt != m_objectEvents.end())
            {
                objIt->second.erase(eventId);

                // Clean up empty entry
                if (objIt->second.empty())
                    m_objectEvents.erase(objIt);
            }
        }

        // Remove event from main storage
        m_events.erase(eventIt);

        LOG_DEBUG("scripts.ale", "Removed event {}", eventId);
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

        LOG_DEBUG("scripts.ale", "Removed {} events for object {}", it->second.size(), objectGuid.ToString());

        // Remove object entry from index
        m_objectEvents.erase(it);
    }

    void TimedEventManager::RemoveAllGlobalEvents()
    {
        // Remove all global events from main storage
        for (uint64 eventId : m_globalEvents)
            m_events.erase(eventId);

        LOG_DEBUG("scripts.ale", "Removed {} global events", m_globalEvents.size());

        m_globalEvents.clear();
    }

    template<typename Container>
    void TimedEventManager::UpdateEventList(const Container& eventIds, WorldObject* obj, uint32 diff)
    {
        // Collect events to remove
        std::vector<uint64> eventsToRemove;

        for (uint64 eventId : eventIds)
        {
            auto eventIt = m_events.find(eventId);
            if (eventIt == m_events.end())
                continue;  // Event was removed externally

            TimedEvent& event = eventIt->second;
            event.elapsed += diff;

            // Check if ready to execute
            if (event.elapsed >= event.delay)
            {
                ExecuteEvent(event, obj);

                // Reset timer for next iteration
                event.elapsed = 0;

                // Handle repeat count
                if (event.repeats > 0)
                {
                    event.remainingRepeats--;
                    if (event.remainingRepeats == 0)
                        eventsToRemove.push_back(eventId);
                }
            }
        }

        // Batch remove expired events
        for (uint64 eventId : eventsToRemove)
            RemoveEvent(eventId);
    }

    void TimedEventManager::Update(uint32 diff)
    {
        if (m_globalEvents.empty())
            return;

        UpdateEventList(m_globalEvents, nullptr, diff);
    }

    void TimedEventManager::UpdateObjectEvents(WorldObject* obj, uint32 diff)
    {
        if (!obj)
            return;

        auto objEventsIt = m_objectEvents.find(obj->GetGUID());
        if (objEventsIt == m_objectEvents.end())
            return;

        UpdateEventList(objEventsIt->second, obj, diff);
    }

    void TimedEventManager::ExecuteEvent(TimedEvent& event, WorldObject* obj)
    {
        LuaGuard guard;

        try
        {
            sol::protected_function_result result;

            auto executeTypedEvent = [&](auto* typedObj) {
                if (!obj || !typedObj)
                    return;
                result = event.callback(event.id, event.delay, event.repeats, typedObj);
            };

            // Dispatch based on object type
            switch (event.objectType)
            {
                case TimedEventObjectType::GLOBAL:
                    result = event.callback(event.id, event.delay, event.repeats);
                    break;

                case TimedEventObjectType::PLAYER:
                    executeTypedEvent(obj->ToPlayer());
                    break;

                case TimedEventObjectType::CREATURE:
                    executeTypedEvent(obj->ToCreature());
                    break;

                case TimedEventObjectType::GAMEOBJECT:
                    executeTypedEvent(obj->ToGameObject());
                    break;
            }

            // Check for Lua errors
            if (!result.valid())
            {
                sol::error err = result;
                LOG_ERROR("scripts.ale", "Event {} callback error: {}", event.id, err.what());
            }
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("scripts.ale", "Event {} exception: {}", event.id, e.what());
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
        m_nextEventId = 1;

        LOG_DEBUG("scripts.ale", "Cleared all timed events (mapId={})", m_mapId);
    }

} // namespace ALE::Core
