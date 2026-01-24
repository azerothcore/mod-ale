/*
 * Copyright (C) 2010 - 2026 ALE - AzerothCore Lua Engine
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#ifndef _ALE_HOOKS_H
#define _ALE_HOOKS_H

#include <sol/sol.hpp>
#include <unordered_map>
#include <vector>

namespace ALE::Hooks
{
    /**
     * @enum PlayerEvent
     * @brief Player-specific event types
     *
     * Triggered by PlayerHooks when player state changes occur.
     */
    enum class PlayerEvent : uint32
    {
        ON_CHARACTER_CREATE = 1,  // < Character created in database
        ON_CHARACTER_DELETE = 2,  // < Character deleted from database
        ON_LOGIN = 3,             // < Player logged into the world
        ON_LOGOUT = 4,            // < Player logged out
        ON_FIRST_LOGIN = 5,       // < First time login for character
        ON_LEVEL_CHANGE = 6,      // < Player gained/lost a level
        ON_MONEY_CHANGE = 7,      // < Player money changed
        ON_GIVE_XP = 8,           // < Player received experience
        ON_REPUTATION_CHANGE = 9, // < Reputation with faction changed
        ON_KILL_PLAYER = 10,      // < Player killed another player
        ON_KILL_CREATURE = 11     // < Player killed a creature
    };

    /**
     * @enum WorldEvent
     * @brief World-level event types
     *
     * Triggered by WorldHooks for server-wide events.
     */
    enum class WorldEvent : uint32
    {
        ON_UPDATE = 1,              // < World update tick
        ON_CONFIG_LOAD = 2,         // < Configuration reloaded
        ON_SHUTDOWN_INIT = 3,       // < Shutdown initiated
        ON_SHUTDOWN_CANCEL = 4,     // < Shutdown cancelled
        ON_STARTUP = 14,            // < Server startup complete
        ON_SHUTDOWN = 6             // < Server shutting down
    };

    /**
     * @enum WorldObjectEvent
     * @brief WorldObject lifecycle event types
     *
     * Triggered by WorldObjectHooks for GameObjects.
     */
    enum class WorldObjectEvent : uint32
    {
        ON_CREATE = 1,  // < GameObject created in world
        ON_DESTROY = 2, // < GameObject removed from world
        ON_UPDATE = 3,  // < GameObject updated
        ON_REPOP = 4    // < GameObject respawned
    };
}

#endif // _ALE_HOOKS_H