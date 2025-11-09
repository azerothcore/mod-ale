/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef SPELLMETHODS_H
#define SPELLMETHODS_H

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
    inline bool IsAutoRepeat(Spell* spell)
    {
        return spell->IsAutoRepeat();
    }

    /**
     * Returns the [Unit] that casted the [Spell].
     *
     * @return [Unit] caster
     */
    inline Unit* GetCaster(Spell* spell)
    {
        return spell->GetCaster();
    }

    /**
     * Returns the cast time of the [Spell].
     *
     * @return int32 castTime
     */
    inline int32 GetCastTime(Spell* spell)
    {
        return spell->GetCastTime();
    }

    /**
     * Returns the entry ID of the [Spell].
     *
     * @return uint32 entryId
     */
    inline uint32 GetEntry(Spell* spell)
    {
        return spell->m_spellInfo->Id;
    }

    /**
     * Returns the power cost of the [Spell].
     *
     * @return uint32 powerCost
     */
    inline uint32 GetPowerCost(Spell* spell)
    {
        return spell->GetPowerCost();
    }

    /**
     * Returns the reagents needed for the [Spell].
     *
     * @return table reagents : a table containing the [ItemTemplate]s and amount of reagents needed for the [Spell]
    */
    inline sol::table GetReagentCost(Spell* spell, sol::this_state s)
    {
        sol::state_view lua(s);

        auto spellInfo = spell->GetSpellInfo();
        sol::table result = lua.create_table();

        for (int i = 0; i < MAX_SPELL_REAGENTS; ++i)
        {
            if (spellInfo->Reagent[i] <= 0) continue;
            
            auto reagent = eObjectMgr->GetItemTemplate(spellInfo->Reagent[i]);
            if (reagent)
                result[reagent] = spellInfo->ReagentCount[i];
        }
        return result;
    }

    /**
     * Returns the spell duration of the [Spell].
     *
     * @return int32 duration
     */
    inline int32 GetDuration(Spell* spell)
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
    inline std::tuple<float, float, float> GetTargetDest(Spell* spell)
    {
        if (!spell->m_targets.HasDst())
            return std::make_tuple(0.0f, 0.0f, 0.0f);

        float x, y, z;
        spell->m_targets.GetDstPos()->GetPosition(x, y, z);
        return std::make_tuple(x, y, z);
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
    inline Object* GetTarget(Spell* spell)
    {
        if (GameObject* target = spell->m_targets.GetGOTarget())
            return target;
        else if (Item* target = spell->m_targets.GetItemTarget())
            return target;
        else if (Corpse* target = spell->m_targets.GetCorpseTarget())
            return target;
        else if (Unit* target = spell->m_targets.GetUnitTarget())
            return target;
        else if (WorldObject* target = spell->m_targets.GetObjectTarget())
            return target;

        return nullptr;
    }

    /**
     * Sets the [Spell] to automatically repeat.
     *
     * @param bool repeat : set variable to 'true' for spell to automatically repeat
     */
    inline void SetAutoRepeat(Spell* spell, bool repeat)
    {
        spell->SetAutoRepeat(repeat);
    }

    /**
     * Casts the [Spell].
     *
     * @param bool skipCheck = false : skips initial checks to see if the [Spell] can be casted or not, this is optional
     */
    inline void Cast(Spell* spell, bool skipCheck = false)
    {
        spell->cast(skipCheck);
    }

    /**
     * Cancels the [Spell].
     */
    inline void Cancel(Spell* spell)
    {
        spell->cancel();
    }

    /**
     * Finishes the [Spell].
     */
    inline void Finish(Spell* spell)
    {
        spell->finish();
    }

    template<typename T>
    void RegisterSpellMethods(sol::usertype<T>& type)
    {
        // Getters
        type["GetCaster"] = &GetCaster;
        type["GetCastTime"] = &GetCastTime;
        type["GetEntry"] = &GetEntry;
        type["GetPowerCost"] = &GetPowerCost;
        type["GetReagentCost"] = &GetReagentCost;
        type["GetDuration"] = &GetDuration;
        type["GetTargetDest"] = &GetTargetDest;
        type["GetTarget"] = &GetTarget;

        // Setters
        type["SetAutoRepeat"] = &SetAutoRepeat;

        // Boolean
        type["IsAutoRepeat"] = &IsAutoRepeat;

        // Actions
        type["Cast"] = &Cast;
        type["Cancel"] = &Cancel;
        type["Finish"] = &Finish;
    }
};
#endif
