/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_BIND_H
#define _ALE_BIND_H

#include "ALEHandles.h"

#include <sol/sol.hpp>

#include <type_traits>
#include <utility>

struct AuctionEntry;
class AuctionHouseObject;
class Aura;
class Battleground;
class ChatHandler;
class GmTicket;
struct Loot;
class Roll;
class Spell;
class Vehicle;
class Weather;

/*
 * Adapters that turn plain C++ functions into Lua-callable methods.
 *
 * Binding files should never touch the Lua stack or the handle machinery: they
 * declare methods and this layer does the rest. The two entry points are:
 *
 *   ALEBind::Method   - binds a method on a game object usertype:
 *
 *       player["GetName"] = ALEBind::Method(&Player::GetName);
 *       player["Emote"]   = ALEBind::Method([](Player* self, uint32 emoteId)
 *       {
 *           self->HandleEmoteCommand(emoteId);
 *       });
 *
 *   ALEBind::Function - same conversions for free functions (no self):
 *
 *       lua["GetPlayerByName"] = ALEBind::Function([](std::string const& name)
 *       {
 *           return ObjectAccessor::FindPlayerByName(name);
 *       });
 *
 * What the adapters do for you:
 *
 *   - `self` arrives as a safe handle (see ALEHandles.h) and is resolved before
 *     your code runs; if the object is gone, the script gets a Lua error
 *     instead of the server dereferencing freed memory.
 *   - every parameter typed as a game object pointer (Player*, Unit*, ...) is
 *     received as a handle from Lua and resolved the same way;
 *   - every game object pointer you return is wrapped back into a handle, and
 *     nullptr becomes nil. Pointers to Unit, WorldObject or Object are wrapped
 *     into the most-derived handle, so Lua always sees the concrete type;
 *   - all other types (numbers, strings, bools, enums, ...) pass through with
 *     sol's usual conversions.
 *
 * Patterns for less common signatures:
 *
 *   - optional parameter: take sol::optional<T> and use value_or:
 *       [](Player* self, sol::optional<uint8> locale) { uint8 l = locale.value_or(DEFAULT_LOCALE); ... }
 *   - nullable object parameter: take sol::optional<UnitRef>;
 *   - multiple return values: return std::tuple<...>;
 *   - C++ out-parameters (uint32&) have no Lua equivalent: wrap the call in a
 *     lambda and return the new value instead.
 */
namespace ALEBind
{
    // ---------------------------------------------------------------------
    // Native type -> handle type mapping
    // ---------------------------------------------------------------------

    template<typename T>
    struct HandleFor
    {
    };

    // Object hierarchy: guid-resolved handles
    template<> struct HandleFor<Object>      { using type = ObjectRef; };
    template<> struct HandleFor<WorldObject> { using type = WorldObjectRef; };
    template<> struct HandleFor<Unit>        { using type = UnitRef; };
    template<> struct HandleFor<Player>      { using type = PlayerRef; };
    template<> struct HandleFor<Creature>    { using type = CreatureRef; };
    template<> struct HandleFor<Pet>         { using type = PetRef; };
    template<> struct HandleFor<GameObject>  { using type = GameObjectRef; };
    template<> struct HandleFor<Transport>   { using type = TransportRef; };
    template<> struct HandleFor<Corpse>      { using type = CorpseRef; };
    template<> struct HandleFor<Item>        { using type = ItemRef; };

    // Manager-resolved handles
    template<> struct HandleFor<Map>         { using type = MapRef; };
    template<> struct HandleFor<Group>       { using type = GroupRef; };
    template<> struct HandleFor<Guild>       { using type = GuildRef; };

    // Transient objects: only valid during the event that provided them
    template<> struct HandleFor<Aura>               { using type = ScopedRef<Aura>; };
    template<> struct HandleFor<Spell>              { using type = ScopedRef<Spell>; };
    template<> struct HandleFor<Vehicle>            { using type = ScopedRef<Vehicle>; };
    template<> struct HandleFor<GmTicket>           { using type = ScopedRef<GmTicket>; };
    template<> struct HandleFor<Battleground>       { using type = ScopedRef<Battleground>; };
    template<> struct HandleFor<Weather>            { using type = ScopedRef<Weather>; };
    template<> struct HandleFor<AuctionHouseObject> { using type = ScopedRef<AuctionHouseObject>; };
    template<> struct HandleFor<AuctionEntry>       { using type = ScopedRef<AuctionEntry>; };
    template<> struct HandleFor<Roll>               { using type = ScopedRef<Roll>; };
    template<> struct HandleFor<Loot>               { using type = ScopedRef<Loot>; };
    template<> struct HandleFor<ChatHandler>        { using type = ScopedRef<ChatHandler>; };

    template<typename T>
    using HandleForT = typename HandleFor<std::remove_const_t<T>>::type;

    // Whether T is a game class that Lua sees through a handle.
    template<typename T>
    concept Handled = requires { typename HandleFor<std::remove_const_t<T>>::type; };

