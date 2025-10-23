/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "ALEInstanceAI.h"
#include "ALEUtility.h"
#include "lmarshal.h"


void ALEInstanceAI::Initialize()
{
    LOCK_ALE;

    ASSERT(!sALE->HasInstanceData(instance));

    // Create a new table for instance data.
    lua_State* L = sALE->L;
    lua_newtable(L);
    sALE->CreateInstanceData(instance);

    sALE->OnInitialize(this);
}

void ALEInstanceAI::Load(const char* data)
{
    LOCK_ALE;

    // If we get passed NULL (i.e. `Reload` was called) then use
    //   the last known save data (or maybe just an empty string).
    if (!data)
    {
        data = lastSaveData.c_str();
    }
    else // Otherwise, copy the new data into our buffer.
    {
        lastSaveData.assign(data);
    }

    if (data[0] == '\0')
    {
        ASSERT(!sALE->HasInstanceData(instance));

        // Create a new table for instance data.
        lua_State* L = sALE->L;
        lua_newtable(L);
        sALE->CreateInstanceData(instance);

        sALE->OnLoad(this);
        // Stack: (empty)
        return;
    }

    size_t decodedLength;
    const unsigned char* decodedData = ALEUtil::DecodeData(data, &decodedLength);
    lua_State* L = sALE->L;

    if (decodedData)
    {
        // Stack: (empty)

        lua_pushcfunction(L, mar_decode);
        lua_pushlstring(L, (const char*)decodedData, decodedLength);
        // Stack: mar_decode, decoded_data

        // Call `mar_decode` and check for success.
        if (lua_pcall(L, 1, 1, 0) == 0)
        {
            // Stack: data
            // Only use the data if it's a table.
            if (lua_istable(L, -1))
            {
                sALE->CreateInstanceData(instance);
                // Stack: (empty)
                sALE->OnLoad(this);
                // WARNING! lastSaveData might be different after `OnLoad` if the Lua code saved data.
            }
            else
            {
                ALE_LOG_ERROR("Error while loading instance data: Expected data to be a table (type 5), got type {} instead", lua_type(L, -1));
                lua_pop(L, 1);
                // Stack: (empty)

                Initialize();
            }
        }
        else
        {
            // Stack: error_message
            ALE_LOG_ERROR("Error while parsing instance data with lua-marshal: {}", lua_tostring(L, -1));
            lua_pop(L, 1);
            // Stack: (empty)

            Initialize();
        }

        delete[] decodedData;
    }
    else
    {
        ALE_LOG_ERROR("Error while decoding instance data: Data is not valid base-64");

        Initialize();
    }
}

const char* ALEInstanceAI::Save() const
{
    LOCK_ALE;
    lua_State* L = sALE->L;
    // Stack: (empty)

    /*
     * Need to cheat because this method actually does modify this instance,
     *   even though it's declared as `const`.
     *
     * Declaring virtual methods as `const` is BAD!
     * Don't dictate to children that their methods must be pure.
     */
    ALEInstanceAI* self = const_cast<ALEInstanceAI*>(this);

    lua_pushcfunction(L, mar_encode);
    sALE->PushInstanceData(L, self, false);
    // Stack: mar_encode, instance_data

    if (lua_pcall(L, 1, 1, 0) != 0)
    {
        // Stack: error_message
        ALE_LOG_ERROR("Error while saving: {}", lua_tostring(L, -1));
        lua_pop(L, 1);
        return NULL;
    }

    // Stack: data
    size_t dataLength;
    const unsigned char* data = (const unsigned char*)lua_tolstring(L, -1, &dataLength);
    ALEUtil::EncodeData(data, dataLength, self->lastSaveData);

    lua_pop(L, 1);
    // Stack: (empty)

    return lastSaveData.c_str();
}

uint32 ALEInstanceAI::GetData(uint32 key) const
{
    LOCK_ALE;
    lua_State* L = sALE->L;
    // Stack: (empty)

    sALE->PushInstanceData(L, const_cast<ALEInstanceAI*>(this), false);
    // Stack: instance_data

    ALE::Push(L, key);
    // Stack: instance_data, key

    lua_gettable(L, -2);
    // Stack: instance_data, value

    uint32 value = ALE::CHECKVAL<uint32>(L, -1, 0);
    lua_pop(L, 2);
    // Stack: (empty)

    return value;
}

void ALEInstanceAI::SetData(uint32 key, uint32 value)
{
    LOCK_ALE;
    lua_State* L = sALE->L;
    // Stack: (empty)

    sALE->PushInstanceData(L, this, false);
    // Stack: instance_data

    ALE::Push(L, key);
    ALE::Push(L, value);
    // Stack: instance_data, key, value

    lua_settable(L, -3);
    // Stack: instance_data

    lua_pop(L, 1);
    // Stack: (empty)
}

uint64 ALEInstanceAI::GetData64(uint32 key) const
{
    LOCK_ALE;
    lua_State* L = sALE->L;
    // Stack: (empty)

    sALE->PushInstanceData(L, const_cast<ALEInstanceAI*>(this), false);
    // Stack: instance_data

    ALE::Push(L, key);
    // Stack: instance_data, key

    lua_gettable(L, -2);
    // Stack: instance_data, value

    uint64 value = ALE::CHECKVAL<uint64>(L, -1, 0);
    lua_pop(L, 2);
    // Stack: (empty)

    return value;
}

void ALEInstanceAI::SetData64(uint32 key, uint64 value)
{
    LOCK_ALE;
    lua_State* L = sALE->L;
    // Stack: (empty)

    sALE->PushInstanceData(L, this, false);
    // Stack: instance_data

    ALE::Push(L, key);
    ALE::Push(L, value);
    // Stack: instance_data, key, value

    lua_settable(L, -3);
    // Stack: instance_data

    lua_pop(L, 1);
    // Stack: (empty)
}
