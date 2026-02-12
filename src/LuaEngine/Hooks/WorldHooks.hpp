/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_WORLD_HOOKS_H
#define _ALE_WORLD_HOOKS_H

#include "ScriptMgr.h"
#include <chrono>

#include "TimedEventManager.h"
#include "EventManager.h"
#include "ALEManager.h"
#include "ALEConfig.h"

namespace ALE::Hooks
{
    /**
     * @brief Trigger Server/World event helper (template implementation)
     *
     * Converts enum to uint32 and forwards to EventManager.
     *
     * @tparam Args Variadic argument types
     * @param eventType WorldEvent enum value
     * @param args Arguments to forward to Lua callbacks
     */
    template<typename... Args>
    void TriggerWorldEvent(uint32 eventId, Args&&... args)
    {
        auto eventType = static_cast<Hooks::WorldEvent>(eventId);
        Core::EventManager::GetInstance().TriggerGlobalEvent(
            eventType,
            std::forward<Args>(args)...
        );
    }

    /**
     * @class WorldHooks
     * @brief AzerothCore world event interceptor for ALE Lua system
     * @note Registered in ALEScriptLoader via sScriptMgr->AddScript<WorldHooks>()
     */
    class WorldHooks : public WorldScript
    {
    public:
        /**
         * @brief Constructor - registers WorldHooks with AzerothCore
         */
        WorldHooks() : WorldScript("ALE_WorldHooks") { }

        void OnBeforeConfigLoad(bool reload) override
        {
            if (!reload)
            {
                ALE::Core::ALEManager::GetInstance().Initialize();
            }
            else
            {
                ALE::Core::ALEManager::GetInstance().ReloadConfig();
            }
        }

        void OnUpdate(uint32 diff) override
        {
            auto& stateMgr = Core::StateManager::GetInstance();

            Core::TimedEventManager* mgr = stateMgr.GetTimedEventManager(-1);
            if (mgr)
                mgr->Update(diff);
        }

        void OnStartup() override
        {
            TriggerWorldEvent(static_cast<uint32>(Hooks::WorldEvent::ON_STARTUP));
        }

        void OnShutdown() override
        {
            TriggerWorldEvent(static_cast<uint32>(Hooks::WorldEvent::ON_SHUTDOWN));
            ALE::Core::ALEManager::GetInstance().Shutdown();
        }
    };
} // namespace ALE::Hooks

#endif // _ALE_WORLD_OBJECT_HOOKS_H
