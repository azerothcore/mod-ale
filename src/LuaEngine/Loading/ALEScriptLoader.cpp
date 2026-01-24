/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ALEScriptLoader.h"
#include "StateManager.h"
#include "FileSystemUtils.h"
#include "ScriptCompiler.h"
#include "BytecodeCache.h"
#include "Statistics.h"
#include "Log.h"
#include <boost/filesystem.hpp>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = boost::filesystem;

namespace ALE::Core
{
    /**
     * @brief Get ScriptLoader singleton instance
     * 
     * Thread-safe initialization (C++11 magic statics).
     * 
     * @return Reference to the global ScriptLoader
     */
    ScriptLoader& ScriptLoader::GetInstance()
    {
        static ScriptLoader instance;
        return instance;
    }

    ScriptLoader::ScriptLoader()
        : m_scriptPath("lua_scripts")
    {
        LOG_DEBUG("ale.loader", "[ALE] ScriptLoader initialized");
    }

    /**
     * @brief Destructor
     */
    ScriptLoader::~ScriptLoader()
    {
    }

    /**
     * @brief Set script directory path
     * 
     * Changes the root directory for script scanning.
     * 
     * @param path Path to lua_scripts directory
     */
    void ScriptLoader::SetScriptPath(const std::string& path)
    {
        m_scriptPath = path;
        LOG_INFO("ale.loader", "[ALE] ScriptLoader - Script path set to: {}", path);
    }


