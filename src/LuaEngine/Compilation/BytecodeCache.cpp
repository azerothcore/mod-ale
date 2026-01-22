/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "BytecodeCache.h"
#include "../State/StateManager.h"
#include "Statistics.h"
#include "Log.h"
#include <fstream>
#include <sys/stat.h>

using Eclipse::Statistics::EclipseStatistics;

namespace Eclipse::Core
{
    BytecodeCache& BytecodeCache::GetInstance()
    {
        static BytecodeCache instance;
        return instance;
    }

    BytecodeCache::BytecodeCache() { }

    /**
     * @brief Destructor - clears all cached bytecode
     */
    BytecodeCache::~BytecodeCache()
    {
        ClearAll();
    }

    /**
     * @brief Get bytecode from cache (no compilation)
     */
    const CompiledBytecode* BytecodeCache::Get(const std::string& filepath)
    {
        // Check if entry exists
        auto it = m_cache.find(filepath);
        if (it == m_cache.end())
        {
            EclipseStatistics::GetInstance().IncrementCacheMiss();
            return nullptr;
        }

        // Validate bytecode is non-empty
        if (it->second->bytecode.empty() || !it->second->isValid())
        {
            m_cache.erase(it);
            return nullptr;
        }

        // Validate file hasn't changed (timestamp check)
        std::time_t currentModTime = GetFileModTime(filepath);
        if (it->second->last_modified != currentModTime || currentModTime == 0)
        {
            m_cache.erase(it);
            EclipseStatistics::GetInstance().IncrementCacheMiss();
            return nullptr;
        }

        EclipseStatistics::GetInstance().IncrementCacheHit();
        return it->second.get();
    }

    /**
     * @brief Store compiled bytecode in cache
     */
    void BytecodeCache::Store(const std::string& filepath, std::shared_ptr<CompiledBytecode> bytecode)
    {
        if (!bytecode || !bytecode->isValid())
        {
            LOG_ERROR("server.loading", "[Eclipse] BytecodeCache::Store - Invalid bytecode for {}", filepath);
            return;
        }

        m_cache[filepath] = bytecode;
        EclipseStatistics::GetInstance().SetCacheTotalScripts(m_cache.size());
        EclipseStatistics::GetInstance().SetCacheTotalMemory(GetTotalMemory());

        LOG_DEBUG("server.loading", "[Eclipse] BytecodeCache::Store - Cached {} ({} bytes)", filepath, bytecode->size());
    }

    /**
     * @brief Clear all cached bytecode
     *
     * Called on script reload or server shutdown.
     * Forces recompilation on next load.
     */
    void BytecodeCache::ClearAll()
    {
        size_t count = m_cache.size();
        if (count > 0)
        {
            LOG_INFO("server.loading", "[Eclipse] BytecodeCache - Clearing {} cached entries", count);
            m_cache.clear();
        }
        m_timestampCache.clear();
    }

    /**
     * @brief Clear timestamp cache
     *
     * Forces fresh stat() calls on next cache validation.
     * Useful after bulk file modifications.
     */
    void BytecodeCache::ClearTimestampCache()
    {
        m_timestampCache.clear();
    }

    /**
     * @brief Get number of cached scripts
     * @return Cache entry count
     */
    size_t BytecodeCache::GetCacheSize() const
    {
        return m_cache.size();
    }

    /**
     * @brief Get total memory used by cache
     *
     * Iterates all cached bytecode and sums sizes.
     *
     * @return Total bytes used by all cached bytecode
     */
    size_t BytecodeCache::GetTotalMemory() const
    {
        size_t total = 0;
        for (const auto& [_, entry] : m_cache)
            total += entry->size();

        return total;
    }

    /**
     * @brief Get file modification time
     *
     * @param filepath Path to file
     * @return Unix timestamp, 0 if file doesn't exist
     */
    std::time_t BytecodeCache::GetFileModTime(const std::string& filepath)
    {
        // Check timestamp cache first
        auto it = m_timestampCache.find(filepath);
        if (it != m_timestampCache.end())
            return it->second;

        // Get actual file modification time via stat()
        struct stat fileInfo;
        std::time_t modTime = 0;
        if (stat(filepath.c_str(), &fileInfo) == 0)
            modTime = fileInfo.st_mtime;

        // Cache the timestamp for future checks
        m_timestampCache[filepath] = modTime;
        return modTime;
    }
} // namespace Eclipse::Core
