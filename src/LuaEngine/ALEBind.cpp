/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEBind.h"

#include "Corpse.h"
#include "GameObject.h"
#include "Item.h"
#include "Object.h"
#include "Player.h"

namespace ALEBind
{
    sol::object ToLuaDynamic(sol::state_view lua, Object const* obj)
    {
        if (!obj)
            return sol::make_object(lua, sol::nil);

        if (Player const* player = obj->ToPlayer())
            return sol::make_object(lua, PlayerRef(player));

        if (Creature const* creature = obj->ToCreature())
            return sol::make_object(lua, CreatureRef(creature));

        if (GameObject const* go = obj->ToGameObject())
            return sol::make_object(lua, GameObjectRef(go));

        if (Corpse const* corpse = obj->ToCorpse())
            return sol::make_object(lua, CorpseRef(corpse));

        if (obj->IsItem())
            return sol::make_object(lua, ItemRef(static_cast<Item const*>(obj)));

        return sol::make_object(lua, sol::nil);
    }

    sol::object ToLuaDynamic(sol::state_view lua, WorldObject const* obj)
    {
        return ToLuaDynamic(lua, static_cast<Object const*>(obj));
    }

    sol::object ToLuaDynamic(sol::state_view lua, Unit const* unit)
    {
        return ToLuaDynamic(lua, static_cast<Object const*>(unit));
    }
}
