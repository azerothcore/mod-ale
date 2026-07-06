/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_METHODS_H
#define _ALE_METHODS_H

#include <sol/sol.hpp>

/*
 * One registration function per Lua-visible type, each implemented in its own
 * file under methods/. RegisterFunctions (LuaFunctions.cpp) calls them all
 * when a Lua state opens.
 */

void RegisterAchievementMethods(sol::state& lua);
void RegisterAuraMethods(sol::state& lua);
void RegisterBattleGroundMethods(sol::state& lua);
void RegisterChatHandlerMethods(sol::state& lua);
void RegisterCorpseMethods(sol::state& lua);
void RegisterCreatureMethods(sol::state& lua);
void RegisterGameObjectMethods(sol::state& lua);
void RegisterGemPropertiesEntryMethods(sol::state& lua);
void RegisterGlobalMethods(sol::state& lua);
void RegisterGroupMethods(sol::state& lua);
void RegisterGuildMethods(sol::state& lua);
void RegisterItemMethods(sol::state& lua);
void RegisterItemTemplateMethods(sol::state& lua);
void RegisterLootMethods(sol::state& lua);
void RegisterMapMethods(sol::state& lua);
void RegisterObjectMethods(sol::state& lua);
void RegisterPetMethods(sol::state& lua);
void RegisterPlayerMethods(sol::state& lua);
void RegisterQueryMethods(sol::state& lua);
void RegisterQuestMethods(sol::state& lua);
void RegisterRollMethods(sol::state& lua);
void RegisterSpellEntryMethods(sol::state& lua);
void RegisterSpellInfoMethods(sol::state& lua);
void RegisterSpellMethods(sol::state& lua);
void RegisterTicketMethods(sol::state& lua);
void RegisterTransportMethods(sol::state& lua);
void RegisterUnitMethods(sol::state& lua);
void RegisterVehicleMethods(sol::state& lua);
void RegisterWorldObjectMethods(sol::state& lua);
void RegisterWorldPacketMethods(sol::state& lua);

#endif // _ALE_METHODS_H
