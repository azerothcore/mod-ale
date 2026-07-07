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
#include "Pet.h"
#include "Player.h"
#include "Transport.h"

namespace ALEBind
{
    sol::object ToLuaDynamic(sol::state_view lua, Object const* obj)
    {
        if (!obj)
            return sol::make_object(lua, sol::nil);

        if (Player const* player = obj->ToPlayer())
            return sol::make_object(lua, PlayerRef(player));

        if (Creature* creature = const_cast<Object*>(obj)->ToCreature())
        {
            if (Pet const* pet = creature->ToPet())
                return sol::make_object(lua, PetRef(pet));

            return sol::make_object(lua, CreatureRef(creature));
        }

        if (GameObject* go = const_cast<Object*>(obj)->ToGameObject())
        {
            if (Transport const* transport = go->ToTransport())
                return sol::make_object(lua, TransportRef(transport));

            return sol::make_object(lua, GameObjectRef(go));
        }

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
