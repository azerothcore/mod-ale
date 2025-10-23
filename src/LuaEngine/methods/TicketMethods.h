/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef TICKETMETHODS_H
#define TICKETMETHODS_H

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
    int IsClosed(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->IsClosed());
        return 1;
    }

    /**
     * Returns true if the [Ticket] is completed or false.
     *
     * @return bool isCompleted
     */
    int IsCompleted(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->IsCompleted());
        return 1;
    }

    /**
     * Return true if this GUID is the same as the [Player] who created the [Ticket] or false.
     *
     * @param ObjectGuid playerGuid
     *
     * @return bool isSamePlayer
     */
    int IsFromPlayer(lua_State* L, GmTicket* ticket)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);

        ALE::Push(L, ticket->IsFromPlayer(guid));
        return 1;
    }

    /**
     * Return true if the [Ticket] is assigned or false.
     *
     * @return bool isAssigned
     */
    int IsAssigned(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->IsAssigned());
        return 1;
    }

    /**
     * Return true if the [Ticket] is assigned to the [Player] or false.
     *
     * @param ObjectGuid playerGuid
     *
     * @return bool isAssignedTo
     */
    int IsAssignedTo(lua_State* L, GmTicket* ticket)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);

        ALE::Push(L, ticket->IsAssignedTo(guid));
        return 1;
    }

    /**
     * Return true if the [Ticket] is not assigned to the [Player] or false.
     *
     * @param ObjectGuid playerGuid
     *
     * @return bool isAssignedNotTo
     */
    int IsAssignedNotTo(lua_State* L, GmTicket* ticket)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);

        ALE::Push(L, ticket->IsAssignedNotTo(guid));
        return 1;
    }

    /**
     * Return the [Ticket] id.
     *
     * @return uint32 ticketId
     */
    int GetId(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetId());
        return 1;
    }

    /**
     * Return the [Player] from the [Ticket].
     *
     * @return [Player] player
     */
    int GetPlayer(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetPlayer());
        return 1;
    }

    /**
     * Return the [Player] name from the [Ticket].
     *
     * @return string playerName
     */
    int GetPlayerName(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetPlayerName());
        return 1;
    }

    /**
     * Returns the message sent in the [Ticket].
     *
     * @return string message
     */
    int GetMessage(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetMessage());
        return 1;
    }

    /**
     * Returns the assigned [Player].
     *
     * @return [Player] assignedPlayer
     */
    int GetAssignedPlayer(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetAssignedPlayer());
        return 1;
    }

    /**
     * Returns the assigned guid.
     *
     * @return uint32 assignedGuid
     */
    int GetAssignedToGUID(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetAssignedToGUID());
        return 1;
    }

    /**
     * Returns the last modified time from the [Ticket].
     *
     * @return uint64 lastModifiedTime
     */
    int GetLastModifiedTime(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetLastModifiedTime());
        return 1;
    }

    /**
     * Assign the [Ticket] to a player via his GUID.
     *
     * @param ObjectGuid playerGuid
     * @param bool isAdmin : true if the [Player] is an Admin or false (default false)
     */
    int SetAssignedTo(lua_State* L, GmTicket* ticket)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        bool is_admin = ALE::CHECKVAL<bool>(L, 2, false);
        ticket->SetAssignedTo(guid, is_admin);
        return 0;
    }

    /**
     * Set [Ticket] resolved by player via his GUID.
     *
     * @param ObjectGuid playerGuid
     */
    int SetResolvedBy(lua_State* L, GmTicket* ticket)
    {
        ObjectGuid guid = ALE::CHECKVAL<ObjectGuid>(L, 2);
        ticket->SetResolvedBy(guid);
        return 0;
    }

    /**
     * Set [Ticket] completed.
     *
     */
    int SetCompleted(lua_State* /*L*/, GmTicket* ticket)
    {
        ticket->SetCompleted();
        return 0;
    }

    /**
     * Set [Ticket] message.
     *
     * @param string message: desired message
     *
     */
    int SetMessage(lua_State* L, GmTicket* ticket)
    {
        std::string message = ALE::CHECKVAL<std::string>(L, 2);

        ticket->SetMessage(message);
        return 0;
    }

    /**
     * Set [Ticket] comment.
     *
     * @param string comment: desired comment
     *
     */
    int SetComment(lua_State* L, GmTicket* ticket)
    {
        std::string comment = ALE::CHECKVAL<std::string>(L, 2);

        ticket->SetComment(comment);
        return 0;
    }

    /**
     * Set [Ticket] as viewed.
     *
     */
    int SetViewed(lua_State* /*L*/, GmTicket* ticket)
    {
        ticket->SetViewed();
        return 0;
    }

    /**
     * Set [Ticket] as unassigned.
     *
     */
    int SetUnassigned(lua_State* /*L*/, GmTicket* ticket)
    {
        ticket->SetUnassigned();
        return 0;
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
    int SetPosition(lua_State* L, GmTicket* ticket)
    {
        uint32 mapId = ALE::CHECKVAL<uint32>(L, 2);
        float x = ALE::CHECKVAL<float>(L, 2);
        float y = ALE::CHECKVAL<float>(L, 2);
        float z = ALE::CHECKVAL<float>(L, 2);

        ticket->SetPosition(mapId, x, y, z);
        return 0;
    }

    /**
     * Adds a response to the [Ticket].
     *
     * @param string response: desired response
     *
     */
    int AppendResponse(lua_State* L, GmTicket* ticket)
    {
        std::string response = ALE::CHECKVAL<std::string>(L, 2);

        ticket->AppendResponse(response);
        return 0;
    }

    /**
     * Return the [Ticket] response.
     *
     * @return string response
     */
    int GetResponse(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetResponse());
        return 1;
    }

    /**
     * Delete the [Ticket] response.
     *
     */
    int DeleteResponse(lua_State* /*L*/, GmTicket* ticket)
    {
        ticket->DeleteResponse();
        return 0;
    }

    /**
     * Return the [Ticket] chatlog.
     *
     * @return string chatlog
     */
    int GetChatLog(lua_State* L, GmTicket* ticket)
    {
        ALE::Push(L, ticket->GetChatLog());
        return 1;
    }
};
#endif

