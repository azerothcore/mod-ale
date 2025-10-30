#ifndef ALE_METHODS_HPP
#define ALE_METHODS_HPP

#include "SpellMethods.h"
#include "ALETemplate.h"

template<typename T>
void RegisterSpellMethods(sol::usertype<T>& type)
{
    LuaSpell::RegisterSpellMethods(type);
}

void RegisterAll(sol::state& lua)
{
    auto spell_type = lua.new_usertype<Spell>("Spell");
    RegisterSpellMethods(spell_type);

    // Initialize ALETemplate static variables for backward compatibility with Push/Check
    ALETemplate<Spell>::tname = "Spell";
    ALETemplate<Spell>::manageMemory = false;
}

#endif // ALE_METHODS_HPP
