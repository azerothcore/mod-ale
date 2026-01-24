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
#include "Methods.hpp"
#include "Statistics.h"
#include "ALEScriptLoader.h"

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

        /**
         * @todo Document all overridden hooks
         */

        void OnBeforeConfigLoad(bool reload) override
        {
            if (!reload)
            {
                // Initialize the StateManager first
                if (!ALE::Core::StateManager::GetInstance().Initialize())
                {
                    LOG_ERROR("ale", "[ALE] Failed to initialize StateManager!");
                    return;
                }

                // Get master state
                sol::state* masterState = ALE::Core::StateManager::GetInstance().GetMasterState();
                if (!masterState)
                {
                    LOG_ERROR("ale", "[ALE] Failed to create master state!");
                    return;
                }

                // Initialize EventManager
                if (!ALE::Core::EventManager::GetInstance().Initialize())
                {
                    LOG_ERROR("ale", "[ALE] Failed to initialize EventManager!");
                    return;
                }

                // Register all Lua methods directly
                ALE::Core::TimedEventManager* timedEventMgr = ALE::Core::StateManager::GetInstance().GetTimedEventManager(-1);
                Methods::RegisterAllMethods(*masterState, timedEventMgr);

                try
                {
                    // Start timing script loading
                    auto startTime = std::chrono::high_resolution_clock::now();

                    // Load all scripts from lua_scripts directory
                    LOG_INFO("server.loading", "[ALE] Scanning and loading scripts from lua_scripts directory...");
                    ALE::Core::ScriptLoader::GetInstance().SetScriptPath("lua_scripts");
                    ALE::Core::ScriptLoader::GetInstance().LoadAllScripts(-1);

                }
                catch (const sol::error& e)
                {
                    LOG_ERROR("server.loading", "[ALE] Initialization failed: {}", e.what());
                }
            }
        }

        void OnUpdate(uint32 diff) override
        {
            auto& stateMgr = Core::StateManager::GetInstance();
            auto stateIds = stateMgr.GetAllStateIds();

            Core::TimedEventManager* mgr = stateMgr.GetTimedEventManager(-1);
            if (mgr)
                mgr->Update(diff);
        }

        void OnStartup() override
        {
            TriggerWorldEvent(static_cast<uint32>(Hooks::WorldEvent::ON_STARTUP));
        }
    };
} // namespace ALE

#endif // _ALE_WORLD_OBJECT_HOOKS_H
