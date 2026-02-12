/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ALEManager.h"
#include "ALEConfig.h"
#include "StateManager.h"
#include "EventManager.h"
#include "ALEScriptLoader.h"
#include "Methods.hpp"
#include "Statistics.h"
#include "Log.h"
#include <chrono>

namespace ALE::Core
{
    namespace
    {
        std::string FormatBytes(size_t bytes)
        {
            if (bytes < 1024)
                return std::to_string(bytes) + "B";
            if (bytes < 1024 * 1024)
                return std::to_string(bytes / 1024) + "KB";
            return std::to_string(bytes / (1024 * 1024)) + "MB";
        }
    }

    ALEManager::ALEManager() = default;

    ALEManager& ALEManager::GetInstance()
    {
        static ALEManager instance;
        return instance;
    }

    bool ALEManager::Initialize()
    {
        if (m_initialized)
            return true;

        ALEConfig::GetInstance().Initialize(false);
        if (!ALEConfig::GetInstance().IsALEEnabled())
        {
            LOG_INFO("server.ale", "[ALE] ALE is disabled in configuration.");
            return false;
        }

        if (!EventManager::GetInstance().Initialize())
        {
            LOG_ERROR("server.ale", "[ALE] Failed to initialize EventManager!");
            return false;
        }

        if (!InitializeCore())
            return false;

        if (!LoadScripts())
        {
            Shutdown();
            return false;
        }

        m_initialized = true;
        return true;
    }

    void ALEManager::Shutdown()
    {
        if (!m_initialized)
            return;

        EventManager::GetInstance().Shutdown();
        StateManager::GetInstance().Shutdown();

        m_initialized = false;
    }

    bool ALEManager::Reload()
    {
        if (!m_initialized)
        {
            LOG_ERROR("server.ale", "[ALE] ALE not initialized.");
            return false;
        }

        LOG_INFO("server.ale", "[ALE] Reloading ALE Engine...");

        EventManager::GetInstance().CancelAllEvents();
        StateManager::GetInstance().Shutdown();

        if (!InitializeCore())
        {
            m_initialized = false;
            return false;
        }

        if (!LoadScripts())
            return false;

        return true;
    }

    bool ALEManager::InitializeCore()
    {
        if (!StateManager::GetInstance().Initialize())
        {
            LOG_ERROR("server.ale", "[ALE] Failed to initialize StateManager!");
            return false;
        }

        sol::state* masterState = StateManager::GetInstance().GetMasterState();
        if (!masterState)
        {
            LOG_ERROR("server.ale", "[ALE] Failed to get master state!");
            StateManager::GetInstance().Shutdown();
            return false;
        }

        TimedEventManager* timedEventMgr = StateManager::GetInstance().GetTimedEventManager(-1);
        if (!timedEventMgr)
        {
            LOG_ERROR("server.ale", "[ALE] Failed to get TimedEventManager!");
            StateManager::GetInstance().Shutdown();
            return false;
        }

        ALE::Methods::RegisterAllMethods(*masterState, timedEventMgr);
        return true;
    }

    bool ALEManager::LoadScripts()
    {
        try
        {
            auto scriptPath = ALEConfig::GetInstance().GetScriptPath();
            ScriptLoader::GetInstance().SetScriptPath(std::string(scriptPath));

            ALE::Statistics::ALEStatistics::GetInstance().ResetLoadStats();

            auto startTime = std::chrono::steady_clock::now();
            uint32 loadedCount = ScriptLoader::GetInstance().LoadAllScripts(-1);
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            auto stats = ALE::Statistics::ALEStatistics::GetInstance().GetSnapshot();
            LOG_INFO("server.ale", "[ALE] Loaded {} scripts in {}ms ({} compiled, {} cached, {})",
                loadedCount, duration, stats.compilationSuccess, stats.cacheHits, FormatBytes(stats.compilationTotalBytecodeSize));

            return true;
        }
        catch (const sol::error& e)
        {
            LOG_ERROR("server.ale", "[ALE] Script loading failed: {}", e.what());
            return false;
        }
    }

    bool ALEManager::ReloadConfig()
    {
        ALEConfig::GetInstance().Initialize(true);
        return true;
    }

    bool ALEManager::IsEnabled() const
    {
        return ALEConfig::GetInstance().IsALEEnabled();
    }

} // namespace ALE::Core
