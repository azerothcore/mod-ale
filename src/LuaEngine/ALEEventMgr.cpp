/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEEventMgr.h"
#include "LuaEngine.h"
#include "Object.h"

ALEEventProcessor::ALEEventProcessor(ALE** _E, WorldObject* _obj) : obj(_obj), E(_E)
{
    // can be called from multiple threads
    if (obj)
    {
        EventMgr::Guard guard((*E)->eventMgr->GetLock());
        (*E)->eventMgr->processors.insert(this);
    }
}

ALEEventProcessor::~ALEEventProcessor()
{
    // can be called from multiple threads
    {
        LOCK_ALE;
        RemoveEvents_internal();
    }

    if (obj && ALE::IsInitialized())
    {
        EventMgr::Guard guard((*E)->eventMgr->GetLock());
        (*E)->eventMgr->processors.erase(this);
    }
}

void ALEEventProcessor::Update(uint32 diff)
{
    m_time += diff;
    for (EventList::iterator it = eventList.begin(); it != eventList.end() && it->first <= m_time; it = eventList.begin())
    {
        LuaEvent* luaEvent = it->second;
        eventList.erase(it);

        if (luaEvent->state != LUAEVENT_STATE_ERASE)
            eventMap.erase(luaEvent->id);

        if (luaEvent->state == LUAEVENT_STATE_RUN)
        {
            uint32 delay = luaEvent->delay;
            bool remove = luaEvent->repeats == 1;
            if (!remove)
                AddEvent(luaEvent); // Reschedule before calling incase RemoveEvents used

            // Call the timed event
            (*E)->OnTimedEvent(luaEvent->callback, luaEvent->id, delay, luaEvent->repeats ? luaEvent->repeats-- : luaEvent->repeats, obj);

            if (!remove)
                continue;
        }

        // Event should be deleted (executed last time or set to be aborted)
        delete luaEvent;
    }
}

void ALEEventProcessor::SetStates(LuaEventState state)
{
    for (auto& [time, luaEvent] : eventList)
        luaEvent->SetState(state);

    if (state == LUAEVENT_STATE_ERASE)
        eventMap.clear();
}

void ALEEventProcessor::RemoveEvents_internal()
{
    for (auto& [time, luaEvent] : eventList)
        delete luaEvent;

    eventList.clear();
    eventMap.clear();
}

void ALEEventProcessor::SetState(uint64 eventId, LuaEventState state)
{
    auto iter = eventMap.find(eventId);
    if (iter != eventMap.end())
        iter->second->SetState(state);

    if (state == LUAEVENT_STATE_ERASE)
        eventMap.erase(eventId);
}

void ALEEventProcessor::AddEvent(LuaEvent* luaEvent)
{
    luaEvent->GenerateDelay();
    eventList.insert({ m_time + luaEvent->delay, luaEvent });
    eventMap[luaEvent->id] = luaEvent;
}

uint64 ALEEventProcessor::AddEvent(sol::protected_function callback, uint32 min, uint32 max, uint32 repeats)
{
    uint64 id = (*E)->eventMgr->NextEventId();
    AddEvent(new LuaEvent(id, std::move(callback), min, max, repeats));
    return id;
}

EventMgr::EventMgr(ALE** _E) : globalProcessor(new ALEEventProcessor(_E, nullptr)), E(_E)
{
}

EventMgr::~EventMgr()
{
    {
        Guard guard(GetLock());
        for (ALEEventProcessor* processor : processors)
            processor->RemoveEvents_internal();
        globalProcessor->RemoveEvents_internal();
    }
    delete globalProcessor;
    globalProcessor = nullptr;
}

void EventMgr::SetStates(LuaEventState state)
{
    Guard guard(GetLock());
    for (ALEEventProcessor* processor : processors)
        processor->SetStates(state);
    globalProcessor->SetStates(state);
}

void EventMgr::SetState(uint64 eventId, LuaEventState state)
{
    Guard guard(GetLock());
    for (ALEEventProcessor* processor : processors)
        processor->SetState(eventId, state);
    globalProcessor->SetState(eventId, state);
}
