/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ECLIPSE_SCRIPT_LOADER_H
#define _ECLIPSE_SCRIPT_LOADER_H

#include <sol/sol.hpp>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "Common.h"

namespace Eclipse::Core
{
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
     * @brief Discovers, compiles, and executes Lua/MoonScript files
     * 
     * **Purpose:**
     * Scans lua_scripts directory, compiles scripts to bytecode via BytecodeCache,
     * and executes them in the appropriate Lua state.
     * 
     * **Architecture:**
     * - Singleton for global script management
     * - Recursive directory scanning (finds scripts in subdirectories)
     * - Bytecode caching for fast reload
     * - Package.path/cpath configuration for require()
     * 
     * **Workflow:**
     * 1. ScanScripts(): Recursively scan lua_scripts directory
     * 2. SetupRequirePaths(): Configure Lua package paths
     * 3. LoadScript(): Compile via BytecodeCache, load, execute
     * 4. ReloadAllScripts(): Clear cache, reload all
     * 
     * **Current Behavior (State -1):**
     * - All scripts loaded into master state (-1)
     * - Called once during server startup
     * - No runtime script loading
     * 
     * **Future Behavior (Multistate):**
     * - Master state: Shared libraries/utilities
     * - Per-map states: Map-specific scripts
     * - Dynamic loading when maps created
     * 
     * **Performance:**
     * - ScanScripts: O(n) where n = files in directory tree
     * - LoadScript: O(1) if bytecode cached, O(m) if compiling (m = script size)
     * - ReloadAllScripts: O(n*m) full recompilation
     * 
     * **Hidden File Handling:**
     * - Windows: Skips FILE_ATTRIBUTE_HIDDEN files
     * - Unix: Skips files starting with '.'
     * 
     * **Thread-Safety:**
     * - NOT thread-safe (initialization only)
     * - Called from single thread during server startup
     * 
     * @note Uses boost::filesystem for cross-platform file operations
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

        // Non-copyable, non-movable (singleton pattern)
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
         * Recursively scans lua_scripts directory tree.
         * Skips hidden files (Windows: FILE_ATTRIBUTE_HIDDEN, Unix: starts with '.').
         * 
         * Performance: O(n) where n = total files in directory tree.
         * 
         * @return Vector of discovered script files
         */
        std::vector<ScriptFile> ScanScripts();

        /**
         * @brief Load all scripts from directory
         * 
         * Workflow:
         * 1. Get or create Lua state
         * 2. Setup require paths
         * 3. Scan for scripts
         * 4. Load each script (compile + execute)
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
         * Loads single script:
         * 1. Get bytecode from BytecodeCache (compile if needed)
         * 2. Load bytecode into state
         * 3. Execute loaded function
         * 
         * Performance: O(1) if bytecode cached, O(n) if compiling.
         * 
         * @param scriptFile Script file metadata
         * @param stateId State ID to load into (-1 for master)
         * @return true if loaded and executed successfully
         */
        bool LoadScript(const ScriptFile& scriptFile, int32 stateId = -1);

        /**
         * @brief Reload all scripts (clear cache + reload)
         * 
         * Full reload workflow:
         * 1. Clear BytecodeCache (force recompilation)
         * 2. Destroy and recreate state (if per-map state)
         * 3. LoadAllScripts()
         * 
         * Performance: O(n*m) where n = scripts, m = avg script size.
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
         * Example paths added:
         * - lua_scripts/?.lua
         * - lua_scripts/?/init.lua
         * - lua_scripts/subdir/?.lua
         * - lua_scripts/?.dll (Windows) or ?.so (Unix)
         * 
         * @param state Lua state to configure
         */
        void SetupRequirePaths(sol::state& state);

    private:
        std::string m_scriptPath;   ///< Root script directory path
        std::string m_requirePath;  ///< Cached Lua require path (unused currently)
        std::string m_requireCPath; ///< Cached Lua require cpath (unused currently)
        // NOTE: All statistics tracked in Eclipse::Statistics::EclipseStatistics

        /**
         * @brief Recursively scan directory for scripts
         * 
         * Internal recursive helper for ScanScripts().
         * 
         * @param directory Directory to scan
         * @param scripts Output vector (appended to)
         */
        void ScanDirectory(const std::string& directory, std::vector<ScriptFile>& scripts);

        /**
         * @brief Check if file is a valid script file
         * 
         * Accepts: .ext, .lua, .moon, .cout
         * 
         * @param filename Filename to check
         * @param extension Output: extracted extension if valid
         * @return true if valid script file extension
         */
        bool IsScriptFile(const std::string& filename, std::string& extension);

        /**
         * @brief Get Lua state with validation
         * @param stateId State ID (-1 for master, >=0 for per-map)
         * @param callerName Caller function name for error logging
         * @return Pointer to state, or nullptr if unavailable (logs error)
         */
        sol::state* GetStateOrNull(int32 stateId, const char* callerName);
    };

} // namespace Eclipse::Core

#endif // _ECLIPSE_SCRIPT_LOADER_H
