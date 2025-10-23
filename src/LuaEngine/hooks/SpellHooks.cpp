/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALEIncludes.h"
#include "ALETemplate.h"

using namespace Hooks;

#define START_HOOK(EVENT, ENTRY) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<SpellEvents>(EVENT, ENTRY);\
    if (!SpellEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

#define START_HOOK_WITH_RETVAL(EVENT, ENTRY, RETVAL) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return RETVAL;\
    auto key = EntryKey<SpellEvents>(EVENT, ENTRY);\
    if (!SpellEventBindings->HasBindingsFor(key))\
        return RETVAL;\
    LOCK_ALE

void ALE::OnSpellCastCancel(Unit* caster, Spell* spell, SpellInfo const* spellInfo, bool bySelf)
{
    START_HOOK(SPELL_EVENT_ON_CAST_CANCEL, spellInfo->Id);
    Push(caster);
    Push(spell);
    Push(bySelf);

    CallAllFunctions(SpellEventBindings, key);
}

void ALE::OnSpellCast(Unit* caster, Spell* spell, SpellInfo const* spellInfo, bool skipCheck)
{
    START_HOOK(SPELL_EVENT_ON_CAST, spellInfo->Id);
    Push(caster);
    Push(spell);
    Push(skipCheck);

    CallAllFunctions(SpellEventBindings, key);
}

void ALE::OnSpellPrepare(Unit* caster, Spell* spell, SpellInfo const* spellInfo)
{
    START_HOOK(SPELL_EVENT_ON_PREPARE, spellInfo->Id);
    Push(caster);
    Push(spell);

    CallAllFunctions(SpellEventBindings, key);
}

