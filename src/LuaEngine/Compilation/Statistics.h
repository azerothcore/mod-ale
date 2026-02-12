/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_STATISTICS_H
#define _ALE_STATISTICS_H

#include <atomic>
#include <cstddef>

namespace ALE::Statistics
{
    /**
     * @class ALEStatistics
     * @brief Centralized lock-free statistics for ALL ALE components
     *
     */
    class ALEStatistics
    {
    public:
        /**
         * @struct Snapshot
         * @brief Complete statistics snapshot from all components
         */
        struct Snapshot
        {
            size_t compilationSuccess = 0;        ///< Scripts compiled this load
            size_t compilationTotalBytecodeSize = 0;  ///< Total bytecode size
            size_t cacheHits = 0;                 ///< Scripts served from cache
        };

        /**
         * @brief Get singleton instance
         */
        static ALEStatistics& GetInstance()
        {
            static ALEStatistics instance;
            return instance;
        }

        ALEStatistics(const ALEStatistics&) = delete;
        ALEStatistics& operator=(const ALEStatistics&) = delete;
        ALEStatistics(ALEStatistics&&) = delete;
        ALEStatistics& operator=(ALEStatistics&&) = delete;

        // ===== COMPILATION METRICS =====
        void IncrementCompilationSuccess() { m_compilationSuccess.fetch_add(1, std::memory_order_relaxed); }
        void AddCompilationBytecodeSize(size_t size) { m_compilationTotalBytecodeSize.fetch_add(size, std::memory_order_relaxed); }

        // ===== CACHE METRICS =====
        void SetCacheTotalScripts(size_t count) { m_cacheTotalScripts.store(count, std::memory_order_relaxed); }
        void SetCacheTotalMemory(size_t bytes) { m_cacheTotalMemory.store(bytes, std::memory_order_relaxed); }
        void IncrementCacheHit() { m_cacheHits.fetch_add(1, std::memory_order_relaxed); }

        /**
         * @brief Reset load counters before script loading
         * 
         * Clears compilation and cache hit counters to track per-load stats.
         */
        void ResetLoadStats()
        {
            m_compilationSuccess.store(0, std::memory_order_relaxed);
            m_compilationTotalBytecodeSize.store(0, std::memory_order_relaxed);
            m_cacheHits.store(0, std::memory_order_relaxed);
        }

        /**
         * @brief Get statistics snapshot
         * @return Current stats values
         */
        Snapshot GetSnapshot() const
        {
            Snapshot snapshot;
            snapshot.compilationSuccess = m_compilationSuccess.load(std::memory_order_relaxed);
            snapshot.compilationTotalBytecodeSize = m_compilationTotalBytecodeSize.load(std::memory_order_relaxed);
            snapshot.cacheHits = m_cacheHits.load(std::memory_order_relaxed);
            return snapshot;
        }

    private:
        ALEStatistics() = default;
        ~ALEStatistics() = default;

        std::atomic<size_t> m_compilationSuccess{0};
        std::atomic<size_t> m_compilationTotalBytecodeSize{0};
        std::atomic<size_t> m_cacheHits{0};

        std::atomic<size_t> m_cacheTotalScripts{0};
        std::atomic<size_t> m_cacheTotalMemory{0};
    };

} // namespace ALE::Statistics

#endif // _ALE_STATISTICS_H
