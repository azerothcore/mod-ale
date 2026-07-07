/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ALEInstanceAI.h"
#include "ALEUtility.h"

#include <cstring>
#include <memory>

namespace
{
    /*
     * Minimal binary serializer for the instance data table.
     *
     * Instance data only ever holds plain data (booleans, numbers, strings and
     * nested tables), so a small tagged format is enough: one tag byte per
     * value, numbers as doubles, strings length-prefixed, tables as a pair
     * count followed by the key/value pairs. Entries of any other type
     * (function, userdata, ...) cannot survive a server restart anyway and are
     * skipped with a warning instead of failing the whole save.
     */
    enum class DataTag : uint8
    {
        False  = 0,
        True   = 1,
        Number = 2,
        String = 3,
        Table  = 4
    };

    bool IsSerializable(sol::object const& value)
    {
        switch (value.get_type())
        {
            case sol::type::boolean:
            case sol::type::number:
            case sol::type::string:
            case sol::type::table:
                return true;
            default:
                return false;
        }
    }

    void EncodeValue(sol::object const& value, std::string& out);

    void EncodeTable(sol::table const& table, std::string& out)
    {
        uint32 count = 0;
        for (auto const& [key, value] : table.pairs())
            if (IsSerializable(key) && IsSerializable(value))
                ++count;

        out.push_back(static_cast<char>(DataTag::Table));
        out.append(reinterpret_cast<char const*>(&count), sizeof(count));

        for (auto const& [key, value] : table.pairs())
        {
            if (!IsSerializable(key) || !IsSerializable(value))
            {
                ALE_LOG_ERROR("Instance data entry skipped while saving: keys and values must be booleans, numbers, strings or tables");
                continue;
            }

            EncodeValue(key, out);
            EncodeValue(value, out);
        }
    }

    void EncodeValue(sol::object const& value, std::string& out)
    {
        switch (value.get_type())
        {
            case sol::type::boolean:
                out.push_back(static_cast<char>(value.as<bool>() ? DataTag::True : DataTag::False));
                break;
            case sol::type::number:
            {
                double number = value.as<double>();
                out.push_back(static_cast<char>(DataTag::Number));
                out.append(reinterpret_cast<char const*>(&number), sizeof(number));
                break;
            }
            case sol::type::string:
            {
                std::string str = value.as<std::string>();
                uint32 length = static_cast<uint32>(str.size());
                out.push_back(static_cast<char>(DataTag::String));
                out.append(reinterpret_cast<char const*>(&length), sizeof(length));
                out.append(str);
                break;
            }
            case sol::type::table:
                EncodeTable(value.as<sol::table>(), out);
                break;
            default:
                // Filtered out by IsSerializable before we get here.
                break;
        }
    }

    // Cursor over the decoded save data; every read is bounds-checked so a
    // truncated or corrupted blob fails cleanly instead of reading past the end.
    struct DataReader
    {
        unsigned char const* pos;
        unsigned char const* end;

        bool Read(void* dest, size_t size)
        {
            if (static_cast<size_t>(end - pos) < size)
                return false;

            std::memcpy(dest, pos, size);
            pos += size;
            return true;
        }
    };

    // Reads one value; returns sol::lua_nil and clears `ok` on malformed data.
    sol::object DecodeValue(sol::state& lua, DataReader& reader, bool& ok)
    {
        uint8 tag;
        if (!reader.Read(&tag, sizeof(tag)))
        {
            ok = false;
            return sol::make_object(lua, sol::lua_nil);
        }

        switch (static_cast<DataTag>(tag))
        {
            case DataTag::False:
                return sol::make_object(lua, false);
            case DataTag::True:
                return sol::make_object(lua, true);
            case DataTag::Number:
            {
                double number;
                if (!reader.Read(&number, sizeof(number)))
                    break;

                return sol::make_object(lua, number);
            }
            case DataTag::String:
            {
                uint32 length;
                if (!reader.Read(&length, sizeof(length)))
                    break;

                std::string str(length, '\0');
                if (!reader.Read(str.data(), length))
                    break;

                return sol::make_object(lua, str);
            }
            case DataTag::Table:
            {
                uint32 count;
                if (!reader.Read(&count, sizeof(count)))
                    break;

                sol::table table = lua.create_table();
                for (uint32 i = 0; i < count; ++i)
                {
                    sol::object key = DecodeValue(lua, reader, ok);
                    sol::object value = DecodeValue(lua, reader, ok);
                    if (!ok)
                        return sol::make_object(lua, sol::lua_nil);

                    table[key] = value;
                }

                return table;
            }
            default:
                break;
        }

        ok = false;
        return sol::make_object(lua, sol::lua_nil);
    }
}

