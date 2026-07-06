/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEScriptCache.h"
#include "ALEConfig.h"
#include "ALEUtility.h"

#include <fstream>
#include <mutex>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

namespace
{
    struct CacheEntry
    {
        std::vector<char> bytecode;
        std::time_t lastModified = 0;
    };

    std::unordered_map<std::string, CacheEntry> bytecodeCache;
    std::unordered_map<std::string, std::time_t> timestampCache;
    std::mutex cacheMutex;

    std::time_t GetFileModTime(std::string const& filepath)
    {
        struct stat fileInfo;
        if (stat(filepath.c_str(), &fileInfo) == 0)
            return fileInfo.st_mtime;

        return 0;
    }

    // Same, but remembers the answer until the next reload so a directory of
    // scripts doesn't stat the same files twice.
    std::time_t GetFileModTimeCached(std::string const& filepath)
    {
        auto iter = timestampCache.find(filepath);
        if (iter != timestampCache.end())
            return iter->second;

        std::time_t modTime = GetFileModTime(filepath);
        timestampCache[filepath] = modTime;
        return modTime;
    }

    // Loads `filepath` as a chunk in `lua` without caching:
    // plain compilation for Lua, the `moonscript` module for MoonScript.
    sol::protected_function LoadUncached(sol::state& lua, std::string const& filepath, bool isMoonScript)
    {
        if (isMoonScript)
        {
            std::string loader = "return require('moonscript').loadfile([[" + filepath + "]])";
            sol::protected_function_result result = lua.safe_script(loader, sol::script_pass_on_error);
            if (!result.valid())
            {
                sol::error error = result;
                ALE_LOG_ERROR("[ALE]: Error compiling MoonScript `{}`: {}", filepath, error.what());
                return sol::protected_function();
            }

            return result.get<sol::protected_function>();
        }

        sol::load_result loaded = lua.load_file(filepath);
        if (!loaded.valid())
        {
            sol::error error = loaded;
            ALE_LOG_ERROR("[ALE]: Error loading `{}`: {}", filepath, error.what());
            return sol::protected_function();
        }

        return loaded.get<sol::protected_function>();
    }

    // Compiles `filepath` in a throwaway state and stores its bytecode.
    // Returns false (leaving no cache entry) if compilation fails.
    bool CompileToCache(std::string const& filepath, bool isMoonScript)
    {
        sol::state compiler;
        compiler.open_libraries();

        sol::protected_function chunk = LoadUncached(compiler, filepath, isMoonScript);
        if (!chunk.valid())
            return false;

        sol::bytecode dumped = chunk.dump();
        if (dumped.as_string_view().empty())
            return false;

        std::lock_guard<std::mutex> guard(cacheMutex);
        CacheEntry& entry = bytecodeCache[filepath];
        entry.lastModified = GetFileModTime(filepath);
        entry.bytecode.assign(dumped.as_string_view().begin(), dumped.as_string_view().end());
        return true;
    }

    // Loads the cached bytecode of `filepath` if it is still up to date.
    sol::protected_function LoadFromCache(sol::state& lua, std::string const& filepath)
    {
        std::lock_guard<std::mutex> guard(cacheMutex);

        auto iter = bytecodeCache.find(filepath);
        if (iter == bytecodeCache.end() || iter->second.bytecode.empty())
            return sol::protected_function();

        std::time_t modTime = GetFileModTimeCached(filepath);
        if (modTime == 0 || iter->second.lastModified != modTime)
            return sol::protected_function();

        sol::load_result loaded = lua.load(
            std::string_view(iter->second.bytecode.data(), iter->second.bytecode.size()),
            filepath, sol::load_mode::binary);
        if (!loaded.valid())
            return sol::protected_function();

        return loaded.get<sol::protected_function>();
    }
}

namespace ALEScriptCache
{
    sol::protected_function Load(sol::state& lua, std::string const& filepath, bool isMoonScript,
        uint32& compiledCount, uint32& cachedCount)
    {
        if (!ALEConfig::GetInstance().IsByteCodeCacheEnabled())
            return LoadUncached(lua, filepath, isMoonScript);

        if (sol::protected_function cached = LoadFromCache(lua, filepath); cached.valid())
        {
            ++cachedCount;
            return cached;
        }

        if (CompileToCache(filepath, isMoonScript))
        {
            if (sol::protected_function compiled = LoadFromCache(lua, filepath); compiled.valid())
            {
                ++compiledCount;
                return compiled;
            }
        }

        // The cache could not serve this file; fall back to a direct load.
        return LoadUncached(lua, filepath, isMoonScript);
    }

    sol::protected_function LoadCompiled(sol::state& lua, std::string const& filepath)
    {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            ALE_LOG_ERROR("[ALE]: Could not open compiled script `{}`", filepath);
            return sol::protected_function();
        }

        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

        sol::load_result loaded = lua.load(
            std::string_view(buffer.data(), buffer.size()), filepath, sol::load_mode::binary);
        if (!loaded.valid())
        {
            sol::error error = loaded;
            ALE_LOG_ERROR("[ALE]: Error loading compiled script `{}`: {}", filepath, error.what());
            return sol::protected_function();
        }

        return loaded.get<sol::protected_function>();
    }

    void ClearCache()
    {
        std::lock_guard<std::mutex> guard(cacheMutex);
        bytecodeCache.clear();
        timestampCache.clear();
        ALE_LOG_INFO("[ALE]: Global bytecode cache cleared");
    }

    void ClearTimestamps()
    {
        std::lock_guard<std::mutex> guard(cacheMutex);
        timestampCache.clear();
    }
}
