/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_TEMPLATE_H
#define _ALE_TEMPLATE_H

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
#include "LuaEngine.h"
#include "ALECompat.h"
#include "ALEUtility.h"
#include "SharedDefines.h"

class ALEGlobal
{
public:
    static int thunk(lua_State* L)
    {
        luaL_Reg* l = static_cast<luaL_Reg*>(lua_touserdata(L, lua_upvalueindex(1)));
        int top = lua_gettop(L);
        int expected = l->func(L);
        int args = lua_gettop(L) - top;
        if (args < 0 || args > expected)
        {
            ALE_LOG_ERROR("[ALE]: {} returned unexpected amount of arguments {} out of {}. Report to devs", l->name, args, expected);
            ASSERT(false);
        }
        lua_settop(L, top + expected);
        return expected;
    }

    static void SetMethods(ALE* E, luaL_Reg* methodTable)
    {
        ASSERT(E);
        ASSERT(methodTable);

        lua_pushglobaltable(E->L);

        for (; methodTable && methodTable->name && methodTable->func; ++methodTable)
        {
            lua_pushstring(E->L, methodTable->name);
            lua_pushlightuserdata(E->L, (void*)methodTable);
            lua_pushcclosure(E->L, thunk, 1);
            lua_rawset(E->L, -3);
        }

        lua_remove(E->L, -1);
    }
};

class ALEObject
{
public:
    template<typename T>
    ALEObject(T * obj, bool manageMemory);

    ~ALEObject()
    {
    }

    // Get wrapped object pointer
    void* GetObj() const { return object; }
    // Returns whether the object is valid or not
    bool IsValid() const { return !callstackid || callstackid == sALE->GetCallstackId(); }
    // Returns whether the object can be invalidated or not
    bool CanInvalidate() const { return _invalidate; }
    // Returns pointer to the wrapped object's type name
    const char* GetTypeName() const { return type_name; }

    // Sets the object pointer that is wrapped
    void SetObj(void* obj)
    {
        ASSERT(obj);
        object = obj;
        SetValid(true);
    }
    // Sets the object pointer to valid or invalid
    void SetValid(bool valid)
    {
        ASSERT(!valid || (valid && object));
        if (valid)
            if (CanInvalidate())
                callstackid = sALE->GetCallstackId();
            else
                callstackid = 0;
        else
            callstackid = 1;
    }
    // Sets whether the pointer will be invalidated at end of calls
    void SetValidation(bool invalidate)
    {
        _invalidate = invalidate;
    }
    // Invalidates the pointer if it should be invalidated
    void Invalidate()
    {
        if (CanInvalidate())
            callstackid = 1;
    }

private:
    uint64 callstackid;
    bool _invalidate;
    void* object;
    const char* type_name;
};

template<typename T>
struct ALERegister
{
    const char* name;
    int(*mfunc)(lua_State*, T*);
};

template<typename T>
class ALETemplate
{
public:
    static const char* tname;
    static bool manageMemory;

