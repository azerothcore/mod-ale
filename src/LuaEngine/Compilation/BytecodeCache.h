/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _SOL_BYTECODE_CACHE_H
#define _SOL_BYTECODE_CACHE_H

#include <sol/sol.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <ctime>
#include "Common.h"

namespace ALE::Core
{
    /**
     * @struct CompiledBytecode
     * @brief Represents compiled Lua bytecode using Sol2's native bytecode format
     */
    struct CompiledBytecode
    {
        sol::bytecode bytecode;        // < Sol2 native bytecode object
        std::string filepath;          // < Original source file path
        std::time_t last_modified;     // < File modification time (unix timestamp)

        /**
         * @brief Default constructor
         */
        CompiledBytecode() : last_modified(0) {}

        /**
         * @brief Check if bytecode is valid
         * @return true if bytecode exists and is non-empty
         */
        bool isValid() const { return !bytecode.as_string_view().empty(); }

        /**
         * @brief Get bytecode size in bytes
         * @return Size of compiled bytecode
         */
        size_t size() const { return bytecode.as_string_view().size(); }
    };

    /**
     * @class BytecodeCache
     * @brief In-memory cache for compiled Lua/MoonScript bytecode
     */
    class BytecodeCache
    {
    public:
        BytecodeCache();
        ~BytecodeCache();

        BytecodeCache(const BytecodeCache&) = delete;
        BytecodeCache& operator=(const BytecodeCache&) = delete;
        BytecodeCache(BytecodeCache&&) = delete;
        BytecodeCache& operator=(BytecodeCache&&) = delete;

        /**
         * @brief Get singleton instance
         * @return Reference to the global BytecodeCache
         */
        static BytecodeCache& GetInstance();

        /**
         * @brief Get bytecode from cache (no compilation)
         *
         * Validates:
         * 1. Entry exists in cache
         * 2. File timestamp matches (not modified)
         * 3. Bytecode is valid (non-empty)
         *
         * @param filepath Path to script file
         * @return Pointer to cached bytecode, nullptr if not found/outdated
         */
        const CompiledBytecode* Get(const std::string& filepath);

        /**
         * @brief Store compiled bytecode in cache
         *
         * Replaces any existing entry for this filepath.
         *
         * @param filepath Path to script file (used as cache key)
         * @param bytecode Compiled bytecode to store
         */
        void Store(const std::string& filepath, std::shared_ptr<CompiledBytecode> bytecode);

        /**
         * @brief Clear all cached bytecode
         *
         * Called on script reload or server shutdown.
         * Forces recompilation on next load.
         */
        void ClearAll();

        /**
         * @brief Clear timestamp cache
         *
         * Forces fresh stat() calls on next cache check.
         * Useful after bulk file operations.
         */
        void ClearTimestampCache();

        /**
         * @brief Get number of cached scripts
         * @return Cache entry count
         */
        size_t GetCacheSize() const;

        /**
         * @brief Get total memory used by cache
         * @return Sum of all bytecode sizes in bytes
         */
        size_t GetTotalMemory() const;

    private:
        std::unordered_map<std::string, std::shared_ptr<CompiledBytecode>> m_cache;      // < Bytecode storage
        std::unordered_map<std::string, std::time_t> m_timestampCache;  // < File timestamp cache

        /**
         * @brief Get file modification time (with caching)
         *
         * Caches timestamp to avoid repeated stat() calls.
         *
         * @param filepath Path to file
         * @return Unix timestamp, 0 if file doesn't exist
         */
        std::time_t GetFileModTime(const std::string& filepath);
    };

} // namespace ALE::Core

#endif // _SOL_BYTECODE_CACHE_H
