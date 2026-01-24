/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_SCRIPT_LOADER_H
#define _ALE_SCRIPT_LOADER_H

#include <sol/sol.hpp>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "Common.h"

namespace ALE::Core
{
    // Forward declarations
    struct CompiledBytecode;
    /**
     * @struct ScriptFile
     * @brief Metadata for discovered script files
     * 
     * Supports: .ext (priority), .lua, .moon, .cout (pre-compiled)
     * - filename: Just the name (e.g., "test.lua")
     * - filepath: Full path (e.g., "lua_scripts/test.lua")
     * - extension: File extension (".ext", ".lua", ".moon", ".cout")
     * - priority: Load order (0 = highest priority)
     */
    struct ScriptFile
    {
        std::string filename;      ///< Filename only
        std::string filepath;      ///< Full path
        std::string extension;     ///< File extension
        int priority;              ///< Load priority (0=.ext, 1=.cout, 2=.moon, 3=.lua)

        /**
         * @brief Default constructor
         */
        ScriptFile() : priority(3) {}

        /**
         * @brief Constructor
         * @param name Filename
         * @param path Full filepath
         * @param ext File extension
         */
        ScriptFile(const std::string& name, const std::string& path, const std::string& ext)
            : filename(name), filepath(path), extension(ext)
        {
            // Set priority: .ext > .cout > .moon > .lua
            if (ext == ".ext")
                priority = 0;
            else if (ext == ".cout")
                priority = 1;
            else if (ext == ".moon")
                priority = 2;
            else // .lua
                priority = 3;
        }

        // Comparator for sorting by priority
        bool operator<(const ScriptFile& other) const
        {
            return priority < other.priority;
        }
    };

    /**
     * @class ScriptLoader
     * @brief Orchestrates script discovery, caching, compilation, and execution
     */
    class ScriptLoader
    {
    public:
        /**
         * @brief Default constructor
         */
        ScriptLoader();

        /**
         * @brief Destructor
         */
        ~ScriptLoader();

        ScriptLoader(const ScriptLoader&) = delete;
        ScriptLoader& operator=(const ScriptLoader&) = delete;
        ScriptLoader(ScriptLoader&&) = delete;
        ScriptLoader& operator=(ScriptLoader&&) = delete;

        /**
         * @brief Get singleton instance
         * @return Reference to the global ScriptLoader
         */
        static ScriptLoader& GetInstance();

        /**
         * @brief Set script directory path
         *
         * Sets the root directory for script scanning.
         * Default: "lua_scripts"
         *
         * @param path Path to lua_scripts directory (absolute or relative)
         */
        void SetScriptPath(const std::string& path);

        /**
         * @brief Get current script path
         * @return Script directory path
         */
        const std::string& GetScriptPath() const { return m_scriptPath; }

        /**
         * @brief Scan directory for .lua and .moon files
         *
         * @return Vector of discovered script files
         */
        std::vector<ScriptFile> ScanScripts();

        /**
         * @brief Load all scripts from directory
         *
         * Current: Loads all into master state (-1).
         * Future: Can specify per-map state ID.
         *
         * @param stateId State ID to load scripts into (-1 for master, >=0 for per-map)
         * @return Number of scripts successfully loaded
         */
        uint32 LoadAllScripts(int32 stateId = -1);

        /**
         * @brief Load a specific script file
         *
         * @param scriptFile Script file metadata
         * @param stateId State ID to load into (-1 for master)
         * @return true if loaded and executed successfully
         */
        bool LoadScript(const ScriptFile& scriptFile, int32 stateId = -1);

        /**
         * @brief Reload all scripts (clear cache + reload)
         *
         * @param stateId State ID to reload scripts for (-1 for master)
         * @return Number of scripts successfully reloaded
         */
        uint32 ReloadAllScripts(int32 stateId = -1);

        /**
         * @brief Setup require paths for a Lua state
         *
         * Configures package.path and package.cpath for require().
         * Adds all subdirectories recursively to search paths.
         *
         * @param state Lua state to configure
         */
        void SetupRequirePaths(sol::state& state);

    private:
        std::string m_scriptPath;   ///< Root script directory path

        /**
         * @brief Recursively scan directory for scripts
         * @param directory Directory to scan
         * @param scripts Output vector (appended to)
         */
        void ScanDirectory(const std::string& directory, std::vector<ScriptFile>& scripts);

        /**
         * @brief Execute compiled bytecode in a state
         * @param bytecode Compiled bytecode to execute
         * @param filename Script filename (for error logging)
         * @param stateId State ID to execute in
         * @return true if execution successful
         */
        bool ExecuteBytecode(const CompiledBytecode* bytecode, const std::string& filename, int32 stateId);
    };

} // namespace ALE::Core

#endif // _ALE_SCRIPT_LOADER_H
