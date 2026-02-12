/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_LUA_GUARD_H
#define _ALE_LUA_GUARD_H

#include <mutex>

namespace ALE::Core
{
    /**
     * @brief Global recursive mutex for Lua state access serialization.
     *
     * @return Reference to the global Lua mutex
     */
    inline std::recursive_mutex& GetLuaMutex()
    {
        static std::recursive_mutex s_luaMutex;
        return s_luaMutex;
    }

    /**
     * @class LuaGuard
     * @brief RAII guard for Lua state access.
     *
     * Without this guard, concurrent map update threads can access
     * the same Lua state simultaneously, causing segmentation faults.
     */
    class LuaGuard
    {
    public:
        LuaGuard() : _lock(GetLuaMutex()) {}

    private:
        std::lock_guard<std::recursive_mutex> _lock;
    };

} // namespace ALE::Core

#endif // _ALE_LUA_GUARD_H
