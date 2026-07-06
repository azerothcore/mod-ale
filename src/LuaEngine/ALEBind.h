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
 *     nullptr becomes nil;
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

    template<> struct HandleFor<Object>      { using type = ObjectRef; };
    template<> struct HandleFor<WorldObject> { using type = WorldObjectRef; };
    template<> struct HandleFor<Unit>        { using type = UnitRef; };
    template<> struct HandleFor<Player>      { using type = PlayerRef; };
    template<> struct HandleFor<Creature>    { using type = CreatureRef; };
    template<> struct HandleFor<GameObject>  { using type = GameObjectRef; };
    template<> struct HandleFor<Corpse>      { using type = CorpseRef; };
    template<> struct HandleFor<Item>        { using type = ItemRef; };

    template<typename T>
    using HandleForT = typename HandleFor<std::remove_const_t<T>>::type;

    // Whether T is a game class that Lua sees through a handle.
    template<typename T>
    concept Handled = requires { typename HandleFor<std::remove_const_t<T>>::type; };

    // Whether P is a (possibly const) pointer to a handled game class.
    template<typename P>
    concept HandledPtr = std::is_pointer_v<P> && Handled<std::remove_pointer_t<P>>;

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

    // Converts a native return value to what Lua receives.
    // Game object pointers become handles, nullptr becomes nil.
    template<typename R>
    decltype(auto) ToLua(R&& value)
    {
        using Bare = std::remove_cvref_t<R>;

        if constexpr (HandledPtr<Bare>)
        {
            using Ref = HandleForT<std::remove_pointer_t<Bare>>;
            return value ? sol::optional<Ref>(Ref(value)) : sol::optional<Ref>(sol::nullopt);
        }
        else
            return std::forward<R>(value);
    }

    // ---------------------------------------------------------------------
    // Binding factories
    // ---------------------------------------------------------------------

    // Builds the Lua-facing lambda for a callable invocable as R(T*, Args...).
    template<typename R, typename T, typename... Args, typename F>
    auto MakeMethod(F&& fn)
    {
        return [fn = std::forward<F>(fn)](HandleForT<T> const& self, LuaParamT<Args>... args) -> decltype(auto)
        {
            T* obj = self.Require();

            if constexpr (std::is_void_v<R>)
                fn(obj, FromLua<Args>(args)...);
            else
                return ToLua(fn(obj, FromLua<Args>(args)...));
        };
    }

    // Builds the Lua-facing lambda for a callable invocable as R(Args...).
    template<typename R, typename... Args, typename F>
    auto MakeFunction(F&& fn)
    {
        return [fn = std::forward<F>(fn)](LuaParamT<Args>... args) -> decltype(auto)
        {
            if constexpr (std::is_void_v<R>)
                fn(FromLua<Args>(args)...);
            else
                return ToLua(fn(FromLua<Args>(args)...));
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

    // Binds a lambda whose first parameter is the self pointer:
    // Method([](Player* self, uint32 id) { ... })
    template<typename F>
        requires (!std::is_member_function_pointer_v<std::remove_reference_t<F>>)
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
}

#endif // _ALE_BIND_H