    // Whether P is a (possibly const) pointer to a handled game class.
    template<typename P>
    concept HandledPtr = std::is_pointer_v<P> && Handled<std::remove_pointer_t<P>>;

    // Whether P points to a polymorphic base (Object, WorldObject, Unit):
    // those are wrapped into the most-derived handle at runtime.
    template<typename P>
    concept PolymorphicBasePtr = std::is_pointer_v<P>
        && (std::is_same_v<std::remove_const_t<std::remove_pointer_t<P>>, Object>
            || std::is_same_v<std::remove_const_t<std::remove_pointer_t<P>>, WorldObject>
            || std::is_same_v<std::remove_const_t<std::remove_pointer_t<P>>, Unit>);

    // ---------------------------------------------------------------------
    // Parameter and return value conversions
    // ---------------------------------------------------------------------

    // Lua-side type of a native parameter: handles for game object pointers,
    // the native type unchanged for everything else.
    template<typename P>
    struct LuaParam
    {
        using type = P;
    };

    template<HandledPtr P>
    struct LuaParam<P>
    {
        using type = HandleForT<std::remove_pointer_t<P>>;
    };

    template<typename P>
    using LuaParamT = typename LuaParam<P>::type;

    // Converts a Lua-side argument to what the native function expects.
    // Handles resolve to live pointers or raise a Lua error if stale.
    template<typename P>
    decltype(auto) FromLua(LuaParamT<P>& value)
    {
        if constexpr (HandledPtr<P>)
            return value.Require();
        else
            return static_cast<P>(value);
    }

    // Wraps a pointer to a polymorphic base into the most-derived handle, so
    // a Unit that is really a Player arrives in Lua as a Player.
    // One exact overload per base class: binding files usually only see
    // forward declarations, so no pointer conversion can happen there.
    sol::object ToLuaDynamic(sol::state_view lua, Object const* obj);
    sol::object ToLuaDynamic(sol::state_view lua, WorldObject const* obj);
    sol::object ToLuaDynamic(sol::state_view lua, Unit const* unit);

    // Converts a native return value to what Lua receives.
    // Game object pointers become handles, nullptr becomes nil.
    // Always returns by value: the native value is a temporary that dies
    // before sol converts the result, so a reference here would dangle.
    template<typename R>
    auto ToLua(sol::state_view lua, R&& value)
    {
        using Bare = std::remove_cvref_t<R>;

        if constexpr (PolymorphicBasePtr<Bare>)
            return ToLuaDynamic(lua, value);
        else if constexpr (HandledPtr<Bare>)
        {
            using Ref = HandleForT<std::remove_pointer_t<Bare>>;
            return value ? sol::optional<Ref>(Ref(value)) : sol::optional<Ref>(sol::nullopt);
        }
        else
            return Bare(std::forward<R>(value));
    }

    // ---------------------------------------------------------------------
    // Binding factories
    // ---------------------------------------------------------------------

    // Builds the Lua-facing lambda for a callable invocable as R(T*, Args...).
    template<typename R, typename T, typename... Args, typename F>
    auto MakeMethod(F&& fn)
    {
        return [fn = std::forward<F>(fn)](sol::this_state state, HandleForT<T> const& self, LuaParamT<Args>... args)
        {
            T* obj = self.Require();

            if constexpr (std::is_void_v<R>)
                fn(obj, FromLua<Args>(args)...);
            else
                return ToLua(sol::state_view(state), fn(obj, FromLua<Args>(args)...));
        };
    }

    // Builds the Lua-facing lambda for a callable invocable as R(Args...).
    template<typename R, typename... Args, typename F>
    auto MakeFunction(F&& fn)
    {
        return [fn = std::forward<F>(fn)](sol::this_state state, LuaParamT<Args>... args)
        {
            if constexpr (std::is_void_v<R>)
                fn(FromLua<Args>(args)...);
            else
                return ToLua(sol::state_view(state), fn(FromLua<Args>(args)...));
        };
    }

    // Signature extraction for lambdas and other callables, via operator().
    template<typename F>
    struct CallableTraits : CallableTraits<decltype(&F::operator())>
    {
    };

    template<typename C, typename R, typename... Args>
    struct CallableTraits<R (C::*)(Args...) const>
    {
        template<typename F>
        static auto Method(F&& fn)
        {
            // The first parameter of the callable is `self`.
            return MethodImpl<R, Args...>(std::forward<F>(fn));
        }

        template<typename F>
        static auto Function(F&& fn)
        {
            return MakeFunction<R, Args...>(std::forward<F>(fn));
        }

    private:
        template<typename R2, typename Self, typename... Rest, typename F>
        static auto MethodImpl(F&& fn)
        {
            static_assert(std::is_pointer_v<Self> && Handled<std::remove_pointer_t<Self>>,
                "the first parameter of a method callable must be a pointer to a game class (Player*, Unit*, ...)");
            return MakeMethod<R2, std::remove_const_t<std::remove_pointer_t<Self>>, Rest...>(std::forward<F>(fn));
        }
    };

