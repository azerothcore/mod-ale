/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Chat.h"

/***
 * Provides access to in-game and console chat commands, messages, and selection context for command execution.
 *
 * Used primarily in GM scripts or command handlers to send messages, check permissions, and access selected targets.
 *
 * Inherits all methods from: none
 */
namespace LuaChatHandler
{
    /**
     * Sends text to the chat handler
     *
     * @proto (text)
     * @proto (entry)
     * @param string text : text to display in chat or console
     * @param uint32 entry : id of the string to display
     */
    void SendSysMessage(ChatHandler* handler, sol::object msg)
    {
        if (msg.get_type() == sol::type::number)
        {
            uint32 entry = msg.as<uint32>();
            handler->SendSysMessage(entry);
        }
        else
        {
            std::string text = msg.as<std::string>();
            handler->SendSysMessage(text);
        }
    }

    /**
     * Returns `true` if the [ChatHandler] comes from the console, `false` if it comes from a player
     *
     * @return bool isConsole
     */
    bool IsConsole(ChatHandler* handler)
    {
        return handler->IsConsole();
    }

    /**
     * Returns the [Player] associated with the handler. Returns `nil` in the case of a console handler
     *
     * @return [Player] player
     */
    Player* GetPlayer(ChatHandler* handler)
    {
        return handler->GetPlayer();
    }

    /**
     * Sends a message to all connected players
     *
     * @param string text : text to send
     */
    void SendGlobalSysMessage(ChatHandler* handler, std::string text)
    {
        handler->SendGlobalSysMessage(text.c_str());
    }

    /**
     * Sends a message to all connected Game Masters
     *
     * @param string text : text to send
     */
    void SendGlobalGMSysMessage(ChatHandler* handler, std::string text)
    {
        handler->SendGlobalGMSysMessage(text.c_str());
    }

    /**
     * Checks if the current security level is lower than the specified [Player]'s account
     *
     * @param [Player] player
     * @param [bool] strong = false : Forces non-player accounts (security level greater than `0`) to go through the regular check if set to `true`.<br>Also, if set to `true`, the current security level will be considered as lower than the [Player]'s security level if the two levels are equal
     * @return [bool] lower
     */
    bool HasLowerSecurity(ChatHandler* handler, Player* player, sol::optional<bool> strong)
    {
        return handler->HasLowerSecurity(player, ObjectGuid::Empty, strong.value_or(false));
    }

    /**
     * Checks if the current security level is lower than the specified `account`'s level
     *
     * @param [uint32] account : the target account ID to compare security levels with
     * @param [bool] strong = false : Forces non-player accounts (security level greater than `0`) to go through the regular check if set to `true`.<br>Also, if set to `true`, the current security level will be considered as lower than the `account`'s security level if the two levels are equal
     * @return [bool] lower
     */
    bool HasLowerSecurityAccount(ChatHandler* handler, uint32 account, sol::optional<bool> strong)
    {
        return handler->HasLowerSecurityAccount(nullptr, account, strong.value_or(false));
    }

    /**
     * Returns the selected [Player]
     *
     * @return [Player] player
     */
    Player* GetSelectedPlayer(ChatHandler* handler)
    {
        return handler->getSelectedPlayer();
    }

    /**
     * Returns the selected [Creature]
     *
     * @return [Creature] creature
     */
    Creature* GetSelectedCreature(ChatHandler* handler)
    {
        return handler->getSelectedCreature();
    }

    /**
     * Returns the selected [Unit]
     *
     * @return [Unit] unit
     */
    Unit* GetSelectedUnit(ChatHandler* handler)
    {
        return handler->getSelectedUnit();
    }

    /**
     * Returns the selected [WorldObject]
     *
     * @return [WorldObject] object
     */
    WorldObject* GetSelectedObject(ChatHandler* handler)
    {
        return handler->getSelectedObject();
    }

    /**
     * Returns the selected [Player] or the current [Player] if nothing is targeted or the target is not a player
     *
     * @return [Player] player
     */
    Player* GetSelectedPlayerOrSelf(ChatHandler* handler)
    {
        return handler->getSelectedPlayerOrSelf();
    }

    /**
     * Checks if the `securityLevel` is available
     *
     * @param [uint32] securityLevel
     * @return [bool] isAvailable
     */
    bool IsAvailable(ChatHandler* handler, uint32 securityLevel)
    {
        return handler->IsAvailable(securityLevel);
    }

    /**
     * Returns `true` if other previously called [ChatHandler] methods sent an error
     *
     * @return [bool] sentErrorMessage
     */
    bool HasSentErrorMessage(ChatHandler* handler)
    {
        return handler->HasSentErrorMessage();
    }
}

void RegisterChatHandlerMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<ChatHandler>> type = ALEBind::NewHandleType<ScopedRef<ChatHandler>>(lua, "ChatHandler");

    type["SendSysMessage"]          = ALEBind::Method(&LuaChatHandler::SendSysMessage);
    type["IsConsole"]               = ALEBind::Method(&LuaChatHandler::IsConsole);
    type["GetPlayer"]               = ALEBind::Method(&LuaChatHandler::GetPlayer);
    type["SendGlobalSysMessage"]    = ALEBind::Method(&LuaChatHandler::SendGlobalSysMessage);
    type["SendGlobalGMSysMessage"]  = ALEBind::Method(&LuaChatHandler::SendGlobalGMSysMessage);
    type["HasLowerSecurity"]        = ALEBind::Method(&LuaChatHandler::HasLowerSecurity);
    type["HasLowerSecurityAccount"] = ALEBind::Method(&LuaChatHandler::HasLowerSecurityAccount);
    type["GetSelectedPlayer"]       = ALEBind::Method(&LuaChatHandler::GetSelectedPlayer);
    type["GetSelectedCreature"]     = ALEBind::Method(&LuaChatHandler::GetSelectedCreature);
    type["GetSelectedUnit"]         = ALEBind::Method(&LuaChatHandler::GetSelectedUnit);
    type["GetSelectedObject"]       = ALEBind::Method(&LuaChatHandler::GetSelectedObject);
    type["GetSelectedPlayerOrSelf"] = ALEBind::Method(&LuaChatHandler::GetSelectedPlayerOrSelf);
    type["IsAvailable"]             = ALEBind::Method(&LuaChatHandler::IsAvailable);
    type["HasSentErrorMessage"]     = ALEBind::Method(&LuaChatHandler::HasSentErrorMessage);
}
