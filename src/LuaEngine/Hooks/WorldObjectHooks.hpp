/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_WORLD_OBJECT_HOOKS_H
#define _ALE_WORLD_OBJECT_HOOKS_H

#include "ScriptMgr.h"

#include "TimedEventManager.h"
#include "EventManager.h"

namespace ALE::Hooks
{
    /**
     * @brief Trigger WorldObject event helper (template implementation)
     *
     * Converts enum to uint32 and forwards to EventManager.
     *
     * @tparam Args Variadic argument types
     * @param eventType WorldObjectEvent enum value
     * @param args Arguments to forward to Lua callbacks
     */
    template<typename... Args>
    void TriggerWorldObjectEvent(Core::WorldObjectEvent eventType, Args&&... args)
    {
        // Cast enum to uint32
        uint32 eventId = static_cast<uint32>(eventType);

        // Trigger event
        uint32 executed = Core::EventManager::GetInstance().TriggerGlobalEvent(
            eventType,
            eventId,
            std::forward<Args>(args)...
        );
    }

    class WorldObjectHooks : public WorldObjectScript
    {
    public:
        WorldObjectHooks() : WorldObjectScript("ALE_WorldObjectHooks") { }

        /**
         * todo: Document all overridden hooks
         */
        void OnWorldObjectUpdate(WorldObject* object, uint32 diff) override
        {
            if (!object || !object->IsInWorld())
                return;

            auto& stateMgr = Core::StateManager::GetInstance();
            Core::TimedEventManager* mgr = stateMgr.GetTimedEventManager(-1);

            // Execute timed events for this WorldObject
            if (mgr)
                mgr->UpdateObjectEvents(object, diff);
        }
    };
} // namespace ALE

#endif // _ALE_WORLD_OBJECT_HOOKS_H
