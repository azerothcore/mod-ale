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
#include "Common.h"

namespace ALE::API
{
    // Import Core types
    using Core::CompiledBytecode;

    /**
     * @class ScriptCompiler
     * @brief Pure script compilation (no caching)
     *
     * Compiles Lua/MoonScript/Cout files to bytecode.
     * Does not handle caching - that's ScriptLoader's responsibility.
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
         * Routes to appropriate compiler based on file extension:
         * - .cout → LoadCoutFile()
         * - .moon → CompileMoonScriptFile()
         * - .lua/.ext → CompileLuaFile()
         *
         * @param filepath Path to script file
         * @return Compiled bytecode, or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> Compile(const std::string& filepath);

    private:
        /**
         * @brief Compile Lua file to bytecode
         * @param filepath Path to .lua or .ext file
         * @return Compiled bytecode or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> CompileLuaFile(const std::string& filepath);

        /**
         * @brief Load pre-compiled .cout file
         * @param filepath Path to .cout file
         * @return Loaded bytecode or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> LoadCoutFile(const std::string& filepath);

        /**
         * @brief Compile MoonScript file to bytecode
         * @param filepath Path to .moon file
         * @return Compiled bytecode or nullptr if failed
         */
        std::shared_ptr<CompiledBytecode> CompileMoonScriptFile(const std::string& filepath);

        /**
         * @brief Create CompiledBytecode structure from sol::bytecode
         * @param bc Sol2 bytecode object
         * @param filepath Original source file path
         * @return Populated CompiledBytecode
         */
        std::shared_ptr<CompiledBytecode> CreateBytecode(sol::bytecode&& bc, const std::string& filepath);

        /**
         * @brief Validate bytecode is non-empty
         * @param bc Bytecode to validate
         * @param filepath File path for error logging
         * @return true if valid
         */
        bool ValidateBytecode(const sol::bytecode& bc, const std::string& filepath);
    };
}

#endif // _SOL_SCRIPT_COMPILER_H
