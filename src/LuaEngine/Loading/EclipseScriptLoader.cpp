/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "EclipseScriptLoader.h"
#include "StateManager.h"
#include "ScriptCompiler.h"
#include "Log.h"
#include <boost/filesystem.hpp>
#include <algorithm>  // For std::sort

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = boost::filesystem;

namespace Eclipse::Core
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

    /**
     * @brief Constructor - initializes default script path
     */
    ScriptLoader::ScriptLoader()
        : m_scriptPath("lua_scripts")
    {
        LOG_DEBUG("server.loading", "[Eclipse] ScriptLoader - Creating script loader");
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
        LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Script path set to: {}", path);
    }

    /**
     * @brief Check if file is a valid script file
     * 
     * Accepts: .ext, .lua, .moon, .cout
     * 
     * @param filename Filename to check
     * @param extension Output: extracted extension if valid
     * @return true if valid script extension
     */
    bool ScriptLoader::IsScriptFile(const std::string& filename, std::string& extension)
    {
        size_t dotPos = filename.rfind('.');
        if (dotPos == std::string::npos)
            return false;

        extension = filename.substr(dotPos);
        return extension == ".ext" || extension == ".lua" || extension == ".moon" || extension == ".cout";
    }

    sol::state* ScriptLoader::GetStateOrNull(int32 stateId, const char* callerName)
    {
        sol::state* state = nullptr;

        if (stateId == -1)
            state = StateManager::GetInstance().GetMasterState();
        else
            state = StateManager::GetInstance().GetOrCreateState(stateId);

        if (!state)
        {
            LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader::{} - Failed to get state {}",
                callerName, stateId);
            return nullptr;
        }

        return state;
    }

    /**
     * @brief Recursively scan directory for scripts
     *
     * Uses boost::filesystem::recursive_directory_iterator.
     * Skips hidden files (Windows: FILE_ATTRIBUTE_HIDDEN, Unix: starts with '.').
     *
     * @param directory Directory to scan
     * @param scripts Output vector (scripts appended)
     */
    void ScriptLoader::ScanDirectory(const std::string& directory, std::vector<ScriptFile>& scripts)
    {
        try
        {
            if (!fs::exists(directory))
            {
                LOG_WARN("eclipse.loader", "[Eclipse] ScriptLoader - Directory does not exist: {}", directory);
                return;
            }

            fs::recursive_directory_iterator end_iter;
            for (fs::recursive_directory_iterator dir_iter(directory); dir_iter != end_iter; ++dir_iter)
            {
                std::string filepath = dir_iter->path().generic_string();

                // Check for hidden files (platform-specific)
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
                    std::string filename = dir_iter->path().filename().generic_string();
                    std::string extension;

                    if (IsScriptFile(filename, extension))
                        scripts.emplace_back(filename, filepath, extension);
                }
            }
        }
        catch (const fs::filesystem_error& e)
        {
            LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader - Filesystem error: {}", e.what());
        }
    }

    /**
     * @brief Scan directory for .lua and .moon files
     *
     * Recursively scans lua_scripts directory tree.
     * Resets statistics before scanning.
     *
     * @return Vector of discovered script files
     */
    std::vector<ScriptFile> ScriptLoader::ScanScripts()
    {
        LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Scanning directory: {}", m_scriptPath);

        // Build require paths (currently unused, reserved for future)
        m_requirePath.clear();
        m_requireCPath.clear();

        std::vector<ScriptFile> scripts;
        ScanDirectory(m_scriptPath, scripts);

        // Sort scripts by priority: .ext (0) first, then .cout (1), .moon (2), .lua (3)
        std::sort(scripts.begin(), scripts.end());

        // Count file types and update statistics
        uint32 extScripts = 0, coutScripts = 0, luaScripts = 0, moonScripts = 0;
        for (const auto& script : scripts)
        {
            if (script.extension == ".ext") extScripts++;
            else if (script.extension == ".cout") coutScripts++;
            else if (script.extension == ".lua") luaScripts++;
            else if (script.extension == ".moon") moonScripts++;
        }

        auto& stats = Statistics::EclipseStatistics::GetInstance();
        stats.SetLoadingExtScripts(extScripts);
        stats.SetLoadingCoutScripts(coutScripts);
        stats.SetLoadingLuaScripts(luaScripts);
        stats.SetLoadingMoonScripts(moonScripts);
        stats.SetLoadingTotalScripts(scripts.size());

        LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Found {} scripts ({} Ext, {} Cout, {} Lua, {} MoonScript)",
            scripts.size(), extScripts, coutScripts, luaScripts, moonScripts);

        return scripts;
    }

    /**
     * @brief Setup require paths for a Lua state
     *
     * Configures package.path and package.cpath to allow require() from subdirectories.
     * Adds all subdirectories recursively.
     *
     * Example paths added:
     * - lua_scripts/?.lua
     * - lua_scripts/subdir/?.lua
     * - lua_scripts/?.dll (Windows) or ?.so (Unix)
     *
     * @param state Lua state to configure
     */
    void ScriptLoader::SetupRequirePaths(sol::state& state)
    {
        // Build require paths recursively for all subdirectories
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

            // Get current package.path and append
            sol::optional<std::string> currentPath = state["package"]["path"];
            if (currentPath)
                luaPath += *currentPath;

            sol::optional<std::string> currentCPath = state["package"]["cpath"];
            if (currentCPath)
                luaCPath += *currentCPath;

            state["package"]["path"] = luaPath;
            state["package"]["cpath"] = luaCPath;

            LOG_DEBUG("eclipse.loader", "[Eclipse] ScriptLoader - Configured require paths for state");
        }
        catch (const fs::filesystem_error& e)
        {
            LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader - Error setting up require paths: {}", e.what());
        }
    }

    /**
     * @brief Load a specific script file
     *
     * Workflow:
     * 1. Get Lua state (master or per-map)
     * 2. Compile script via ScriptCompiler (uses BytecodeCache)
     * 3. Load bytecode into state
     * 4. Execute loaded function
     *
     * @param scriptFile Script file metadata
     * @param stateId State ID to load into (-1 for master)
     * @return true if loaded and executed successfully
     */
    bool ScriptLoader::LoadScript(const ScriptFile& scriptFile, int32 stateId)
    {
        LOG_DEBUG("eclipse.loader", "[Eclipse] ScriptLoader - Loading script: {}", scriptFile.filepath);

        // Get the state to load into (DRY - uses helper)
        sol::state* state = GetStateOrNull(stateId, "LoadScript");
        if (!state)
        {
            Statistics::EclipseStatistics::GetInstance().IncrementLoadingFailed();
            return false;
        }

        try
        {
            // Compile to bytecode (via BytecodeCache)
            API::CompileOptions opts;
            opts.allowMoonScript = true;

            auto bytecode = API::ScriptCompiler::GetInstance().CompileFile(scriptFile.filepath, opts);

            if (!bytecode || !bytecode->isValid())
            {
                LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader - Failed to compile script: {}",
                    scriptFile.filepath);
                Statistics::EclipseStatistics::GetInstance().IncrementLoadingFailed();
                return false;
            }

            // Load bytecode into state (direct sol2 call)
            auto result = state->load(bytecode->bytecode.as_string_view(), scriptFile.filename);

            if (!result.valid())
            {
                sol::error err = result;
                LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader - Failed to load script: {} - Error: {}",
                    scriptFile.filepath, err.what());
                Statistics::EclipseStatistics::GetInstance().IncrementLoadingFailed();
                return false;
            }

            // Execute the loaded script
            sol::protected_function scriptFunc = result;
            auto execResult = scriptFunc();

            if (!execResult.valid())
            {
                sol::error err = execResult;
                LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader - Error executing script: {} - Error: {}",
                    scriptFile.filepath, err.what());
                Statistics::EclipseStatistics::GetInstance().IncrementLoadingFailed();
                return false;
            }

            LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Successfully loaded: {}", scriptFile.filename);
            Statistics::EclipseStatistics::GetInstance().IncrementLoadingSuccessful();
            return true;
        }
        catch (const sol::error& e)
        {
            LOG_ERROR("eclipse.loader", "[Eclipse] ScriptLoader - Exception loading script: {} - Error: {}",
                scriptFile.filepath, e.what());
            Statistics::EclipseStatistics::GetInstance().IncrementLoadingFailed();
            return false;
        }
    }

    /**
     * @brief Load all scripts from directory
     * 
     * Main script loading workflow:
     * 1. Get or create Lua state
     * 2. Setup require paths
     * 3. Scan for scripts
     * 4. Load each script
     * 
     * @param stateId State ID to load scripts into (-1 for master)
     * @return Number of scripts successfully loaded
     */
    uint32 ScriptLoader::LoadAllScripts(int32 stateId)
    {
        LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Loading all scripts into state {}", stateId);

        // Get the state and setup require paths first (DRY - uses helper)
        sol::state* state = GetStateOrNull(stateId, "LoadAllScripts");
        if (!state)
            return 0;

        // Setup require paths before loading scripts
        SetupRequirePaths(*state);

        auto scripts = ScanScripts();
        if (scripts.empty())
        {
            LOG_WARN("eclipse.loader", "[Eclipse] ScriptLoader - No scripts found in: {}", m_scriptPath);
            return 0;
        }

        // Track loading stats
        uint32 successCount = 0;
        for (const auto& script : scripts)
        {
            if (LoadScript(script, stateId))
                successCount++;
        }

        auto stats = Statistics::EclipseStatistics::GetInstance().GetSnapshot();
        LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Loaded {}/{} scripts ({} failed)", stats.loadingSuccessful, scripts.size(), stats.loadingFailed);

        return successCount;
    }

    /**
     * @brief Reload all scripts (clear cache + reload)
     *
     * Full reload workflow:
     * 1. Remove and recreate state (if per-map state)
     * 2. LoadAllScripts()
     *
     * @param stateId State ID to reload scripts for (-1 for master)
     * @return Number of scripts successfully reloaded
     */
    uint32 ScriptLoader::ReloadAllScripts(int32 stateId)
    {
        LOG_INFO("eclipse.loader", "[Eclipse] ScriptLoader - Reloading all scripts for state {}", stateId);

        // Remove and recreate the state (if per-map state)
        if (stateId >= 0)
            StateManager::GetInstance().RemoveState(stateId);

        // Reload scripts
        return LoadAllScripts(stateId);
    }

} // namespace Eclipse::Core
