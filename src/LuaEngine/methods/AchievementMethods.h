/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef ACHIEVEMENTMETHODS_H
#define ACHIEVEMENTMETHODS_H

#include "ALEBind.h"

#include "Common.h"
#include "DBCStructure.h"
#include "SharedDefines.h"

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
    uint32 GetId(AchievementEntry const& achievement)
    {
        return achievement.ID;
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
    char const* GetName(AchievementEntry const& achievement, sol::optional<uint8> locale)
    {
        uint8 localeIndex = locale.value_or(DEFAULT_LOCALE);
        if (localeIndex >= TOTAL_LOCALES)
            throw std::invalid_argument("valid LocaleConstant expected");

        return achievement.name[localeIndex];
    }
}

void RegisterAchievementMethods(sol::state& lua)
{
    sol::usertype<AchievementEntry> type = lua.new_usertype<AchievementEntry>("Achievement", sol::no_constructor);

    type["GetId"]   = &LuaAchievement::GetId;
    type["GetName"] = &LuaAchievement::GetName;
}
#endif
