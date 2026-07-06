/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_SCRIPT_CACHE_H
#define _ALE_SCRIPT_CACHE_H

#include "Common.h"

#include <sol/sol.hpp>

#include <ctime>
#include <string>

/*
 * Script loading with an optional global bytecode cache.
 *
 * Compiling a large script collection on every reload is slow, so when
 * `ALE.BytecodeCache` is enabled, compiled bytecode is kept in a cache that
 * survives Lua state reloads and is only invalidated when the file on disk
 * changes. MoonScript sources are compiled through the `moonscript` Lua module
 * and cached the same way.
 */
namespace ALEScriptCache
{
    /*
     * Loads a Lua or MoonScript file as a callable chunk, going through the
     * bytecode cache when it is enabled.
     *
     * Increments `compiledCount` / `cachedCount` for the reload statistics.
     * Returns an invalid function on failure (the error is logged).
     */
    sol::protected_function Load(sol::state& lua, std::string const& filepath, bool isMoonScript,
        uint32& compiledCount, uint32& cachedCount);

    /*
     * Loads a precompiled (.out, luac output) script file as a callable chunk.
     * Returns an invalid function on failure (the error is logged).
     */
    sol::protected_function LoadCompiled(sol::state& lua, std::string const& filepath);

    // Drops all cached bytecode (called on shutdown).
    void ClearCache();

    // Drops the cached file timestamps so the next load re-checks the disk
    // (called at the start of every reload).
    void ClearTimestamps();
}

#endif // _ALE_SCRIPT_CACHE_H
