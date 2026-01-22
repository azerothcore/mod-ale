/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ECLIPSE_STATISTICS_H
#define _ECLIPSE_STATISTICS_H

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace Eclipse::Statistics
{
    /**
     * @class EclipseStatistics
     * @brief Centralized lock-free statistics for ALL Eclipse components
     *
     */
    class EclipseStatistics
    {
    public:
        /**
         * @struct Snapshot
         * @brief Complete statistics snapshot from all components
         */
        struct Snapshot
        {
            // ===== Compilation (ScriptCompiler) =====
            size_t compilationSuccess = 0;
            size_t compilationFailed = 0;
            size_t compilationTotalBytecodeSize = 0;
            size_t compilationMoonScript = 0;
            size_t compilationExtFiles = 0;      // .ext files compiled
            size_t compilationCoutFiles = 0;     // .cout files loaded

            // ===== BytecodeCache =====
            size_t cacheTotalScripts = 0;
            size_t cacheTotalMemory = 0;
            size_t cacheCompilationCount = 0;
            size_t cacheHits = 0;
            size_t cacheMisses = 0;

            // ===== Loading (ScriptLoader) =====
            uint32 loadingTotalScripts = 0;
            uint32 loadingLuaScripts = 0;
            uint32 loadingMoonScripts = 0;
            uint32 loadingExtScripts = 0;      // .ext files
            uint32 loadingCoutScripts = 0;     // .cout files
            uint32 loadingSuccessful = 0;
            uint32 loadingFailed = 0;

            // ===== Events (EventManager) =====
            size_t eventsGlobalHandlers = 0;
            size_t eventsEntryHandlers = 0;
            size_t eventsUniqueHandlers = 0;
            uint64 eventsTotalTriggers = 0;
            uint64 eventsTotalHandlersExecuted = 0;
        };

        /**
         * @brief Get singleton instance
         */
        static EclipseStatistics& GetInstance()
        {
            static EclipseStatistics instance;
            return instance;
        }

        EclipseStatistics(const EclipseStatistics&) = delete;
        EclipseStatistics& operator=(const EclipseStatistics&) = delete;
        EclipseStatistics(EclipseStatistics&&) = delete;
        EclipseStatistics& operator=(EclipseStatistics&&) = delete;

        // ===== COMPILATION METRICS =====
        void IncrementCompilationSuccess() { m_compilationSuccess.fetch_add(1, std::memory_order_relaxed); }
        void IncrementCompilationFailed() { m_compilationFailed.fetch_add(1, std::memory_order_relaxed); }
        void AddCompilationBytecodeSize(size_t size) { m_compilationTotalBytecodeSize.fetch_add(size, std::memory_order_relaxed); }
        void IncrementCompilationMoonScript() { m_compilationMoonScript.fetch_add(1, std::memory_order_relaxed); }
        void IncrementCompilationExtFile() { m_compilationExtFiles.fetch_add(1, std::memory_order_relaxed); }
        void IncrementCompilationCoutFile() { m_compilationCoutFiles.fetch_add(1, std::memory_order_relaxed); }

        // ===== CACHE METRICS =====
        void SetCacheTotalScripts(size_t count) { m_cacheTotalScripts.store(count, std::memory_order_relaxed); }
        void SetCacheTotalMemory(size_t bytes) { m_cacheTotalMemory.store(bytes, std::memory_order_relaxed); }
        void IncrementCacheCompilation() { m_cacheCompilationCount.fetch_add(1, std::memory_order_relaxed); }
        void IncrementCacheHit() { m_cacheHits.fetch_add(1, std::memory_order_relaxed); }
        void IncrementCacheMiss() { m_cacheMisses.fetch_add(1, std::memory_order_relaxed); }

        // ===== LOADING METRICS =====
        void SetLoadingTotalScripts(uint32 count) { m_loadingTotalScripts.store(count, std::memory_order_relaxed); }
        void SetLoadingLuaScripts(uint32 count) { m_loadingLuaScripts.store(count, std::memory_order_relaxed); }
        void SetLoadingMoonScripts(uint32 count) { m_loadingMoonScripts.store(count, std::memory_order_relaxed); }
        void SetLoadingExtScripts(uint32 count) { m_loadingExtScripts.store(count, std::memory_order_relaxed); }
        void SetLoadingCoutScripts(uint32 count) { m_loadingCoutScripts.store(count, std::memory_order_relaxed); }
        void IncrementLoadingSuccessful() { m_loadingSuccessful.fetch_add(1, std::memory_order_relaxed); }
        void IncrementLoadingFailed() { m_loadingFailed.fetch_add(1, std::memory_order_relaxed); }
        void ResetLoadingCounters() 
        {
            m_loadingSuccessful.store(0, std::memory_order_relaxed);
            m_loadingFailed.store(0, std::memory_order_relaxed);
        }

        // ===== EVENT METRICS =====
        void IncrementEventsGlobalHandlers() { m_eventsGlobalHandlers.fetch_add(1, std::memory_order_relaxed); }
        void DecrementEventsGlobalHandlers(size_t count) { m_eventsGlobalHandlers.fetch_sub(count, std::memory_order_relaxed); }
        void IncrementEventsEntryHandlers() { m_eventsEntryHandlers.fetch_add(1, std::memory_order_relaxed); }
        void DecrementEventsEntryHandlers(size_t count) { m_eventsEntryHandlers.fetch_sub(count, std::memory_order_relaxed); }
        void IncrementEventsUniqueHandlers() { m_eventsUniqueHandlers.fetch_add(1, std::memory_order_relaxed); }
        void IncrementEventsTrigger() { m_eventsTotalTriggers.fetch_add(1, std::memory_order_relaxed); }
        void IncrementEventsHandlerExecuted() { m_eventsTotalHandlersExecuted.fetch_add(1, std::memory_order_relaxed); }

        /**
         * @brief Get complete statistics snapshot
         * @return Snapshot of all metrics (eventually consistent)
         */
        Snapshot GetSnapshot() const
        {
            Snapshot snapshot;

            // Compilation
            snapshot.compilationSuccess = m_compilationSuccess.load(std::memory_order_relaxed);
            snapshot.compilationFailed = m_compilationFailed.load(std::memory_order_relaxed);
            snapshot.compilationTotalBytecodeSize = m_compilationTotalBytecodeSize.load(std::memory_order_relaxed);
            snapshot.compilationMoonScript = m_compilationMoonScript.load(std::memory_order_relaxed);
            snapshot.compilationExtFiles = m_compilationExtFiles.load(std::memory_order_relaxed);
            snapshot.compilationCoutFiles = m_compilationCoutFiles.load(std::memory_order_relaxed);

            // Cache
            snapshot.cacheTotalScripts = m_cacheTotalScripts.load(std::memory_order_relaxed);
            snapshot.cacheTotalMemory = m_cacheTotalMemory.load(std::memory_order_relaxed);
            snapshot.cacheCompilationCount = m_cacheCompilationCount.load(std::memory_order_relaxed);
            snapshot.cacheHits = m_cacheHits.load(std::memory_order_relaxed);
            snapshot.cacheMisses = m_cacheMisses.load(std::memory_order_relaxed);

            // Loading
            snapshot.loadingTotalScripts = m_loadingTotalScripts.load(std::memory_order_relaxed);
            snapshot.loadingLuaScripts = m_loadingLuaScripts.load(std::memory_order_relaxed);
            snapshot.loadingMoonScripts = m_loadingMoonScripts.load(std::memory_order_relaxed);
            snapshot.loadingExtScripts = m_loadingExtScripts.load(std::memory_order_relaxed);
            snapshot.loadingCoutScripts = m_loadingCoutScripts.load(std::memory_order_relaxed);
            snapshot.loadingSuccessful = m_loadingSuccessful.load(std::memory_order_relaxed);
            snapshot.loadingFailed = m_loadingFailed.load(std::memory_order_relaxed);

            // Events
            snapshot.eventsGlobalHandlers = m_eventsGlobalHandlers.load(std::memory_order_relaxed);
            snapshot.eventsEntryHandlers = m_eventsEntryHandlers.load(std::memory_order_relaxed);
            snapshot.eventsUniqueHandlers = m_eventsUniqueHandlers.load(std::memory_order_relaxed);
            snapshot.eventsTotalTriggers = m_eventsTotalTriggers.load(std::memory_order_relaxed);
            snapshot.eventsTotalHandlersExecuted = m_eventsTotalHandlersExecuted.load(std::memory_order_relaxed);

            return snapshot;
        }

        /**
         * @brief Reset ALL counters to zero
         */
        void ResetAll()
        {
            // Compilation
            m_compilationSuccess.store(0, std::memory_order_relaxed);
            m_compilationFailed.store(0, std::memory_order_relaxed);
            m_compilationTotalBytecodeSize.store(0, std::memory_order_relaxed);
            m_compilationMoonScript.store(0, std::memory_order_relaxed);
            m_compilationExtFiles.store(0, std::memory_order_relaxed);
            m_compilationCoutFiles.store(0, std::memory_order_relaxed);

            // Cache
            m_cacheTotalScripts.store(0, std::memory_order_relaxed);
            m_cacheTotalMemory.store(0, std::memory_order_relaxed);
            m_cacheCompilationCount.store(0, std::memory_order_relaxed);
            m_cacheHits.store(0, std::memory_order_relaxed);
            m_cacheMisses.store(0, std::memory_order_relaxed);

            // Loading
            m_loadingTotalScripts.store(0, std::memory_order_relaxed);
            m_loadingLuaScripts.store(0, std::memory_order_relaxed);
            m_loadingMoonScripts.store(0, std::memory_order_relaxed);
            m_loadingExtScripts.store(0, std::memory_order_relaxed);
            m_loadingCoutScripts.store(0, std::memory_order_relaxed);
            m_loadingSuccessful.store(0, std::memory_order_relaxed);
            m_loadingFailed.store(0, std::memory_order_relaxed);

            // Events
            m_eventsGlobalHandlers.store(0, std::memory_order_relaxed);
            m_eventsEntryHandlers.store(0, std::memory_order_relaxed);
            m_eventsUniqueHandlers.store(0, std::memory_order_relaxed);
            m_eventsTotalTriggers.store(0, std::memory_order_relaxed);
            m_eventsTotalHandlersExecuted.store(0, std::memory_order_relaxed);
        }

    private:
        EclipseStatistics() = default;
        ~EclipseStatistics() = default;

        // Compilation atomics
        std::atomic<size_t> m_compilationSuccess{0};
        std::atomic<size_t> m_compilationFailed{0};
        std::atomic<size_t> m_compilationTotalBytecodeSize{0};
        std::atomic<size_t> m_compilationMoonScript{0};
        std::atomic<size_t> m_compilationExtFiles{0};
        std::atomic<size_t> m_compilationCoutFiles{0};

        // Cache atomics
        std::atomic<size_t> m_cacheTotalScripts{0};
        std::atomic<size_t> m_cacheTotalMemory{0};
        std::atomic<size_t> m_cacheCompilationCount{0};
        std::atomic<size_t> m_cacheHits{0};
        std::atomic<size_t> m_cacheMisses{0};

        // Loading atomics
        std::atomic<uint32> m_loadingTotalScripts{0};
        std::atomic<uint32> m_loadingLuaScripts{0};
        std::atomic<uint32> m_loadingMoonScripts{0};
        std::atomic<uint32> m_loadingExtScripts{0};
        std::atomic<uint32> m_loadingCoutScripts{0};
        std::atomic<uint32> m_loadingSuccessful{0};
        std::atomic<uint32> m_loadingFailed{0};

        // Event atomics
        std::atomic<size_t> m_eventsGlobalHandlers{0};
        std::atomic<size_t> m_eventsEntryHandlers{0};
        std::atomic<size_t> m_eventsUniqueHandlers{0};
        std::atomic<uint64> m_eventsTotalTriggers{0};
        std::atomic<uint64> m_eventsTotalHandlersExecuted{0};
    };

} // namespace Eclipse::Statistics

#endif // _ECLIPSE_STATISTICS_H
