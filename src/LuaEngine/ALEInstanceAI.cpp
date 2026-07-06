/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ALEInstanceAI.h"
#include "ALEUtility.h"
#include "lmarshal.h"

#include <memory>

namespace
{
    // Wraps one of lmarshal's C functions (mar_encode / mar_decode) so it can
    // be called safely through sol.
    sol::protected_function WrapMarshalFunction(sol::state& lua, lua_CFunction fn)
    {
        return sol::make_object(lua, fn).as<sol::protected_function>();
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

    sol::protected_function decode = WrapMarshalFunction(sALE->lua, &mar_decode);
    sol::protected_function_result result = decode(std::string_view(reinterpret_cast<char const*>(decodedData.get()), decodedLength));

    if (!result.valid())
    {
        sol::error error = result;
        ALE_LOG_ERROR("Error while parsing instance data with lua-marshal: {}", error.what());
        Initialize();
        return;
    }

    sol::object decoded = result.get<sol::object>(0);
    if (!decoded.is<sol::table>())
    {
        ALE_LOG_ERROR("Error while loading instance data: Expected data to be a table, got a {} instead",
            sol::type_name(sALE->lua.lua_state(), decoded.get_type()));
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

    sol::protected_function encode = WrapMarshalFunction(sALE->lua, &mar_encode);
    sol::protected_function_result result = encode(sALE->GetInstanceData(self));

    if (!result.valid())
    {
        sol::error error = result;
        ALE_LOG_ERROR("Error while saving: {}", error.what());
        return nullptr;
    }

    // The marshalled table is a binary string; store it base-64 encoded.
    std::string encoded = result.get<std::string>(0);
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
