/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Common.h"
#include "DBCStructure.h"

/***
 * Represents static gem data used in item enhancement, including spell enchantments triggered by socketed gems.
 *
 * Provides access to gem-related properties from the DBC table `GemProperties.dbc`.
 *
 * Inherits all methods from: none
 */
namespace LuaGemPropertiesEntry
{

    /**
     * Returns the ID of a [GemPropertiesEntry].
     *
     * @return uint32 id : The ID of the specified GemPropertiesEntry.
     */
    uint32 GetId(GemPropertiesEntry* gempropertiesentry)
    {
        return gempropertiesentry->ID;
    }

    /**
     * Returns the spell item enchantment of a [GemPropertiesEntry].
     *
     * This function retrieves the `spellitemenchantement` attribute from the provided `GemPropertiesEntry`.
     *
     * @return uint32 spellitemenchantement : The spell item enchantment ID.
     */
    uint32 GetSpellItemEnchantement(GemPropertiesEntry* gempropertiesentry)
    {
        return gempropertiesentry->spellitemenchantement;
    }
}

void RegisterGemPropertiesEntryMethods(sol::state& lua)
{
    sol::usertype<GemPropertiesEntry> type = lua.new_usertype<GemPropertiesEntry>("GemPropertiesEntry", sol::no_constructor);

    type["GetId"]                    = &LuaGemPropertiesEntry::GetId;
    type["GetSpellItemEnchantement"] = &LuaGemPropertiesEntry::GetSpellItemEnchantement;
}