    // Binds a non-const member function: Method(&Player::HasSpell)
    template<typename R, typename T, typename... Args>
    auto Method(R (T::*f)(Args...))
    {
        return MakeMethod<R, T, Args...>([f](T* self, Args... args) -> R
        {
            return (self->*f)(std::forward<Args>(args)...);
        });
    }

    // Binds a const member function: Method(&Player::GetName)
    template<typename R, typename T, typename... Args>
    auto Method(R (T::*f)(Args...) const)
    {
        return MakeMethod<R, T, Args...>([f](T* self, Args... args) -> R
        {
            return (self->*f)(std::forward<Args>(args)...);
        });
    }

    // Binds a free function whose first parameter is the self pointer:
    // Method(&LuaCorpse::GetOwnerGUID) with ObjectGuid GetOwnerGUID(Corpse* corpse)
    template<typename R, typename T, typename... Args>
        requires Handled<std::remove_const_t<T>>
    auto Method(R (*f)(T*, Args...))
    {
        return MakeMethod<R, std::remove_const_t<T>, Args...>([f](std::remove_const_t<T>* self, Args... args) -> R
        {
            return f(self, std::forward<Args>(args)...);
        });
    }

    // Binds a lambda whose first parameter is the self pointer:
    // Method([](Player* self, uint32 id) { ... })
    template<typename F>
        requires (!std::is_member_function_pointer_v<std::remove_reference_t<F>>
            && !std::is_pointer_v<std::remove_reference_t<F>>)
    auto Method(F&& fn)
    {
        return CallableTraits<std::remove_reference_t<F>>::Method(std::forward<F>(fn));
    }

    // Binds a free function or lambda with no self parameter:
    // Function([](std::string const& name) { ... })
    template<typename F>
        requires (!std::is_member_function_pointer_v<std::remove_reference_t<F>>
            && !std::is_pointer_v<std::remove_reference_t<F>>)
    auto Function(F&& fn)
    {
        return CallableTraits<std::remove_reference_t<F>>::Function(std::forward<F>(fn));
    }

    template<typename R, typename... Args>
    auto Function(R (*f)(Args...))
    {
        return MakeFunction<R, Args...>([f](Args... args) -> R
        {
            return f(std::forward<Args>(args)...);
        });
    }

    // ---------------------------------------------------------------------
    // Usertype creation
    // ---------------------------------------------------------------------

    /*
     * Creates the Lua usertype for a handle class:
     *
     *     sol::usertype<PlayerRef> type =
     *         ALEBind::NewHandleType<PlayerRef, UnitRef, WorldObjectRef, ObjectRef>(lua, "Player");
     *
     * `Bases` lists the handle's base classes so Lua can call, for example,
     * Unit methods on a Player. Every handle type also receives:
     *
     *   - IsValid()       - whether the object is currently reachable;
     *   - GetObjectType() - the type name as a string ("Player", ...);
     *   - tostring()      - "TypeName (guid)" for guid-based handles.
     */
    template<typename Ref, typename... Bases>
    sol::usertype<Ref> NewHandleType(sol::state& lua, char const* name)
    {
        sol::usertype<Ref> type = lua.new_usertype<Ref>(name,
            sol::no_constructor,
            sol::base_classes, sol::bases<Bases...>());

        type["IsValid"] = &Ref::IsValid;
        type["GetObjectType"] = [name](Ref const&) { return name; };
        type[sol::meta_function::to_string] = [name](Ref const& ref)
        {
            if constexpr (requires { ref.GetGuid(); })
                return std::string(name) + " (" + ref.GetGuid().ToString() + ")";
            else
                return std::string(name);
        };

        return type;
    }
}

// Lua-visible names of transient types, used in stale-handle error messages.
template<> struct ALETypeName<Aura>               { static constexpr char const* value = "Aura"; };
template<> struct ALETypeName<Spell>              { static constexpr char const* value = "Spell"; };
template<> struct ALETypeName<Vehicle>            { static constexpr char const* value = "Vehicle"; };
template<> struct ALETypeName<GmTicket>           { static constexpr char const* value = "Ticket"; };
template<> struct ALETypeName<Battleground>       { static constexpr char const* value = "BattleGround"; };
template<> struct ALETypeName<Weather>            { static constexpr char const* value = "Weather"; };
template<> struct ALETypeName<AuctionHouseObject> { static constexpr char const* value = "AuctionHouseObject"; };
template<> struct ALETypeName<AuctionEntry>       { static constexpr char const* value = "AuctionEntry"; };
template<> struct ALETypeName<Roll>               { static constexpr char const* value = "Roll"; };
template<> struct ALETypeName<Loot>               { static constexpr char const* value = "Loot"; };
template<> struct ALETypeName<ChatHandler>        { static constexpr char const* value = "ChatHandler"; };

#endif // _ALE_BIND_H
