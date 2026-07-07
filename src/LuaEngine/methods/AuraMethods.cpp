/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "SpellAuras.h"
#include "Unit.h"

/***
 * The persistent effect of a [Spell] that remains on a [Unit] after the [Spell]
 *   has been cast.
 *
 * As an example, if you cast a damage-over-time spell on a target, an [Aura] is
 *   put on the target that deals damage continuously.
 *
 * [Aura]s on your player are displayed in-game as a series of icons to the left
 *   of the mini-map.
 *
 * Inherits all methods from: none
 */
namespace LuaAura
{
    /**
     * Returns the [Unit] that casted the [Spell] that caused this [Aura] to be applied.
     *
     * @return [Unit] caster
     */
    Unit* GetCaster(Aura* aura)
    {
        return aura->GetCaster();
    }

    /**
     * Returns the GUID of the [Unit] that casted the [Spell] that caused this [Aura] to be applied.
     *
     * @return string caster_guid : the GUID of the Unit as a decimal string
     */
    ObjectGuid GetCasterGUID(Aura* aura)
    {
        return aura->GetCasterGUID();
    }

    /**
     * Returns the level of the [Unit] that casted the [Spell] that caused this [Aura] to be applied.
     *
     * @return uint32 caster_level
     */
    uint8 GetCasterLevel(Aura* aura)
    {
        return aura->GetCaster()->GetLevel();
    }

    /**
     * Returns the amount of time left until the [Aura] expires.
     *
     * @return int32 duration : amount of time left in milliseconds
     */
    int32 GetDuration(Aura* aura)
    {
        return aura->GetDuration();
    }

    /**
     * Returns the ID of the [Spell] that caused this [Aura] to be applied.
     *
     * @return uint32 aura_id
     */
    uint32 GetAuraId(Aura* aura)
    {
        return aura->GetId();
    }

    /**
     * Returns the amount of time this [Aura] lasts when applied.
     *
     * To determine how much time has passed since this Aura was applied,
     *   subtract the result of [Aura]:GetDuration from the result of this method.
     *
     * @return int32 max_duration : the maximum duration of the Aura, in milliseconds
     */
    int32 GetMaxDuration(Aura* aura)
    {
        return aura->GetMaxDuration();
    }

    /**
     * Returns the number of times the [Aura] has "stacked".
     *
     * This is the same as the number displayed on the [Aura]'s icon in-game.
     *
     * @return uint32 stack_amount
     */
    uint8 GetStackAmount(Aura* aura)
    {
        return aura->GetStackAmount();
    }

    /**
     * Returns the [Unit] that the [Aura] has been applied to.
     *
     * @return [Unit] owner
     */
    WorldObject* GetOwner(Aura* aura)
    {
        return aura->GetOwner();
    }

    /**
     * Change the amount of time before the [Aura] expires.
     *
     * @param int32 duration : the new duration of the Aura, in milliseconds
     */
    void SetDuration(Aura* aura, int32 duration)
    {
        aura->SetDuration(duration);
    }

    /**
     * Change the maximum amount of time before the [Aura] expires.
     *
     * This does not affect the current duration of the [Aura], but if the [Aura]
     *   is reset to the maximum duration, it will instead change to `duration`.
     *
     * @param int32 duration : the new maximum duration of the Aura, in milliseconds
     */
    void SetMaxDuration(Aura* aura, int32 duration)
    {
        aura->SetMaxDuration(duration);
    }

    /**
     * Change the amount of times the [Aura] has "stacked" on the [Unit].
     *
     * If `amount` is greater than or equal to the current number of stacks,
     *   then the [Aura] has its duration reset to the maximum duration.
     *
     * @param uint32 amount
     */
    void SetStackAmount(Aura* aura, uint8 amount)
    {
        aura->SetStackAmount(amount);
    }

    /**
     * Remove this [Aura] from the [Unit] it is applied to.
     */
    void Remove(Aura* aura)
    {
        aura->Remove();
    }
}

void RegisterAuraMethods(sol::state& lua)
{
    sol::usertype<ScopedRef<Aura>> type = ALEBind::NewHandleType<ScopedRef<Aura>>(lua, "Aura");

    type["GetCaster"]      = ALEBind::Method(&LuaAura::GetCaster);
    type["GetCasterGUID"]  = ALEBind::Method(&LuaAura::GetCasterGUID);
    type["GetCasterLevel"] = ALEBind::Method(&LuaAura::GetCasterLevel);
    type["GetDuration"]    = ALEBind::Method(&LuaAura::GetDuration);
    type["GetAuraId"]      = ALEBind::Method(&LuaAura::GetAuraId);
    type["GetMaxDuration"] = ALEBind::Method(&LuaAura::GetMaxDuration);
    type["GetStackAmount"] = ALEBind::Method(&LuaAura::GetStackAmount);
    type["GetOwner"]       = ALEBind::Method(&LuaAura::GetOwner);
    type["SetDuration"]    = ALEBind::Method(&LuaAura::SetDuration);
    type["SetMaxDuration"] = ALEBind::Method(&LuaAura::SetMaxDuration);
    type["SetStackAmount"] = ALEBind::Method(&LuaAura::SetStackAmount);
    type["Remove"]         = ALEBind::Method(&LuaAura::Remove);
}
