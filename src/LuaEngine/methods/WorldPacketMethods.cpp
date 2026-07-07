/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Opcodes.h"
#include "WorldPacket.h"

/***
 * A packet used to pass messages between the server and a client.
 *
 * Each packet has an opcode that determines the type of message being sent,
 *   e.g. if a CMSG_LOGOUT_REQUEST packet is sent to the server,
 *   the client has sent a message that its [Player] wants to logout.
 *
 * The packet can contain further data, the format of which depends on the opcode.
 *
 * Inherits all methods from: none
 */
namespace LuaWorldPacket
{
    /**
     * Returns the opcode of the [WorldPacket].
     *
     * @return uint16 opcode
     */
    uint16 GetOpcode(WorldPacket* packet)
    {
        return packet->GetOpcode();
    }

    /**
     * Returns the size of the [WorldPacket].
     *
     * @return uint32 size
     */
    uint32 GetSize(WorldPacket* packet)
    {
        return packet->size();
    }

    /**
     * Sets the opcode of the [WorldPacket] to the specified opcode.
     *
     * @param [Opcodes] opcode : see Opcodes.h for all known opcodes
     */
    void SetOpcode(WorldPacket* packet, uint32 opcode)
    {
        if (opcode >= NUM_MSG_TYPES)
            throw std::invalid_argument("valid opcode expected");
        packet->SetOpcode(static_cast<Opcodes>(opcode));
    }

    /**
     * Reads and returns a signed 8-bit integer value from the [WorldPacket].
     *
     * @return int8 value
     */
    int8 ReadByte(WorldPacket* packet)
    {
        int8 _byte;
        (*packet) >> _byte;
        return _byte;
    }

    /**
     * Reads and returns an unsigned 8-bit integer value from the [WorldPacket].
     *
     * @return uint8 value
     */
    uint8 ReadUByte(WorldPacket* packet)
    {
        uint8 _ubyte;
        (*packet) >> _ubyte;
        return _ubyte;
    }

    /**
     * Reads and returns a signed 16-bit integer value from the [WorldPacket].
     *
     * @return int16 value
     */
    int16 ReadShort(WorldPacket* packet)
    {
        int16 _short;
        (*packet) >> _short;
        return _short;
    }

    /**
     * Reads and returns an unsigned 16-bit integer value from the [WorldPacket].
     *
     * @return uint16 value
     */
    uint16 ReadUShort(WorldPacket* packet)
    {
        uint16 _ushort;
        (*packet) >> _ushort;
        return _ushort;
    }

    /**
     * Reads and returns a signed 32-bit integer value from the [WorldPacket].
     *
     * @return int32 value
     */
    int32 ReadLong(WorldPacket* packet)
    {
        int32 _long;
        (*packet) >> _long;
        return _long;
    }

    /**
     * Reads and returns an unsigned 32-bit integer value from the [WorldPacket].
     *
     * @return uint32 value
     */
    uint32 ReadULong(WorldPacket* packet)
    {
        uint32 _ulong;
        (*packet) >> _ulong;
        return _ulong;
    }

    /**
     * Reads and returns a single-precision floating-point value from the [WorldPacket].
     *
     * @return float value
     */
    float ReadFloat(WorldPacket* packet)
    {
        float _val;
        (*packet) >> _val;
        return _val;
    }

    /**
     * Reads and returns a double-precision floating-point value from the [WorldPacket].
     *
     * @return double value
     */
    double ReadDouble(WorldPacket* packet)
    {
        double _val;
        (*packet) >> _val;
        return _val;
    }

    /**
     * Reads and returns an unsigned 64-bit integer value from the [WorldPacket].
     *
     * @return ObjectGuid value : value returned as string
     */
    ObjectGuid ReadGUID(WorldPacket* packet)
    {
        ObjectGuid guid;
        (*packet) >> guid;
        return guid;
    }

    /**
     * Reads a packed GUID from the [WorldPacket] and returns it as a full 64-bit integer.
     * The packed data size varies (2-9 bytes), but always unpacks to a complete 64-bit GUID.
     *
     * @return uint64 value : value returned as string
     */
    uint64 ReadPackedGUID(WorldPacket* packet)
    {
        uint64 guid;
        packet->readPackGUID(guid);
        return guid;
    }

    /**
     * Reads and returns a string value from the [WorldPacket].
     *
     * @return string value
     */
    std::string ReadString(WorldPacket* packet)
    {
        std::string _val;
        (*packet) >> _val;
        return _val;
    }

    /**
     * Writes an unsigned 64-bit integer value to the [WorldPacket].
     *
     * @param ObjectGuid value : the value to be written to the [WorldPacket]
     */
    void WriteGUID(WorldPacket* packet, ObjectGuid guid)
    {
        (*packet) << guid;
    }

