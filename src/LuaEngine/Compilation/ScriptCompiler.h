/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _SOL_SCRIPT_COMPILER_H
#define _SOL_SCRIPT_COMPILER_H

#include <sol/sol.hpp>
#include <string>
#include <memory>
#include "BytecodeCache.h"
#include "Statistics.h"
#include "Common.h"

namespace Eclipse::API
{
    // Import Core types
    using Core::CompiledBytecode;

    /**
     * @brief Script compilation options
     */
    struct CompileOptions
    {
        bool stripDebugInfo = false;
        bool optimizeCode = true;
        bool allowMoonScript = true;
        std::string moonScriptPath;
    };

    /**
     * @class ScriptCompiler
     * @brief Compiles Lua/MoonScript files to bytecode with caching
     *
     */
    class ScriptCompiler
    {
    public:
        ScriptCompiler();
        ~ScriptCompiler();

        ScriptCompiler(const ScriptCompiler&) = delete;
        ScriptCompiler& operator=(const ScriptCompiler&) = delete;
        static ScriptCompiler& GetInstance();

        /**
         * @brief Compile a script file to bytecode
         *
         * @param filepath Path to the script file (delegates to BytecodeCache)
         * @param options Compilation options (currently unused, delegated to cache)
         * @return Const pointer to compiled bytecode, or nullptr if failed
         */
        const CompiledBytecode* CompileFile(const std::string& filepath, const CompileOptions& options = CompileOptions());

        /**
         * @brief Check if a file is a MoonScript file (.moon extension)
         *
         * @param filepath File path to check (absolute or relative)
         * @return true if extension is ".moon", false otherwise
         */
        static inline bool IsMoonScriptFile(const std::string& filepath)
        {
            return HasExtension(filepath, ".moon");
        }

        /**
         * @brief Check if file is .ext file
         * @param filepath File path to check
         * @return true if .ext extension
         */
        static inline bool IsExtFile(const std::string& filepath)
        {
            return HasExtension(filepath, ".ext");
        }

        /**
         * @brief Check if file is .cout file
         * @param filepath File path to check
         * @return true if .cout extension
         */
        static inline bool IsCoutFile(const std::string& filepath)
        {
            return HasExtension(filepath, ".cout");
        }

    private:
        /**
         * @brief Compile Lua file to bytecode (handles both .lua and .ext files)
         * @param filepath Path to .lua or .ext file
         * @return Compiled bytecode or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> CompileLuaFile(const std::string& filepath);

        /**
         * @brief Load pre-compiled .cout file (bytecode)
         * @param filepath Path to .cout file
         * @return CompiledBytecode or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> LoadCoutFile(const std::string& filepath);

        /**
         * @brief Compile MoonScript file to bytecode
         * @param filepath Path to .moon file
         * @return Compiled bytecode or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> CompileMoonScriptFile(const std::string& filepath);

        /**
         * @brief Create CompiledBytecode structure from bytecode
         * @param bc Sol2 bytecode object
         * @param filepath Original source file path
         * @return Populated CompiledBytecode shared_ptr
         */
        std::shared_ptr<CompiledBytecode> CreateBytecodeStructure(sol::bytecode&& bc, const std::string& filepath);

        /**
         * @brief Get master state with validation
         * @param callerName Name of calling function for error logging
         * @return Pointer to master state, or nullptr if unavailable (logs error)
         */
        sol::state* GetMasterStateOrNull(const char* callerName);

        /**
         * @brief Validate bytecode is non-empty
         * @param bc Bytecode to validate
         * @param filepath File path for error logging
         * @param callerName Caller function name for error logging
         * @return true if valid (non-empty)
         */
        bool ValidateBytecode(const sol::bytecode& bc, const std::string& filepath, const char* callerName);

        /**
         * @brief Check if file has specific extension
         * @param filepath File path to check
         * @param extension Extension to match (e.g., ".moon")
         * @return true if extension matches
         */
        static bool HasExtension(const std::string& filepath, const std::string& extension);
    };
}

#endif // _SOL_SCRIPT_COMPILER_H
