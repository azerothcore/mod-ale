/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEIncludes.h"
#include "ALETemplate.h"

using namespace Hooks;

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<TicketEvents>(EVENT);\
    if (!TicketEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK(EVENT) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EventKey<TicketEvents>(EVENT);\
    if (!TicketEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

void ALE::OnTicketCreate(GmTicket* ticket)
{
    START_HOOK(TICKET_EVENT_ON_CREATE);
    Push(ticket);
    CallAllFunctions(TicketEventBindings, key);
}

void ALE::OnTicketUpdateLastChange(GmTicket* ticket)
{
    START_HOOK(TICKET_EVENT_UPDATE_LAST_CHANGE);
    Push(ticket);
    CallAllFunctions(TicketEventBindings, key);
}

void ALE::OnTicketClose(GmTicket* ticket)
{
    START_HOOK(TICKET_EVENT_ON_CLOSE);
    Push(ticket);
    CallAllFunctions(TicketEventBindings, key);
}

void ALE::OnTicketResolve(GmTicket* ticket)
{
    START_HOOK(TICKET_EVENT_ON_RESOLVE);
    Push(ticket);
    CallAllFunctions(TicketEventBindings, key);
}

