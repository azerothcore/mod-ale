/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "LuaEngine.h"

#include "ALERegistry.h"
#include "CreatureData.h"
#include "ObjectGuid.h"

/*
 * Every game object crossing the C++/Lua boundary carries its ObjectGuid, and
 * many methods return one (GetGUID, GetOwnerGUID, ...). Exposing it as a real
 * usertype replaces the old boxed "long long"/"unsigned long long" userdata:
 * guids compare with ==, print with tostring() and never lose precision the
 * way Lua numbers would.
 */
namespace LuaObjectGuid
{
    /**
     * Returns the low (counter) part of the guid, unique per object type.
     *
     * @return uint32 counter
     */
    uint32 GetCounter(ObjectGuid guid)
    {
        return guid.GetCounter();
    }

    /**
     * Returns the entry id encoded in the guid (creature/gameobject template entry), or 0.
     *
     * @return uint32 entry
     */
    uint32 GetEntry(ObjectGuid guid)
    {
        return guid.GetEntry();
    }

    /**
     * Returns 'true' if the guid is empty (references nothing).
     *
     * @return bool isEmpty
     */
    bool IsEmpty(ObjectGuid guid)
    {
        return guid.IsEmpty();
    }

    /**
     * Returns 'true' if the guid belongs to a player.
     *
     * @return bool isPlayer
     */
    bool IsPlayer(ObjectGuid guid)
    {
        return guid.IsPlayer();
    }

    /**
     * Returns 'true' if the guid belongs to a creature, pet or vehicle.
     *
     * @return bool isCreature
     */
    bool IsCreature(ObjectGuid guid)
    {
        return guid.IsAnyTypeCreature();
    }

    /**
     * Returns 'true' if the guid belongs to a gameobject.
     *
     * @return bool isGameObject
     */
    bool IsGameObject(ObjectGuid guid)
    {
        return guid.IsGameObject();
    }

    /**
     * Returns 'true' if the guid belongs to an item.
     *
     * @return bool isItem
     */
    bool IsItem(ObjectGuid guid)
    {
        return guid.IsItem();
    }

    /**
     * Returns a readable representation of the guid, e.g. "Creature/0/12345".
     *
     * @return string guidString
     */
    std::string ToString(ObjectGuid guid)
    {
        return guid.ToString();
    }
}

namespace
{
    void RegisterObjectGuid(sol::state& lua)
    {
        sol::usertype<ObjectGuid> type = lua.new_usertype<ObjectGuid>("ObjectGuid", sol::no_constructor);

        type["GetCounter"]   = &LuaObjectGuid::GetCounter;
        type["GetEntry"]     = &LuaObjectGuid::GetEntry;
        type["IsEmpty"]      = &LuaObjectGuid::IsEmpty;
        type["IsPlayer"]     = &LuaObjectGuid::IsPlayer;
        type["IsCreature"]   = &LuaObjectGuid::IsCreature;
        type["IsGameObject"] = &LuaObjectGuid::IsGameObject;
        type["IsItem"]       = &LuaObjectGuid::IsItem;
        type["ToString"]     = &LuaObjectGuid::ToString;

        type[sol::meta_function::to_string] = &LuaObjectGuid::ToString;
    }

    // Some methods return a CreatureTemplate pointer; the type is registered
    // without methods (as it always was) so those returns are valid Lua values.
    void RegisterCreatureTemplate(sol::state& lua)
    {
        lua.new_usertype<CreatureTemplate>("CreatureTemplate", sol::no_constructor);
    }
}

void RegisterFunctions(ALE* E)
{
    sol::state& lua = E->lua;

    RegisterGlobalMethods(lua);

    RegisterObjectGuid(lua);
    RegisterCreatureTemplate(lua);

    // The handle hierarchy, most-derived types after their bases.
    RegisterObjectMethods(lua);
    RegisterWorldObjectMethods(lua);
    RegisterUnitMethods(lua);
    RegisterPlayerMethods(lua);
    RegisterCreatureMethods(lua);
    RegisterGameObjectMethods(lua);
    RegisterTransportMethods(lua);
    RegisterCorpseMethods(lua);
    RegisterItemMethods(lua);
    RegisterPetMethods(lua);

    // Manager-resolved handles.
    RegisterMapMethods(lua);
    RegisterGroupMethods(lua);
    RegisterGuildMethods(lua);

    // Event-scoped types, only valid during the event that provided them.
    RegisterAuraMethods(lua);
    RegisterSpellMethods(lua);
    RegisterVehicleMethods(lua);
    RegisterBattleGroundMethods(lua);
    RegisterChatHandlerMethods(lua);
    RegisterTicketMethods(lua);
    RegisterRollMethods(lua);
    RegisterLootMethods(lua);

    // Static game data.
    RegisterAchievementMethods(lua);
    RegisterItemTemplateMethods(lua);
    RegisterQuestMethods(lua);
    RegisterSpellInfoMethods(lua);
    RegisterSpellEntryMethods(lua);
    RegisterGemPropertiesEntryMethods(lua);

    // Value types.
    RegisterWorldPacketMethods(lua);
    RegisterALEQueryMethods(lua);
}
