/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEScriptCache.h"
#include "ALEConfig.h"
#include "ALEUtility.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace
{
    /*
     * Identity of a script file's content: sub-second modification time plus
     * size. Second-level mtime alone would let a script edited twice within
     * the same second serve stale bytecode.
     */
    struct FileStamp
    {
        std::filesystem::file_time_type mtime{};
        uintmax_t size = 0;

        bool operator==(FileStamp const&) const = default;
        bool IsValid() const { return *this != FileStamp{}; }
    };

    struct CacheEntry
    {
        std::vector<char> bytecode;
        FileStamp stamp;
    };

    std::unordered_map<std::string, CacheEntry> bytecodeCache;
    std::unordered_map<std::string, FileStamp> stampCache;
    std::mutex cacheMutex;

    FileStamp GetFileStamp(std::string const& filepath)
    {
        std::error_code ec;
        FileStamp stamp;

        stamp.mtime = std::filesystem::last_write_time(filepath, ec);
        if (ec)
            return {};

        stamp.size = std::filesystem::file_size(filepath, ec);
        if (ec)
            return {};

        return stamp;
    }

    // Same, but remembers the answer until the next reload so a directory of
    // scripts doesn't stat the same files twice.
    FileStamp GetFileStampCached(std::string const& filepath)
    {
        auto iter = stampCache.find(filepath);
        if (iter != stampCache.end())
            return iter->second;

        FileStamp stamp = GetFileStamp(filepath);
        stampCache[filepath] = stamp;
        return stamp;
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
        entry.stamp = GetFileStamp(filepath);
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

        FileStamp stamp = GetFileStampCached(filepath);
        if (!stamp.IsValid() || iter->second.stamp != stamp)
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
        stampCache.clear();
        ALE_LOG_INFO("[ALE]: Global bytecode cache cleared");
    }

    void ClearTimestamps()
    {
        std::lock_guard<std::mutex> guard(cacheMutex);
        stampCache.clear();
    }
}