void ALEInstanceAI::Initialize()
{
    LOCK_ALE;

    ASSERT(!sALE->HasInstanceData(instance));

    // Create a new table for instance data.
    sALE->CreateInstanceData(instance, sALE->lua.create_table());

    sALE->OnInitialize(this);
}

void ALEInstanceAI::Load(const char* data)
{
    LOCK_ALE;

    // If we get passed NULL (i.e. `Reload` was called) then use
    //   the last known save data (or maybe just an empty string).
    if (!data)
        data = lastSaveData.c_str();
    else // Otherwise, copy the new data into our buffer.
        lastSaveData.assign(data);

    if (data[0] == '\0')
    {
        ASSERT(!sALE->HasInstanceData(instance));

        // Create a new table for instance data.
        sALE->CreateInstanceData(instance, sALE->lua.create_table());

        sALE->OnLoad(this);
        return;
    }

    size_t decodedLength;
    std::unique_ptr<unsigned char const[]> decodedData(ALEUtil::DecodeData(data, &decodedLength));

    if (!decodedData)
    {
        ALE_LOG_ERROR("Error while decoding instance data: Data is not valid base-64");
        Initialize();
        return;
    }

    DataReader reader{ decodedData.get(), decodedData.get() + decodedLength };
    bool ok = true;
    sol::object decoded = DecodeValue(sALE->lua, reader, ok);

    if (!ok || !decoded.is<sol::table>())
    {
        ALE_LOG_ERROR("Error while loading instance data: save data is malformed");
        Initialize();
        return;
    }

    sALE->CreateInstanceData(instance, decoded.as<sol::table>());
    // WARNING! lastSaveData might be different after `OnLoad` if the Lua code saved data.
    sALE->OnLoad(this);
}

const char* ALEInstanceAI::Save() const
{
    LOCK_ALE;

    /*
     * Need to cheat because this method actually does modify this instance,
     *   even though it's declared as `const`.
     */
    ALEInstanceAI* self = const_cast<ALEInstanceAI*>(this);

    std::string encoded;
    EncodeTable(sALE->GetInstanceData(self), encoded);

    // The serialized table is binary; store it base-64 encoded.
    ALEUtil::EncodeData(reinterpret_cast<unsigned char const*>(encoded.data()), encoded.size(), self->lastSaveData);

    return lastSaveData.c_str();
}

uint32 ALEInstanceAI::GetData(uint32 key) const
{
    LOCK_ALE;

    sol::table data = sALE->GetInstanceData(const_cast<ALEInstanceAI*>(this));
    return data.get_or(key, 0u);
}

void ALEInstanceAI::SetData(uint32 key, uint32 value)
{
    LOCK_ALE;

    sol::table data = sALE->GetInstanceData(this);
    data[key] = value;
}

uint64 ALEInstanceAI::GetData64(uint32 key) const
{
    LOCK_ALE;

    sol::table data = sALE->GetInstanceData(const_cast<ALEInstanceAI*>(this));
    return data.get_or(key, uint64(0));
}

void ALEInstanceAI::SetData64(uint32 key, uint64 value)
{
    LOCK_ALE;

    sol::table data = sALE->GetInstanceData(this);
    data[key] = value;
}