    // name will be used as type name
    // If gc is true, lua will handle the memory management for object pushed
    // gc should be used if pushing for example WorldPacket,
    // that will only be needed on lua side and will not be managed by TC/mangos/<core>
    static void Register(ALE* E, const char* name, bool gc = false)
    {
        ASSERT(E);
        ASSERT(name);

        // check that metatable isn't already there
        lua_getglobal(E->L, name);
        ASSERT(lua_isnoneornil(E->L, -1));

        // pop nil
        lua_pop(E->L, 1);

        tname = name;
        manageMemory = gc;

        // create metatable for userdata of this type
        luaL_newmetatable(E->L, tname);
        int metatable  = lua_gettop(E->L);

        // push methodtable to stack to be accessed and modified by users
        lua_pushvalue(E->L, metatable);
        lua_setglobal(E->L, tname);

        // tostring
        lua_pushcfunction(E->L, ToString);
        lua_setfield(E->L, metatable, "__tostring");

        // garbage collecting
        lua_pushcfunction(E->L, CollectGarbage);
        lua_setfield(E->L, metatable, "__gc");

        // make methods accessible through metatable
        lua_pushvalue(E->L, metatable);
        lua_setfield(E->L, metatable, "__index");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Add);
        lua_setfield(E->L, metatable, "__add");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Substract);
        lua_setfield(E->L, metatable, "__sub");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Multiply);
        lua_setfield(E->L, metatable, "__mul");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Divide);
        lua_setfield(E->L, metatable, "__div");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Mod);
        lua_setfield(E->L, metatable, "__mod");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Pow);
        lua_setfield(E->L, metatable, "__pow");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, UnaryMinus);
        lua_setfield(E->L, metatable, "__unm");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Concat);
        lua_setfield(E->L, metatable, "__concat");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Length);
        lua_setfield(E->L, metatable, "__len");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Equal);
        lua_setfield(E->L, metatable, "__eq");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Less);
        lua_setfield(E->L, metatable, "__lt");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, LessOrEqual);
        lua_setfield(E->L, metatable, "__le");

        // make new indexes saved to methods
        lua_pushcfunction(E->L, Call);
        lua_setfield(E->L, metatable, "__call");

        // special method to get the object type
        lua_pushcfunction(E->L, GetType);
        lua_setfield(E->L, metatable, "GetObjectType");

        // special method to decide object invalidation at end of call
        lua_pushcfunction(E->L, SetInvalidation);
        lua_setfield(E->L, metatable, "SetInvalidation");

        // pop metatable
        lua_pop(E->L, 1);
    }

    template<typename C>
    static void SetMethods(ALE* E, ALERegister<C>* methodTable)
    {
        ASSERT(E);
        ASSERT(tname);
        ASSERT(methodTable);

        // get metatable
        lua_pushstring(E->L, tname);
        lua_rawget(E->L, LUA_REGISTRYINDEX);
        ASSERT(lua_istable(E->L, -1));

        for (; methodTable && methodTable->name && methodTable->mfunc; ++methodTable)
        {
            lua_pushstring(E->L, methodTable->name);
            lua_pushlightuserdata(E->L, (void*)methodTable);
            lua_pushcclosure(E->L, CallMethod, 1);
            lua_rawset(E->L, -3);
        }

        lua_pop(E->L, 1);
    }

    static int Push(lua_State* L, T const* obj)
    {
        if (!obj)
        {
            lua_pushnil(L);
            return 1;
        }

        // Create new userdata
        ALEObject** ptrHold = static_cast<ALEObject**>(lua_newuserdata(L, sizeof(ALEObject*)));
        if (!ptrHold)
        {
            ALE_LOG_ERROR("{} could not create new userdata", tname);
            lua_pushnil(L);
            return 1;
        }
        *ptrHold = new ALEObject(const_cast<T*>(obj), manageMemory);

        // Set metatable for it
        lua_pushstring(L, tname);
        lua_rawget(L, LUA_REGISTRYINDEX);
        if (!lua_istable(L, -1))
        {
            ALE_LOG_ERROR("{} missing metatable", tname);
            lua_pop(L, 2);
            lua_pushnil(L);
            return 1;
        }
        lua_setmetatable(L, -2);
        return 1;
    }

    static T* Check(lua_State* L, int narg, bool error = true)
    {
        ALEObject* ALEObj = ALE::CHECKTYPE(L, narg, tname, error);
        if (!ALEObj)
            return NULL;

        if (!ALEObj->IsValid())
        {
            char buff[256];
            snprintf(buff, 256, "%s expected, got pointer to nonexisting (invalidated) object (%s). Check your code.", tname, luaL_typename(L, narg));
            if (error)
            {
                luaL_argerror(L, narg, buff);
            }
            else
            {
                ALE_LOG_ERROR("{}", buff);
            }
            return NULL;
        }
        return static_cast<T*>(ALEObj->GetObj());
    }

    static int GetType(lua_State* L)
    {
        lua_pushstring(L, tname);
        return 1;
    }

    static int SetInvalidation(lua_State* L)
    {
        ALEObject* ALEObj = ALE::CHECKOBJ<ALEObject>(L, 1);
        bool invalidate = ALE::CHECKVAL<bool>(L, 2);

        ALEObj->SetValidation(invalidate);
        return 0;
    }

    static int CallMethod(lua_State* L)
    {
        T* obj = ALE::CHECKOBJ<T>(L, 1); // get self
        if (!obj)
            return 0;
        ALERegister<T>* l = static_cast<ALERegister<T>*>(lua_touserdata(L, lua_upvalueindex(1)));
        int top = lua_gettop(L);
        int expected = l->mfunc(L, obj);
        int args = lua_gettop(L) - top;
        if (args < 0 || args > expected)
        {
            ALE_LOG_ERROR("[ALE]: {} returned unexpected amount of arguments {} out of {}. Report to devs", l->name, args, expected);
            ASSERT(false);
        }
        lua_settop(L, top + expected);
        return expected;
    }

    // Metamethods ("virtual")

    // Remember special cases like ALETemplate<Vehicle>::CollectGarbage
    static int CollectGarbage(lua_State* L)
    {
        // Get object pointer (and check type, no error)
        ALEObject* obj = ALE::CHECKOBJ<ALEObject>(L, 1, false);
        if (obj && manageMemory)
            delete static_cast<T*>(obj->GetObj());
        delete obj;
        return 0;
    }

    static int ToString(lua_State* L)
    {
        T* obj = ALE::CHECKOBJ<T>(L, 1, true); // get self
        lua_pushfstring(L, "%s: %p", tname, obj);
        return 1;
    }

    static int ArithmeticError(lua_State* L) { return luaL_error(L, "attempt to perform arithmetic on a %s value", tname); }
    static int CompareError(lua_State* L) { return luaL_error(L, "attempt to compare %s", tname); }
    static int Add(lua_State* L) { return ArithmeticError(L); }
    static int Substract(lua_State* L) { return ArithmeticError(L); }
    static int Multiply(lua_State* L) { return ArithmeticError(L); }
    static int Divide(lua_State* L) { return ArithmeticError(L); }
    static int Mod(lua_State* L) { return ArithmeticError(L); }
    static int Pow(lua_State* L) { return ArithmeticError(L); }
    static int UnaryMinus(lua_State* L) { return ArithmeticError(L); }
    static int Concat(lua_State* L) { return luaL_error(L, "attempt to concatenate a %s value", tname); }
    static int Length(lua_State* L) { return luaL_error(L, "attempt to get length of a %s value", tname); }
    static int Equal(lua_State* L) { ALE::Push(L, ALE::CHECKOBJ<T>(L, 1) == ALE::CHECKOBJ<T>(L, 2)); return 1; }
    static int Less(lua_State* L) { return CompareError(L); }
    static int LessOrEqual(lua_State* L) { return CompareError(L); }
    static int Call(lua_State* L) { return luaL_error(L, "attempt to call a %s value", tname); }
};

template<typename T>
ALEObject::ALEObject(T * obj, bool manageMemory) : callstackid(1), _invalidate(!manageMemory), object(obj), type_name(ALETemplate<T>::tname)
{
    SetValid(true);
}

template<typename T> const char* ALETemplate<T>::tname = NULL;
template<typename T> bool ALETemplate<T>::manageMemory = false;

#endif
