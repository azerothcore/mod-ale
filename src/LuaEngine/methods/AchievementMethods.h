/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef ACHIEVEMENTMETHODS_H
#define ACHIEVEMENTMETHODS_H

/***
 * Represents an entry from the game's achievement database (e.g., achievement earned for completing certain tasks).
 *
 * Inherits all methods from: none
 */
namespace LuaAchievement
{
    /**
     * Returns the [Achievement]'s ID.
     *
     * @return uint32 id
     */
    int GetId(lua_State* L, AchievementEntry* const achievement)
    {
        ALE::Push(L, achievement->ID);
        return 1;
    }

    /**
     * Returns the [Achievement]'s name.
     *
     *     enum LocaleConstant
     *     {
     *         LOCALE_enUS = 0,
     *         LOCALE_koKR = 1,
     *         LOCALE_frFR = 2,
     *         LOCALE_deDE = 3,
     *         LOCALE_zhCN = 4,
     *         LOCALE_zhTW = 5,
     *         LOCALE_esES = 6,
     *         LOCALE_esMX = 7,
     *         LOCALE_ruRU = 8
     *     };
     *
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [Achievement] name in
     * @return string name
     */
    int GetName(lua_State* L, AchievementEntry* const achievement)
    {
        uint8 locale = ALE::CHECKVAL<uint8>(L, 2, DEFAULT_LOCALE);
        if (locale >= TOTAL_LOCALES)
        {
            return luaL_argerror(L, 2, "valid LocaleConstant expected");
        }

        ALE::Push(L, achievement->name[locale]);
        return 1;
    }
};
#endif
