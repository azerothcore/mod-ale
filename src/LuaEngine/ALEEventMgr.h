/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_EVENT_MGR_H
#define _ALE_EVENT_MGR_H

#include "ALEUtility.h"
#include "Common.h"
#include "Util.h"

#include <sol/sol.hpp>

#include <map>
#include <unordered_map>
#include <unordered_set>

class ALE;
class EventMgr;
class ALEEventProcessor;
class WorldObject;

enum LuaEventState
{
    LUAEVENT_STATE_RUN,    // On next call run the function normally
    LUAEVENT_STATE_ABORT,  // On next call unregisters the callback and erases the data
    LUAEVENT_STATE_ERASE,  // On next call just erases the data
};

/*
 * A timed Lua callback registered with CreateLuaEvent or object:RegisterEvent.
 * The callback keeps itself alive in the Lua registry for as long as the
 * event exists.
 */
struct LuaEvent
{
    LuaEvent(uint64 _id, sol::protected_function _callback, uint32 _min, uint32 _max, uint32 _repeats) :
        min(_min), max(_max), delay(0), repeats(_repeats), id(_id), callback(std::move(_callback)), state(LUAEVENT_STATE_RUN)
    {
    }

    void SetState(LuaEventState _state)
    {
        if (state != LUAEVENT_STATE_ERASE)
            state = _state;
    }

    void GenerateDelay()
    {
        delay = urand(min, max);
    }

    uint32 min;   // Minimum delay between event calls
    uint32 max;   // Maximum delay between event calls
    uint32 delay; // The currently used waiting time
    uint32 repeats; // Amount of repeats to make, 0 for infinite
    uint64 id;      // Event ID, used to remove the event from Lua
    sol::protected_function callback;
    LuaEventState state;    // State for next call
};

class ALEEventProcessor
{
    friend class EventMgr;

public:
    typedef std::multimap<uint64, LuaEvent*> EventList;
    typedef std::unordered_map<uint64, LuaEvent*> EventMap;

    ALEEventProcessor(ALE** _E, WorldObject* _obj);
    ~ALEEventProcessor();

    void Update(uint32 diff);
    // removes all timed events on next tick or at tick end
    void SetStates(LuaEventState state);
    // set the event to be removed when executing
    void SetState(uint64 eventId, LuaEventState state);
    // registers the callback and returns its event ID
    uint64 AddEvent(sol::protected_function callback, uint32 min, uint32 max, uint32 repeats);
    EventMap eventMap;

private:
    void RemoveEvents_internal();
    void AddEvent(LuaEvent* luaEvent);
    EventList eventList;
    uint64 m_time = 0;
    WorldObject* obj;
    ALE** E;
};

class EventMgr : public ALEUtil::Lockable
{
public:
    typedef std::unordered_set<ALEEventProcessor*> ProcessorSet;
    ProcessorSet processors;
    ALEEventProcessor* globalProcessor;
    ALE** E;

    EventMgr(ALE** _E);
    ~EventMgr();

    // Returns a new unique id for a timed event.
    uint64 NextEventId() { return ++maxEventId; }

    // Set the state of all timed events
    // Execute only in safe env
    void SetStates(LuaEventState state);

    // Sets the eventId's state in all processors
    // Execute only in safe env
    void SetState(uint64 eventId, LuaEventState state);

private:
    uint64 maxEventId = 0;
};

#endif // _ALE_EVENT_MGR_H
