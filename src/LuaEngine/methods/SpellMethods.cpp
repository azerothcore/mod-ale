/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "ObjectMgr.h"
#include "Spell.h"
#include "SpellInfo.h"

/***
 * An instance of a spell, created when the spell is cast by a [Unit].
 *
 * Inherits all methods from: none
 */
namespace LuaSpell
{
    /**
     * Returns `true` if the [Spell] is automatically repeating, `false` otherwise.
     *
     * @return bool isAutoRepeating
     */
    bool IsAutoRepeat(Spell* spell)
    {
        return spell->IsAutoRepeat();
    }

    /**
     * Returns the [Unit] that casted the [Spell].
     *
     * @return [Unit] caster
     */
    Unit* GetCaster(Spell* spell)
    {
        return spell->GetCaster();
    }

    /**
     * Returns the cast time of the [Spell].
     *
     * @return int32 castTime
     */
    int32 GetCastTime(Spell* spell)
    {
        return spell->GetCastTime();
    }

    /**
     * Returns the entry ID of the [Spell].
     *
     * @return uint32 entryId
     */
    uint32 GetEntry(Spell* spell)
    {
        return spell->m_spellInfo->Id;
    }

    /**
     * Returns the power cost of the [Spell].
     *
     * @return uint32 powerCost
     */
    uint32 GetPowerCost(Spell* spell)
    {
        return spell->GetPowerCost();
    }

    /**
     * Returns the reagents needed for the [Spell].
     *
     * @return table reagents : a table containing the [ItemTemplate]s and amount of reagents needed for the [Spell]
    */
    sol::table GetReagentCost(Spell* spell, sol::this_state s)
    {
        auto spellInfo = spell->GetSpellInfo();
        auto reagents = spellInfo->Reagent;
        auto reagentCounts = spellInfo->ReagentCount;
        sol::table tbl = sol::state_view(s).create_table();
        for (auto i = 0; i < MAX_SPELL_REAGENTS; ++i)
        {
            if (reagents[i] <= 0)
                continue;
            auto reagent = sObjectMgr->GetItemTemplate(reagents[i]);
            auto count = reagentCounts[i];
            tbl[reagent] = count;
        }
        return tbl;
    }

    /**
     * Returns the spell duration of the [Spell].
     *
     * @return int32 duration
     */
    int32 GetDuration(Spell* spell)
    {
        return spell->GetSpellInfo()->GetDuration();
    }

    /**
     * Returns the target destination coordinates of the [Spell].
     *
     * @return float x : x coordinate of the [Spell]
     * @return float y : y coordinate of the [Spell]
     * @return float z : z coordinate of the [Spell]
     */
    std::tuple<sol::optional<float>, sol::optional<float>, sol::optional<float>> GetTargetDest(Spell* spell)
    {
        if (!spell->m_targets.HasDst())
            return { sol::nullopt, sol::nullopt, sol::nullopt };
        float x, y, z;
        spell->m_targets.GetDstPos()->GetPosition(x, y, z);

        return { x, y, z };
    }

    /**
     * Returns the target [Object] of the [Spell].
     *
     * The target can be any of the following [Object] types:
     * - [Player]
     * - [Creature]
     * - [GameObject]
     * - [Item]
     * - [Corpse]
     *
     * @return [Object] target
     */
    sol::object GetTarget(Spell* spell, sol::this_state s)
    {
        sol::state_view lua(s);
        if (GameObject* target = spell->m_targets.GetGOTarget())
            return sol::make_object(lua, GameObjectRef(target));
        else if (Item* target = spell->m_targets.GetItemTarget())
            return sol::make_object(lua, ItemRef(target));
        else if (Corpse* target = spell->m_targets.GetCorpseTarget())
            return sol::make_object(lua, CorpseRef(target));
        else if (Unit* target = spell->m_targets.GetUnitTarget())
            return ALEBind::ToLuaDynamic(lua, target);
        else if (WorldObject* target = spell->m_targets.GetObjectTarget())
            return ALEBind::ToLuaDynamic(lua, target);
        return sol::make_object(lua, sol::lua_nil);
    }

    /**
     * Sets the [Spell] to automatically repeat.
     *
     * @param bool repeat : set variable to 'true' for spell to automatically repeat
     */
    void SetAutoRepeat(Spell* spell, bool repeat)
    {
        spell->SetAutoRepeat(repeat);
    }

    /**
     * Casts the [Spell].
     *
     * @param bool skipCheck = false : skips initial checks to see if the [Spell] can be casted or not, this is optional
     */
    void Cast(Spell* spell, sol::optional<bool> skipCheck)
    {
        spell->cast(skipCheck.value_or(false));
    }

    /**
     * Cancels the [Spell].
     */
    void Cancel(Spell* spell)
    {
        spell->cancel();
    }

    /**
     * Finishes the [Spell].
     */
    void Finish(Spell* spell)
    {
        spell->finish();
    }
}

void RegisterSpellMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<Spell>> type = ALEBind::NewHandleType<ScopedRef<Spell>>(lua, "Spell");

    type["IsAutoRepeat"]   = ALEBind::Method(&LuaSpell::IsAutoRepeat);
    type["GetCaster"]      = ALEBind::Method(&LuaSpell::GetCaster);
    type["GetCastTime"]    = ALEBind::Method(&LuaSpell::GetCastTime);
    type["GetEntry"]       = ALEBind::Method(&LuaSpell::GetEntry);
    type["GetPowerCost"]   = ALEBind::Method(&LuaSpell::GetPowerCost);
    type["GetReagentCost"] = ALEBind::Method(&LuaSpell::GetReagentCost);
    type["GetDuration"]    = ALEBind::Method(&LuaSpell::GetDuration);
    type["GetTargetDest"]  = ALEBind::Method(&LuaSpell::GetTargetDest);
    type["GetTarget"]      = ALEBind::Method(&LuaSpell::GetTarget);
    type["SetAutoRepeat"]  = ALEBind::Method(&LuaSpell::SetAutoRepeat);
    type["Cast"]           = ALEBind::Method(&LuaSpell::Cast);
    type["Cancel"]         = ALEBind::Method(&LuaSpell::Cancel);
    type["Finish"]         = ALEBind::Method(&LuaSpell::Finish);
}
