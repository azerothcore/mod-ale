/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ScriptCompiler.h"
#include "StateManager.h"
#include "FileSystemUtils.h"
#include "Statistics.h"
#include "Log.h"
#include <fstream>

namespace ALE::API
{
    using Core::CompiledBytecode;
    using Core::StateManager;
    using Statistics::ALEStatistics;

    ScriptCompiler& ScriptCompiler::GetInstance()
    {
        static ScriptCompiler instance;
        return instance;
    }

    ScriptCompiler::ScriptCompiler() { }

    ScriptCompiler::~ScriptCompiler() { }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::Compile(const std::string& filepath)
    {
        // Determine file type and compile accordingly
        if (Utils::FileSystemUtils::IsCoutFile(filepath))
            return LoadCoutFile(filepath);
        else if (Utils::FileSystemUtils::IsMoonScriptFile(filepath))
            return CompileMoonScriptFile(filepath);
        else // .lua and .ext both use Lua syntax
            return CompileLuaFile(filepath);
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::CompileLuaFile(const std::string& filepath)
    {
        try
        {
            sol::state* state = StateManager::GetInstance().GetMasterState();
            if (!state)
            {
                LOG_ERROR("ale.compiler", "[ALE] Master state not available");
                return nullptr;
            }

            sol::load_result loadResult = state->load_file(filepath);
            if (!loadResult.valid())
            {
                sol::error err = loadResult;
                LOG_ERROR("ale.compiler", "[ALE] Failed to load {}: {}", filepath, err.what());
                return nullptr;
            }

            sol::protected_function pf = std::move(loadResult).get<sol::protected_function>();
            sol::bytecode bc = pf.dump();

            if (!ValidateBytecode(bc, filepath))
                return nullptr;

            auto bytecode = CreateBytecode(std::move(bc), filepath);

            LOG_DEBUG("ale.compiler", "[ALE] Compiled Lua {} ({} bytes)", filepath, bytecode->size());

            return bytecode;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ale.compiler", "[ALE] Exception compiling {}: {}", filepath, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::CompileMoonScriptFile(const std::string& filepath)
    {
        try
        {
            sol::state* state = StateManager::GetInstance().GetMasterState();
            if (!state)
            {
                LOG_ERROR("ale.compiler", "[ALE] Master state not available");
                return nullptr;
            }

            std::string moonLoader = "return require('moonscript').loadfile([[" + filepath + "]])";
            sol::load_result moonLoadResult = state->load(moonLoader);

            if (!moonLoadResult.valid())
            {
                sol::error err = moonLoadResult;
                LOG_ERROR("ale.compiler", "[ALE] Failed to load MoonScript {}: {}", filepath, err.what());
                return nullptr;
            }

            sol::protected_function_result pfr = moonLoadResult();
            if (!pfr.valid())
            {
                sol::error err = pfr;
                LOG_ERROR("ale.compiler", "[ALE] Failed to compile MoonScript {}: {}", filepath, err.what());
                return nullptr;
            }

            sol::protected_function pf = pfr;
            sol::bytecode bc = pf.dump();

            if (!ValidateBytecode(bc, filepath))
                return nullptr;

            auto bytecode = CreateBytecode(std::move(bc), filepath);

            LOG_DEBUG("ale.compiler", "[ALE] Compiled MoonScript {} ({} bytes)", filepath, bytecode->size());

            return bytecode;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ale.compiler", "[ALE] Exception compiling {}: {}", filepath, e.what());
            return nullptr;
        }
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::LoadCoutFile(const std::string& filepath)
    {
        try
        {
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open())
            {
                LOG_ERROR("ale.compiler", "[ALE] Failed to open: {}", filepath);
                return nullptr;
            }

            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            if (fileSize == 0)
            {
                file.close();
                LOG_ERROR("ale.compiler", "[ALE] Empty bytecode file: {}", filepath);
                return nullptr;
            }

            std::vector<std::byte> byteBuffer(fileSize);
            file.read(reinterpret_cast<char*>(byteBuffer.data()), fileSize);
            file.close();

            sol::bytecode bc(byteBuffer.begin(), byteBuffer.end());

            if (!ValidateBytecode(bc, filepath))
                return nullptr;

            auto bytecode = CreateBytecode(std::move(bc), filepath);

            LOG_DEBUG("ale.compiler", "[ALE] Loaded Cout {} ({} bytes)", filepath, bytecode->size());

            return bytecode;
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("ale.compiler", "[ALE] Exception loading {}: {}", filepath, e.what());
            return nullptr;
        }
    }

    bool ScriptCompiler::ValidateBytecode(const sol::bytecode& bc, const std::string& filepath)
    {
        if (bc.as_string_view().empty())
        {
            LOG_ERROR("ale.compiler", "[ALE] Failed to dump bytecode: {}", filepath);
            return false;
        }
        return true;
    }

    std::shared_ptr<CompiledBytecode> ScriptCompiler::CreateBytecode(sol::bytecode&& bc, const std::string& filepath)
    {
        auto bytecode = std::make_shared<CompiledBytecode>();
        bytecode->bytecode = std::move(bc);
        bytecode->filepath = filepath;
        bytecode->last_modified = Utils::FileSystemUtils::GetFileModTime(filepath);
        return bytecode;
    }

}