    void ScriptLoader::ScanDirectory(const std::string& directory, std::vector<ScriptFile>& scripts)
    {
        try
        {
            if (!fs::exists(directory))
            {
                LOG_WARN("ale.loader", "[ALE] Directory does not exist: {}", directory);
                return;
            }

            fs::recursive_directory_iterator end_iter;
            for (fs::recursive_directory_iterator dir_iter(directory); dir_iter != end_iter; ++dir_iter)
            {
                std::string filepath = dir_iter->path().generic_string();

                // Skip hidden files
#ifdef _WIN32
                DWORD dwAttrib = GetFileAttributes(filepath.c_str());
                if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_HIDDEN))
                    continue;
#else
                std::string name = dir_iter->path().filename().generic_string();
                if (!name.empty() && name[0] == '.')
                    continue;
#endif

                if (fs::is_regular_file(dir_iter->status()))
                {
                    if (Utils::FileSystemUtils::IsScriptFile(filepath))
                    {
                        std::string filename = dir_iter->path().filename().generic_string();
                        std::string extension = Utils::FileSystemUtils::GetExtension(filepath);
                        scripts.emplace_back(filename, filepath, extension);
                    }
                }
            }
        }
        catch (const fs::filesystem_error& e)
        {
            LOG_ERROR("ale.loader", "[ALE] Filesystem error: {}", e.what());
        }
    }

    std::vector<ScriptFile> ScriptLoader::ScanScripts()
    {
        LOG_INFO("ale.loader", "[ALE] Scanning directory: {}", m_scriptPath);

        std::vector<ScriptFile> scripts;
        ScanDirectory(m_scriptPath, scripts);

        // Sort by priority: .ext > .cout > .moon > .lua
        std::sort(scripts.begin(), scripts.end());

        // Update statistics
        uint32 extScripts = 0, coutScripts = 0, luaScripts = 0, moonScripts = 0;
        for (const auto& script : scripts)
        {
            if (script.extension == ".ext") extScripts++;
            else if (script.extension == ".cout") coutScripts++;
            else if (script.extension == ".lua") luaScripts++;
            else if (script.extension == ".moon") moonScripts++;
        }

        auto& stats = Statistics::ALEStatistics::GetInstance();
        stats.SetLoadingExtScripts(extScripts);
        stats.SetLoadingCoutScripts(coutScripts);
        stats.SetLoadingLuaScripts(luaScripts);
        stats.SetLoadingMoonScripts(moonScripts);
        stats.SetLoadingTotalScripts(scripts.size());

        LOG_INFO("ale.loader", "[ALE] Found {} scripts ({} Ext, {} Cout, {} Lua, {} Moon)",
            scripts.size(), extScripts, coutScripts, luaScripts, moonScripts);

        return scripts;
    }

    void ScriptLoader::SetupRequirePaths(sol::state& state)
    {
        if (!fs::exists(m_scriptPath))
            return;

        std::string luaPath;
        std::string luaCPath;

        try
        {
            fs::recursive_directory_iterator end_iter;
            for (fs::recursive_directory_iterator dir_iter(m_scriptPath); dir_iter != end_iter; ++dir_iter)
            {
                if (fs::is_directory(dir_iter->status()))
                {
                    std::string dirPath = dir_iter->path().generic_string();
                    luaPath += dirPath + "/?.lua;" + dirPath + "/?.moon;" + dirPath + "/?.ext;";
#ifdef _WIN32
                    luaCPath += dirPath + "/?.dll;";
#else
                    luaCPath += dirPath + "/?.so;";
#endif
                }
            }

            // Add base path
            luaPath = m_scriptPath + "/?.lua;" + m_scriptPath + "/?.moon;" + m_scriptPath + "/?.ext;" + luaPath;
#ifdef _WIN32
            luaCPath = m_scriptPath + "/?.dll;" + luaCPath;
#else
            luaCPath = m_scriptPath + "/?.so;" + luaCPath;
#endif

            // Append to existing paths
            sol::optional<std::string> currentPath = state["package"]["path"];
            if (currentPath)
                luaPath += *currentPath;

            sol::optional<std::string> currentCPath = state["package"]["cpath"];
            if (currentCPath)
                luaCPath += *currentCPath;

            state["package"]["path"] = luaPath;
            state["package"]["cpath"] = luaCPath;

            LOG_DEBUG("ale.loader", "[ALE] Configured require paths");
        }
        catch (const fs::filesystem_error& e)
        {
            LOG_ERROR("ale.loader", "[ALE] Error setting up require paths: {}", e.what());
        }
    }

    bool ScriptLoader::LoadScript(const ScriptFile& scriptFile, int32 stateId)
    {
        LOG_DEBUG("ale.loader", "[ALE] Loading: {}", scriptFile.filepath);

        auto& cache = Core::BytecodeCache::GetInstance();
        auto& stats = Statistics::ALEStatistics::GetInstance();

        // 1. Check cache
        const auto* cached = cache.Get(scriptFile.filepath);
        if (cached)
        {
            if (ExecuteBytecode(cached, scriptFile.filename, stateId))
            {
                stats.IncrementLoadingSuccessful();
                return true;
            }
            stats.IncrementLoadingFailed();
            return false;
        }

        // 2. Compile (cache miss)
        auto bytecode = API::ScriptCompiler::GetInstance().Compile(scriptFile.filepath);
        if (!bytecode || !bytecode->isValid())
        {
            LOG_ERROR("ale.loader", "[ALE] Failed to compile: {}", scriptFile.filepath);
            stats.IncrementCompilationFailed();
            stats.IncrementLoadingFailed();
            return false;
        }

        // 3. Store in cache
        cache.Store(scriptFile.filepath, bytecode);
        stats.IncrementCompilationSuccess();
        stats.AddCompilationBytecodeSize(bytecode->size());

        // 4. Execute
        if (ExecuteBytecode(bytecode.get(), scriptFile.filename, stateId))
        {
            LOG_INFO("ale.loader", "[ALE] Loaded: {}", scriptFile.filename);
            stats.IncrementLoadingSuccessful();
            return true;
        }

        stats.IncrementLoadingFailed();
        return false;
    }

    bool ScriptLoader::ExecuteBytecode(const CompiledBytecode* bytecode, const std::string& filename, int32 stateId)
    {
        sol::state* state = StateManager::GetInstance().GetOrCreateState(stateId);
        if (!state)
        {
            LOG_ERROR("ale.loader", "[ALE] Failed to get state {}", stateId);
            return false;
        }

        try
        {
            auto result = state->load(bytecode->bytecode.as_string_view(), filename);
            if (!result.valid())
            {
                sol::error err = result;
                LOG_ERROR("ale.loader", "[ALE] Failed to load bytecode for {}: {}", filename, err.what());
                return false;
            }

            sol::protected_function scriptFunc = result;
            auto execResult = scriptFunc();
            if (!execResult.valid())
            {
                sol::error err = execResult;
                LOG_ERROR("ale.loader", "[ALE] Execution error in {}: {}", filename, err.what());
                return false;
            }

            return true;
        }
        catch (const sol::error& e)
        {
            LOG_ERROR("ale.loader", "[ALE] Exception executing {}: {}", filename, e.what());
            return false;
        }
    }

    uint32 ScriptLoader::LoadAllScripts(int32 stateId)
    {
        LOG_INFO("ale.loader", "[ALE] Loading all scripts into state {}", stateId);

        sol::state* state = StateManager::GetInstance().GetOrCreateState(stateId);
        if (!state)
        {
            LOG_ERROR("ale.loader", "[ALE] Failed to get state {}", stateId);
            return 0;
        }

        SetupRequirePaths(*state);

        auto scripts = ScanScripts();
        if (scripts.empty())
        {
            LOG_WARN("ale.loader", "[ALE] No scripts found in: {}", m_scriptPath);
            return 0;
        }

        uint32 successCount = 0;
        for (const auto& script : scripts)
        {
            if (LoadScript(script, stateId))
                successCount++;
        }

        auto stats = Statistics::ALEStatistics::GetInstance().GetSnapshot();
        LOG_INFO("ale.loader", "[ALE] Loaded {}/{} scripts ({} failed)",
            stats.loadingSuccessful, scripts.size(), stats.loadingFailed);

        return successCount;
    }

    uint32 ScriptLoader::ReloadAllScripts(int32 stateId)
    {
        LOG_INFO("ale.loader", "[ALE] Reloading all scripts for state {}", stateId);

        // Clear cache to force recompilation
        Core::BytecodeCache::GetInstance().ClearAll();

        // Remove and recreate state if per-map
        if (stateId >= 0)
            StateManager::GetInstance().RemoveState(stateId);

        return LoadAllScripts(stateId);
    }

} // namespace ALE::Core
