#include "LuaState.hpp"
#include "ALEIncludes.h"
#include "ALEUtility.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

// ========================================================================
// Constructor / Destructor
// ========================================================================

LuaState::LuaState() : lua(), errorHandler(nullptr), initialized(false) { }

LuaState::~LuaState()
{
    Shutdown();
}

// ========================================================================
// Initialization
// ========================================================================

void LuaState::Initialize()
{
    if (initialized)
    {
        ALE_LOG_INFO("[LuaState]: Already initialized, skipping");
        return;
    }

    SetupStandardLibraries();

    SetupErrorHandling();

    initialized = true;
    ALE_LOG_INFO("[LuaState]: Lua state initialized with Sol2");
}

void LuaState::Shutdown()
{
    if (!initialized)
    {
        return;
    }

    // Clear error handler
    errorHandler = nullptr;

    // Sol2 handles lua_close automatically via RAII in destructor
    // But we can explicitly collect garbage before shutdown
    GarbageCollect(LUA_GCCOLLECT, 0);

    initialized = false;
    ALE_LOG_INFO("[LuaState]: Lua state shutdown");
}

void LuaState::Reset()
{
    ALE_LOG_INFO("[LuaState]: Resetting Lua state");
    Shutdown();

    // Create a new sol::state (this destroys and recreates the Lua state)
    lua = sol::state{};

    Initialize();
}

// ========================================================================
// Script Execution
// ========================================================================

sol::protected_function_result LuaState::ExecuteString(const std::string& code)
{
    if (!initialized)
    {
        ALE_LOG_ERROR("[LuaState]: Cannot execute string, state not initialized");
        // Return error result (constructor: lua_State*, idx, retnum, popped, call_status)
        return sol::protected_function_result(lua.lua_state(), -1, 0, 0, sol::call_status::handler);
    }

    auto result = lua.safe_script(code, sol::script_pass_on_error);

    if (!result.valid())
    {
        sol::error err = result;
        HandleError(fmt::format("ExecuteString failed: {}", err.what()));
    }

    return result;
}

sol::protected_function_result LuaState::LoadFile(const std::string& filepath)
{
    if (!initialized)
    {
        ALE_LOG_ERROR("[LuaState]: Cannot load file, state not initialized");
        // Return error result (constructor: lua_State*, idx, retnum, popped, call_status)
        return sol::protected_function_result(lua.lua_state(), -1, 0, 0, sol::call_status::handler);
    }

    auto result = lua.safe_script_file(filepath, sol::script_pass_on_error);

    if (!result.valid())
    {
        sol::error err = result;
        HandleError(fmt::format("LoadFile '{}' failed: {}", filepath, err.what()));
    }

    return result;
}

sol::load_result LuaState::LoadScript(const std::string& filepath)
{
    if (!initialized)
    {
        ALE_LOG_ERROR("[LuaState]: Cannot load script, state not initialized");
        return lua.load("");
    }

    auto result = lua.load_file(filepath);

    if (!result.valid())
    {
        sol::error err = result;
        HandleError(fmt::format("LoadScript '{}' failed: {}", filepath, err.what()));
    }

    return result;
}

sol::load_result LuaState::LoadBytecode(const char* bytecode, size_t size, const std::string& name)
{
    if (!initialized)
    {
        ALE_LOG_ERROR("[LuaState]: Cannot load bytecode, state not initialized");
        return lua.load("");
    }

    // Load bytecode using luaL_loadbuffer via Sol2
    lua_State* L = lua.lua_state();
    int result = luaL_loadbuffer(L, bytecode, size, name.c_str());

    if (result != LUA_OK)
    {
        const char* error = lua_tostring(L, -1);
        HandleError(fmt::format("LoadBytecode '{}' failed: {}", name, error ? error : "unknown error"));
        lua_pop(L, 1); // Pop error message
    }

    // Return the loaded function (or error) as sol::load_result
    return sol::load_result(L, lua_absindex(L, -1), result);
}

// ========================================================================
// Error Handling
// ========================================================================

void LuaState::SetErrorHandler(std::function<void(const std::string&)> handler)
{
    errorHandler = std::move(handler);
}

void LuaState::HandleError(const std::string& error)
{
    if (errorHandler)
    {
        errorHandler(error);
    }
    else
    {
        // Default: log to ALE logger
        ALE_LOG_ERROR("[LuaState]: {}", error);
    }
}

// ========================================================================
// State Information
// ========================================================================

int LuaState::GarbageCollect(int mode, int data)
{
    if (!initialized)
    {
        return 0;
    }

    lua_State* L = lua.lua_state();
    return lua_gc(L, mode, data);
}

// ========================================================================
// Package Path Management
// ========================================================================

void LuaState::SetRequirePath(const std::string& path)
{
    if (!initialized)
    {
        ALE_LOG_ERROR("[LuaState]: Cannot set require path, state not initialized");
        return;
    }

    // Access package.path
    lua["package"]["path"] = path;
}

void LuaState::SetRequireCPath(const std::string& path)
{
    if (!initialized)
    {
        ALE_LOG_ERROR("[LuaState]: Cannot set require cpath, state not initialized");
        return;
    }

    // Access package.cpath
    lua["package"]["cpath"] = path;
}

std::string LuaState::GetRequirePath() const
{
    if (!initialized)
    {
        return "";
    }

    // Read package.path
    sol::optional<std::string> path = lua["package"]["path"];
    return path.value_or("");
}

std::string LuaState::GetRequireCPath() const
{
    if (!initialized)
    {
        return "";
    }

    // Read package.cpath
    sol::optional<std::string> cpath = lua["package"]["cpath"];
    return cpath.value_or("");
}

// ========================================================================
// Private Setup Methods
// ========================================================================

void LuaState::SetupStandardLibraries()
{
    // Open all standard Lua libraries
    lua.open_libraries(
        sol::lib::base,      // Basic functions (print, assert, etc.)
        sol::lib::package,   // require() and module system
        sol::lib::string,    // String manipulation
        sol::lib::table,     // Table manipulation
        sol::lib::math,      // Math functions
        sol::lib::io,        // File I/O
        sol::lib::os,        // OS interface
        sol::lib::debug      // Debug library
    );

    ALE_LOG_DEBUG("[LuaState]: Standard libraries loaded");
}

void LuaState::SetupErrorHandling()
{
    lua_State* L = lua.lua_state();

    // Set custom panic handler for unrecoverable errors
    lua_atpanic(L, [](lua_State* L) -> int {
        const char* msg = lua_tostring(L, -1);
        ALE_LOG_ERROR("[LuaState] LUA PANIC: {}", msg ? msg : "unknown error");

        // Return 0 to continue unwinding (or abort() will be called)
        return 0;
    });

    ALE_LOG_DEBUG("[LuaState]: Error handling configured");
}
