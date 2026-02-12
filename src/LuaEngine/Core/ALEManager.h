/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_MANAGER_H
#define _ALE_MANAGER_H

#include "Common.h"

namespace ALE::Core
{
    /**
     * @class ALEManager
     * @brief Central manager for ALE lifecycle (initialization, shutdown, reload)
     */
    class ALEManager
    {
    public:
        /**
         * @brief Get singleton instance
         * @return Reference to the global ALEManager
         */
        static ALEManager& GetInstance();

        /**
         * @brief Initialize ALE system (first-time startup)
         *
         * @return true if initialization successful
         */
        bool Initialize();

        /**
         * @brief Shutdown ALE system (cleanup)
         *
         * Safe to call even if not initialized.
         */
        void Shutdown();

        /**
         * @brief Reload ALE system (hot reload)
         * @return true if reload successful
         */
        bool Reload();

        /*
         * @brief Reload ALE config
         *
         * @return true if config reloaded successfully
         */
        bool ReloadConfig();

        /**
         * @brief Check if ALE is initialized
         * @return true if Initialize() succeeded
         */
        bool IsInitialized() const { return m_initialized; }

        /**
         * @brief Check if ALE is enabled in config
         * @return true if ALE.Enabled = true
         */
        bool IsEnabled() const;

    private:
        ALEManager();
        ~ALEManager() = default;

        ALEManager(const ALEManager&) = delete;
        ALEManager& operator=(const ALEManager&) = delete;
        ALEManager(ALEManager&&) = delete;
        ALEManager& operator=(ALEManager&&) = delete;

        /**
         * @brief Initialize core components (StateManager, EventManager, Methods)
         * @return true if successful
         */
        bool InitializeCore();

        /**
         * @brief Load scripts with timing and statistics
         * @return true if successful
         */
        bool LoadScripts();

        bool m_initialized = false;
    };

} // namespace ALE::Core

#endif // _ALE_MANAGER_H
