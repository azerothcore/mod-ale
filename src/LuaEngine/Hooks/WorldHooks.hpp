/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_WORLD_HOOKS_H
#define _ALE_WORLD_HOOKS_H

#include "ScriptMgr.h"

#include "TimedEventManager.h"
#include "EventManager.h"
#include "Methods.hpp"

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
        auto eventType = static_cast<Core::WorldEvent>(eventId);
        uint32 executed = Core::EventManager::GetInstance().TriggerGlobalEvent(
            eventType,
            std::forward<Args>(args)...
        );

        if (executed > 0)
            LOG_DEBUG("ale.hooks", "[ALE] WorldObject event {} triggered ({} handlers executed)", eventId, executed);
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
                Methods::RegisterAllMethods(*masterState, -1, timedEventMgr);

                try
                {
                    // Start timing script loading
                    auto startTime = std::chrono::high_resolution_clock::now();

                    // Load all scripts from lua_scripts directory
                    LOG_INFO("ale", "[ALE] Scanning and loading scripts from lua_scripts directory...");
                    ALE::Core::ScriptLoader::GetInstance().SetScriptPath("lua_scripts");
                    // uint32 loadedCount =
                    ALE::Core::ScriptLoader::GetInstance().LoadAllScripts(-1);

                    // // Calculate elapsed time
                    // auto endTime = std::chrono::high_resolution_clock::now();
                    // auto durationMicro = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
                    // auto durationMilli = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

                    // // Get comprehensive statistics from centralized Statistics system
                    // auto stats = ALE::Statistics::ALEStatistics::GetInstance().GetSnapshot();

                    // // Display comprehensive statistics
                    // LOG_INFO("server.loading", "");
                    // LOG_INFO("server.loading", "========================================");
                    // LOG_INFO("server.loading", "[ALE] Script Loading Statistics");
                    // LOG_INFO("server.loading", "========================================");
                    // LOG_INFO("server.loading", "Scripts Discovered : {} total", stats.loadingTotalScripts);
                    // LOG_INFO("server.loading", "  - Ext Scripts    : {} (priority)", stats.loadingExtScripts);
                    // LOG_INFO("server.loading", "  - Cout Scripts   : {} (pre-compiled)", stats.loadingCoutScripts);
                    // LOG_INFO("server.loading", "  - Lua Scripts    : {}", stats.loadingLuaScripts);
                    // LOG_INFO("server.loading", "  - MoonScripts    : {}", stats.loadingMoonScripts);
                    // LOG_INFO("server.loading", "");
                    // LOG_INFO("server.loading", "Loading Results    :");
                    // LOG_INFO("server.loading", "  - Successful     : {} scripts", stats.loadingSuccessful);
                    // LOG_INFO("server.loading", "  - Failed         : {} scripts", stats.loadingFailed);
                    // LOG_INFO("server.loading", "");
                    // LOG_INFO("server.loading", "Bytecode Cache     :");
                    // LOG_INFO("server.loading", "  - Cached Scripts : {}", stats.cacheTotalScripts);
                    // LOG_INFO("server.loading", "  - Cache Hits     : {}", stats.cacheHits);
                    // LOG_INFO("server.loading", "  - Cache Misses   : {}", stats.cacheMisses);
                    // LOG_INFO("server.loading", "  - Total Size     : {} bytes ({:.2f} KB)",
                    //     stats.cacheTotalMemory,
                    //     stats.cacheTotalMemory / 1024.0);
                    // LOG_INFO("server.loading", "");
                    // LOG_INFO("server.loading", "Compilation        :");
                    // LOG_INFO("server.loading", "  - Total Success  : {}", stats.compilationSuccess);
                    // LOG_INFO("server.loading", "  - Total Failed   : {}", stats.compilationFailed);
                    // LOG_INFO("server.loading", "  - Ext Compiled   : {}", stats.compilationExtFiles);
                    // LOG_INFO("server.loading", "  - Cout Loaded    : {}", stats.compilationCoutFiles);
                    // LOG_INFO("server.loading", "  - Moon Compiled  : {}", stats.compilationMoonScript);
                    // LOG_INFO("server.loading", "  - Bytecode Size  : {} bytes", stats.compilationTotalBytecodeSize);
                    // LOG_INFO("server.loading", "");
                    // LOG_INFO("server.loading", "Performance        :");
                    // LOG_INFO("server.loading", "  - Load Time      : {} µs ({} ms)", durationMicro, durationMilli);
                    // if (stats.loadingSuccessful > 0)
                    // {
                    //     LOG_INFO("server.loading", "  - Avg per Script : {:.2f} µs ({:.2f} ms)",
                    //         static_cast<double>(durationMicro) / stats.loadingSuccessful,
                    //         static_cast<double>(durationMilli) / stats.loadingSuccessful);
                    // }
                    // LOG_INFO("server.loading", "========================================");
                    // LOG_INFO("server.loading", "");

                    // if (stats.loadingFailed > 0)
                    // {
                    //     LOG_WARN("server.loading", "[ALE] Warning: {} script(s) failed to load. Check logs above for details.", 
                    //         stats.loadingFailed);
                    // }
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
    };
} // namespace ALE

#endif // _ALE_WORLD_OBJECT_HOOKS_H
