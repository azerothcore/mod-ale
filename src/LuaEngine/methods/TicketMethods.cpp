/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "TicketMgr.h"

/***
 * Represents a support ticket created by a [Player] using the in-game ticket system.
 *
 * Inherits all methods from: none
 */
namespace LuaTicket
{
    /**
     * Returns true if the [Ticket] is closed or false.
     *
     * @return bool isClosed
     */
    bool IsClosed(GmTicket* ticket)
    {
        return ticket->IsClosed();
    }

    /**
     * Returns true if the [Ticket] is completed or false.
     *
     * @return bool isCompleted
     */
    bool IsCompleted(GmTicket* ticket)
    {
        return ticket->IsCompleted();
    }

    /**
     * Return true if this GUID is the same as the [Player] who created the [Ticket] or false.
     *
     * @param ObjectGuid playerGuid
     *
     * @return bool isSamePlayer
     */
    bool IsFromPlayer(GmTicket* ticket, ObjectGuid guid)
    {
        return ticket->IsFromPlayer(guid);
    }

    /**
     * Return true if the [Ticket] is assigned or false.
     *
     * @return bool isAssigned
     */
    bool IsAssigned(GmTicket* ticket)
    {
        return ticket->IsAssigned();
    }

    /**
     * Return true if the [Ticket] is assigned to the [Player] or false.
     *
     * @param ObjectGuid playerGuid
     *
     * @return bool isAssignedTo
     */
    bool IsAssignedTo(GmTicket* ticket, ObjectGuid guid)
    {
        return ticket->IsAssignedTo(guid);
    }

    /**
     * Return true if the [Ticket] is not assigned to the [Player] or false.
     *
     * @param ObjectGuid playerGuid
     *
     * @return bool isAssignedNotTo
     */
    bool IsAssignedNotTo(GmTicket* ticket, ObjectGuid guid)
    {
        return ticket->IsAssignedNotTo(guid);
    }

    /**
     * Return the [Ticket] id.
     *
     * @return uint32 ticketId
     */
    uint32 GetId(GmTicket* ticket)
    {
        return ticket->GetId();
    }

    /**
     * Return the [Player] from the [Ticket].
     *
     * @return [Player] player
     */
    Player* GetPlayer(GmTicket* ticket)
    {
        return ticket->GetPlayer();
    }

    /**
     * Return the [Player] name from the [Ticket].
     *
     * @return string playerName
     */
    std::string GetPlayerName(GmTicket* ticket)
    {
        return ticket->GetPlayerName();
    }

    /**
     * Returns the message sent in the [Ticket].
     *
     * @return string message
     */
    std::string GetMessage(GmTicket* ticket)
    {
        return ticket->GetMessage();
    }

    /**
     * Returns the assigned [Player].
     *
     * @return [Player] assignedPlayer
     */
    Player* GetAssignedPlayer(GmTicket* ticket)
    {
        return ticket->GetAssignedPlayer();
    }

    /**
     * Returns the assigned guid.
     *
     * @return uint32 assignedGuid
     */
    ObjectGuid GetAssignedToGUID(GmTicket* ticket)
    {
        return ticket->GetAssignedToGUID();
    }

    /**
     * Returns the last modified time from the [Ticket].
     *
     * @return uint64 lastModifiedTime
     */
    uint64 GetLastModifiedTime(GmTicket* ticket)
    {
        return ticket->GetLastModifiedTime();
    }

    /**
     * Assign the [Ticket] to a player via his GUID.
     *
     * @param ObjectGuid playerGuid
     * @param bool isAdmin : true if the [Player] is an Admin or false (default false)
     */
    void SetAssignedTo(GmTicket* ticket, ObjectGuid guid, sol::optional<bool> isAdmin)
    {
        ticket->SetAssignedTo(guid, isAdmin.value_or(false));
    }

    /**
     * Set [Ticket] resolved by player via his GUID.
     *
     * @param ObjectGuid playerGuid
     */
    void SetResolvedBy(GmTicket* ticket, ObjectGuid guid)
    {
        ticket->SetResolvedBy(guid);
    }

    /**
     * Set [Ticket] completed.
     *
     */
    void SetCompleted(GmTicket* ticket)
    {
        ticket->SetCompleted();
    }

    /**
     * Set [Ticket] message.
     *
     * @param string message: desired message
     *
     */
    void SetMessage(GmTicket* ticket, std::string message)
    {
        ticket->SetMessage(message);
    }