    /**
     * Writes an ObjectGuid as packed GUID format to the [WorldPacket].
     *
     * @param ObjectGuid value : the ObjectGuid to be packed to the [WorldPacket]
     */
    void WritePackedGUID(WorldPacket* packet, ObjectGuid guid)
    {
        PackedGuid packedGuid(guid);
        (*packet) << packedGuid;
    }

    /**
     * Writes a string to the [WorldPacket].
     *
     * @param string value : the string to be written to the [WorldPacket]
     */
    void WriteString(WorldPacket* packet, std::string _val)
    {
        (*packet) << _val;
    }

    /**
     * Writes a signed 8-bit integer value to the [WorldPacket].
     *
     * @param int8 value : the int8 value to be written to the [WorldPacket]
     */
    void WriteByte(WorldPacket* packet, int8 byte)
    {
        (*packet) << byte;
    }

    /**
     * Writes an unsigned 8-bit integer value to the [WorldPacket].
     *
     * @param uint8 value : the uint8 value to be written to the [WorldPacket]
     */
    void WriteUByte(WorldPacket* packet, uint8 byte)
    {
        (*packet) << byte;
    }

    /**
     * Writes a signed 16-bit integer value to the [WorldPacket].
     *
     * @param int16 value : the int16 value to be written to the [WorldPacket]
     */
    void WriteShort(WorldPacket* packet, int16 _short)
    {
        (*packet) << _short;
    }

    /**
     * Writes an unsigned 16-bit integer value to the [WorldPacket].
     *
     * @param uint16 value : the uint16 value to be written to the [WorldPacket]
     */
    void WriteUShort(WorldPacket* packet, uint16 _ushort)
    {
        (*packet) << _ushort;
    }

    /**
     * Writes a signed 32-bit integer value to the [WorldPacket].
     *
     * @param int32 value : the int32 value to be written to the [WorldPacket]
     */
    void WriteLong(WorldPacket* packet, int32 _long)
    {
        (*packet) << _long;
    }

    /**
     * Writes an unsigned 32-bit integer value to the [WorldPacket].
     *
     * @param uint32 value : the uint32 value to be written to the [WorldPacket]
     */
    void WriteULong(WorldPacket* packet, uint32 _ulong)
    {
        (*packet) << _ulong;
    }

    /**
     * Writes a 32-bit floating-point value to the [WorldPacket].
     *
     * @param float value : the float value to be written to the [WorldPacket]
     */
    void WriteFloat(WorldPacket* packet, float _val)
    {
        (*packet) << _val;
    }

    /**
     * Writes a 64-bit floating-point value to the [WorldPacket].
     *
     * @param double value : the double value to be written to the [WorldPacket]
     */
    void WriteDouble(WorldPacket* packet, double _val)
    {
        (*packet) << _val;
    }
}

void RegisterWorldPacketMethods(sol::state& lua)
{
    sol::usertype<WorldPacket> type = lua.new_usertype<WorldPacket>("WorldPacket", sol::no_constructor);

    type["GetOpcode"]       = &LuaWorldPacket::GetOpcode;
    type["GetSize"]         = &LuaWorldPacket::GetSize;
    type["SetOpcode"]       = &LuaWorldPacket::SetOpcode;
    type["ReadByte"]        = &LuaWorldPacket::ReadByte;
    type["ReadUByte"]       = &LuaWorldPacket::ReadUByte;
    type["ReadShort"]       = &LuaWorldPacket::ReadShort;
    type["ReadUShort"]      = &LuaWorldPacket::ReadUShort;
    type["ReadLong"]        = &LuaWorldPacket::ReadLong;
    type["ReadULong"]       = &LuaWorldPacket::ReadULong;
    type["ReadFloat"]       = &LuaWorldPacket::ReadFloat;
    type["ReadDouble"]      = &LuaWorldPacket::ReadDouble;
    type["ReadGUID"]        = &LuaWorldPacket::ReadGUID;
    type["ReadPackedGUID"]  = &LuaWorldPacket::ReadPackedGUID;
    type["ReadString"]      = &LuaWorldPacket::ReadString;
    type["WriteGUID"]       = &LuaWorldPacket::WriteGUID;
    type["WritePackedGUID"] = &LuaWorldPacket::WritePackedGUID;
    type["WriteString"]     = &LuaWorldPacket::WriteString;
    type["WriteByte"]       = &LuaWorldPacket::WriteByte;
    type["WriteUByte"]      = &LuaWorldPacket::WriteUByte;
    type["WriteShort"]      = &LuaWorldPacket::WriteShort;
    type["WriteUShort"]     = &LuaWorldPacket::WriteUShort;
    type["WriteLong"]       = &LuaWorldPacket::WriteLong;
    type["WriteULong"]      = &LuaWorldPacket::WriteULong;
    type["WriteFloat"]      = &LuaWorldPacket::WriteFloat;
    type["WriteDouble"]     = &LuaWorldPacket::WriteDouble;
}
