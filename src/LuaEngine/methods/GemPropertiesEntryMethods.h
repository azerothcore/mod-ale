/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef GEMPROPERTIESENTRYMETHODS_H
#define GEMPROPERTIESENTRYMETHODS_H

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
     * This method retrieves the ID from a given GemPropertiesEntry instance 
     * and pushes it onto the Lua stack.
     *
     * @return uint32 id : The ID of the specified GemPropertiesEntry.
     */
    int GetId(lua_State* L, GemPropertiesEntry* gemProperties)
    {
        ALE::Push(L, gemProperties->ID);
        return 1;
    }

    /**
     * Returns the spell item enchantment of a [GemPropertiesEntry].
     *
     * This function retrieves the `spellitemenchantement` attribute from the provided `GemPropertiesEntry`.
     *
     * @return uint32 spellitemenchantement : The spell item enchantment ID.
     */
    int GetSpellItemEnchantement(lua_State* L, GemPropertiesEntry* entry)
    {
        ALE::Push(L, entry->spellitemenchantement);
        return 1;
    }
}
#endif

