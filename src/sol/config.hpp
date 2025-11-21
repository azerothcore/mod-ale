// The MIT License (MIT)

// Copyright (c) 2013-2020 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// This file was generated with a script.
// Generated 2022-06-25 08:14:19.336233 UTC
// This header was generated with sol v3.3.0 (revision eba86625)
// https://github.com/ThePhD/sol2

#ifndef SOL_SINGLE_CONFIG_HPP
#define SOL_SINGLE_CONFIG_HPP

// beginning of sol/config.hpp

/* AzerothCore mod-ale sol2 configuration
 *
 * This file configures sol2 for use with mod-ale (AzerothCore Lua Engine).
 *
 */

// ============================================================================
// Lua Compatibility Flags
// ============================================================================
#define LUA_COMPAT_ALL 1
#define LUA_COMPAT_APIINTCASTS 1

// ============================================================================
// Lua Headers
// ============================================================================

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

// ============================================================================
// Lua Version Detection
// ============================================================================

// Detect LuaJIT
#if defined(LUAJIT_VERSION)
    #define SOL_LUAJIT 1
    #define SOL_USING_CXX_LUAJIT 1
    #define SOL_LUAJIT_VERSION 20100
    #define SOL_LUA_VERSION 501
#else
    #define SOL_LUAJIT 0
    #define SOL_USING_CXX_LUAJIT 0

    // Detect Lua 5.1
    #if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 501
        #define SOL_LUA_VERSION 501

    // Detect Lua 5.2
    #elif defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 502
        #define SOL_LUA_VERSION 502

    // Detect Lua 5.3
    #elif defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 503
        #define SOL_LUA_VERSION 503

    // Detect Lua 5.4
    #elif defined(LUA_VERSION_NUM) && LUA_VERSION_NUM == 504
        #define SOL_LUA_VERSION 504
    #endif
#endif

// ============================================================================
// Sol2 Safety Settings
// ============================================================================

// Enable safe function calls (catches errors instead of crashing)
#define SOL_SAFE_FUNCTION_CALLS 1

// Enable exception handling trampoline
#define SOL_FUNCTION_CALL_TRAMPOLINE 1

// ============================================================================
// Sol2 Performance Optimizations
// ============================================================================

// Optimize for single return values (most common case)
#define SOL_OPTIMIZE_FOR_SINGLE_RETURN 1

// Don't check number precision (faster, but less safe)
#define SOL_NO_CHECK_NUMBER_PRECISION 1

// Optimize stack string operations (2KB buffer)
#define SOL_STACK_STRING_OPTIMIZATION_SIZE 2048

// Enable fast type checking
#define SOL_FAST_TYPE_CHECKING 1

// Minimize usertype size for better memory usage
#define SOL_MINIMIZE_USERTYPE_SIZE 1

// ============================================================================
// Sol2 Container Support
// ============================================================================

// Enable container traits (support for std::vector, etc.)
#define SOL_CONTAINER_TRAITS 1

// Enable container start (required for container support)
#define SOL_CONTAINERS_START 1

// ============================================================================
// Sol2 Compatibility Settings
// ============================================================================

// Use compatibility layer for different Lua versions
#define SOL_USE_COMPATIBILITY_LAYER 1

// Don't disable compatibility features
#define SOL_NO_COMPAT 0

// end of sol/config.hpp

#endif // SOL_SINGLE_CONFIG_HPP