    /**
     * Set [Ticket] comment.
     *
     * @param string comment: desired comment
     *
     */
    void SetComment(GmTicket* ticket, std::string comment)
    {
        ticket->SetComment(comment);
    }

    /**
     * Set [Ticket] as viewed.
     *
     */
    void SetViewed(GmTicket* ticket)
    {
        ticket->SetViewed();
    }

    /**
     * Set [Ticket] as unassigned.
     *
     */
    void SetUnassigned(GmTicket* ticket)
    {
        ticket->SetUnassigned();
    }

    /**
     * Set the new [Ticket] creation position.
     *
     * @param uint32 mapId
     * @param float x
     * @param float y
     * @param float z
     *
     */
    void SetPosition(GmTicket* ticket, uint32 mapId, float x, float y, float z)
    {
        ticket->SetPosition(mapId, x, y, z);
    }

    /**
     * Adds a response to the [Ticket].
     *
     * @param string response: desired response
     *
     */
    void AppendResponse(GmTicket* ticket, std::string response)
    {
        ticket->AppendResponse(response);
    }

    /**
     * Return the [Ticket] response.
     *
     * @return string response
     */
    std::string GetResponse(GmTicket* ticket)
    {
        return ticket->GetResponse();
    }

    /**
     * Delete the [Ticket] response.
     *
     */
    void DeleteResponse(GmTicket* ticket)
    {
        ticket->DeleteResponse();
    }

    /**
     * Return the [Ticket] chatlog.
     *
     * @return string chatlog
     */
    std::string GetChatLog(GmTicket* ticket)
    {
        return ticket->GetChatLog();
    }
}

void RegisterTicketMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<GmTicket>> type = ALEBind::NewHandleType<ScopedRef<GmTicket>>(lua, "Ticket");

    type["IsClosed"]            = ALEBind::Method(&LuaTicket::IsClosed);
    type["IsCompleted"]         = ALEBind::Method(&LuaTicket::IsCompleted);
    type["IsFromPlayer"]        = ALEBind::Method(&LuaTicket::IsFromPlayer);
    type["IsAssigned"]          = ALEBind::Method(&LuaTicket::IsAssigned);
    type["IsAssignedTo"]        = ALEBind::Method(&LuaTicket::IsAssignedTo);
    type["IsAssignedNotTo"]     = ALEBind::Method(&LuaTicket::IsAssignedNotTo);
    type["GetId"]               = ALEBind::Method(&LuaTicket::GetId);
    type["GetPlayer"]           = ALEBind::Method(&LuaTicket::GetPlayer);
    type["GetPlayerName"]       = ALEBind::Method(&LuaTicket::GetPlayerName);
    type["GetMessage"]          = ALEBind::Method(&LuaTicket::GetMessage);
    type["GetAssignedPlayer"]   = ALEBind::Method(&LuaTicket::GetAssignedPlayer);
    type["GetAssignedToGUID"]   = ALEBind::Method(&LuaTicket::GetAssignedToGUID);
    type["GetLastModifiedTime"] = ALEBind::Method(&LuaTicket::GetLastModifiedTime);
    type["SetAssignedTo"]       = ALEBind::Method(&LuaTicket::SetAssignedTo);
    type["SetResolvedBy"]       = ALEBind::Method(&LuaTicket::SetResolvedBy);
    type["SetCompleted"]        = ALEBind::Method(&LuaTicket::SetCompleted);
    type["SetMessage"]          = ALEBind::Method(&LuaTicket::SetMessage);
    type["SetComment"]          = ALEBind::Method(&LuaTicket::SetComment);
    type["SetViewed"]           = ALEBind::Method(&LuaTicket::SetViewed);
    type["SetUnassigned"]       = ALEBind::Method(&LuaTicket::SetUnassigned);
    type["SetPosition"]         = ALEBind::Method(&LuaTicket::SetPosition);
    type["AppendResponse"]      = ALEBind::Method(&LuaTicket::AppendResponse);
    type["GetResponse"]         = ALEBind::Method(&LuaTicket::GetResponse);
    type["DeleteResponse"]      = ALEBind::Method(&LuaTicket::DeleteResponse);
    type["GetChatLog"]          = ALEBind::Method(&LuaTicket::GetChatLog);
}
