/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ScriptCompiler.h"
#include "StateManager.h"
#include "Statistics.h"
#include "Log.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sys/stat.h>

namespace Eclipse::API
{
    using Core::BytecodeCache;
    using Core::StateManager;
    using Core::CompiledBytecode;
    using Statistics::EclipseStatistics;

    ScriptCompiler& ScriptCompiler::GetInstance()
    {
        static ScriptCompiler instance;
        return instance;
    }

    ScriptCompiler::ScriptCompiler() { }

    ScriptCompiler::~ScriptCompiler()
    {
        auto stats = EclipseStatistics::GetInstance().GetSnapshot();
        LOG_INFO("server.loading", "Eclipse::ScriptCompiler - Shutting down. Stats: {} successful, {} failed", stats.compilationSuccess, stats.compilationFailed);
    }

    const CompiledBytecode* ScriptCompiler::CompileFile(const std::string& filepath, const CompileOptions& options)
    {
        auto& cache = BytecodeCache::GetInstance();
        auto& stats = EclipseStatistics::GetInstance();

        // Try to get from cache first
        const auto* cached = cache.Get(filepath);
        if (cached)
            return cached;

        // Determine file type and compile accordingly
        // Priority: .ext > .cout > .moon > .lua
        std::shared_ptr<CompiledBytecode> bytecode;

        if (IsCoutFile(filepath))
        {
            bytecode = LoadCoutFile(filepath);
        }
        else if (IsMoonScriptFile(filepath))
        {
            bytecode = CompileMoonScriptFile(filepath);
        }
        else  // Both .lua and .ext use CompileLuaFile (Lua syntax)
        {
            bytecode = CompileLuaFile(filepath);
        }

        if (bytecode && bytecode->isValid())
        {
            // Store in cache
            cache.Store(filepath, bytecode);

            // Update statistics
            stats.IncrementCompilationSuccess();
            stats.AddCompilationBytecodeSize(bytecode->size());

            return bytecode.get();
        }

        stats.IncrementCompilationFailed();
        return nullptr;
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::CompileLuaFile(const std::string& filepath)
    {
        try
        {
            sol::state* masterState = GetMasterStateOrNull("CompileLuaFile");
            if (!masterState)
                return nullptr;

            // Load Lua file
            sol::load_result loadResult = masterState->load_file(filepath);

            if (!loadResult.valid())
            {
                sol::error err = loadResult;
                LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::CompileLuaFile - Failed to load {}: {}",
                    filepath, err.what());
                return nullptr;
            }

            // Convert to protected_function and dump to bytecode
            sol::protected_function pf = std::move(loadResult).get<sol::protected_function>();
            sol::bytecode bc = pf.dump();

            // Validate bytecode
            if (!ValidateBytecode(bc, filepath, "CompileLuaFile"))
                return nullptr;

            // Create bytecode structure
            auto bytecode = CreateBytecodeStructure(std::move(bc), filepath);

            LOG_DEBUG("server.loading", "[Eclipse] ScriptCompiler::CompileLuaFile - Compiled {} ({} bytes)",
                filepath, bytecode->size());

            // Track statistics (.ext files are priority Lua)
            if (IsExtFile(filepath))
                EclipseStatistics::GetInstance().IncrementCompilationExtFile();

            return bytecode;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::CompileLuaFile - Exception compiling {}: {}",
                filepath, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::CompileMoonScriptFile(const std::string& filepath)
    {
        try
        {
            sol::state* masterState = GetMasterStateOrNull("CompileMoonScriptFile");
            if (!masterState)
                return nullptr;

            std::string moonLoader = "return require('moonscript').loadfile([[" + filepath + "]])";
            sol::load_result moonLoadResult = masterState->load(moonLoader);

            if (!moonLoadResult.valid())
            {
                sol::error err = moonLoadResult;
                LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::CompileMoonScriptFile - Failed to load MoonScript {}: {}", filepath, err.what());
                return nullptr;
            }

            // Execute moonscript.loadfile to get compiled function
            sol::protected_function_result pfr = moonLoadResult();
            if (!pfr.valid())
            {
                sol::error err = pfr;
                LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::CompileMoonScriptFile - Failed to compile MoonScript {}: {}", filepath, err.what());
                return nullptr;
            }

            // Extract compiled function and dump to bytecode
            sol::protected_function pf = pfr;
            sol::bytecode bc = pf.dump();

            // Validate bytecode
            if (!ValidateBytecode(bc, filepath, "CompileMoonScriptFile"))
                return nullptr;

            // Create bytecode structure
            auto bytecode = CreateBytecodeStructure(std::move(bc), filepath);

            LOG_DEBUG("server.loading", "[Eclipse] ScriptCompiler::CompileMoonScriptFile - Compiled {} ({} bytes)", filepath, bytecode->size());

            EclipseStatistics::GetInstance().IncrementCompilationMoonScript();

            return bytecode;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::CompileMoonScriptFile - Exception compiling {}: {}", filepath, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::LoadCoutFile(const std::string& filepath)
    {
        try
        {
            // Read the bytecode file
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open())
            {
                LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::LoadCoutFile - Failed to open: {}", filepath);
                return nullptr;
            }

            // Get file size
            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            if (fileSize == 0)
            {
                file.close();
                LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::LoadCoutFile - Empty bytecode file: {}", filepath);
                return nullptr;
            }

            // Read directly into std::byte buffer (native bytecode format)
            std::vector<std::byte> byteBuffer(fileSize);
            file.read(reinterpret_cast<char*>(byteBuffer.data()), fileSize);
            file.close();

            // Create bytecode from buffer (zero-copy from file to sol::bytecode)
            sol::bytecode bc(byteBuffer.begin(), byteBuffer.end());

            // Validate bytecode
            if (!ValidateBytecode(bc, filepath, "LoadCoutFile"))
                return nullptr;

            // Create bytecode structure
            auto bytecode = CreateBytecodeStructure(std::move(bc), filepath);

            LOG_DEBUG("server.loading", "[Eclipse] ScriptCompiler::LoadCoutFile - Loaded {} ({} bytes)", filepath, bytecode->size());

            EclipseStatistics::GetInstance().IncrementCompilationCoutFile();

            return bytecode;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::LoadCoutFile - Exception loading {}: {}", filepath, e.what());
            return nullptr;
        }
    }

    sol::state* ScriptCompiler::GetMasterStateOrNull(const char* callerName)
    {
        auto& stateManager = StateManager::GetInstance();
        sol::state* masterState = stateManager.GetMasterState();

        if (!masterState)
        {
            LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::{} - Master state not available", callerName);
            return nullptr;
        }

        return masterState;
    }

    bool ScriptCompiler::ValidateBytecode(const sol::bytecode& bc, const std::string& filepath, const char* callerName)
    {
        if (bc.as_string_view().empty())
        {
            LOG_ERROR("server.loading", "[Eclipse] ScriptCompiler::{} - Failed to dump bytecode: {}", callerName, filepath);
            return false;
        }
        return true;
    }

    bool ScriptCompiler::HasExtension(const std::string& filepath, const std::string& extension)
    {
        std::filesystem::path path(filepath);
        return path.extension() == extension;
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::CreateBytecodeStructure(sol::bytecode&& bc, const std::string& filepath)
    {
        auto& cache = BytecodeCache::GetInstance();

        // Get file modification time
        std::time_t modTime = cache.GetFileModTime(filepath);

        // Create and populate bytecode structure
        auto bytecode = std::make_shared<CompiledBytecode>();
        bytecode->bytecode = std::move(bc);
        bytecode->filepath = filepath;
        bytecode->last_modified = modTime;

        return bytecode;
    }

}
