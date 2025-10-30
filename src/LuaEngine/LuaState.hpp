#ifndef ALE_LUA_STATE_HPP
#define ALE_LUA_STATE_HPP

#include <sol/sol.hpp>

#include <memory>
#include <functional>
#include <string>

class LuaState
{
    public:
        LuaState();
        ~LuaState();

        LuaState(const LuaState&) = delete;
        LuaState& operator=(const LuaState&) = delete;

        // === Initialization === //

        /**
         * @brief Initialize the Lua state with standard libraries
         */
        void Initialize();

        /**
         * @brief Shutdown and cleanup the Lua state
         */
        void Shutdown();

        /**
         * @brief Reset the Lua state (shutdown + re-initialize)
         */
        void Reset();

        // === State Access === //

        /**
         * @brief Get the Sol2 state
         *
         * @return Reference to the sol::state
         */
        sol::state& GetState() { return lua; }

        /**
         * @brief Get the Sol2 state (const version)
         *
         * @return Const reference to the sol::state
         */
        const sol::state& GetState() const { return lua; }


        /**
         * @brief Get raw lua_State pointer for legacy compatibility
         * @return Raw lua_State pointer
         */
        lua_State* GetLuaState() { return lua.lua_state(); }

        /**
         * @brief Get raw lua_State pointer (const version)
         *
         * @return Const raw lua_State pointer
         */
        const lua_State* GetLuaState() const { return lua.lua_state(); }

        // === Script Execution === //

        /**
         * @brief Execute a Lua string safely
         * @param code Lua code to execute
         *
         * @return Protected function result (check with result.valid())
         */
        sol::protected_function_result ExecuteString(const std::string& code);

        /**
         * @brief Load and execute a Lua file safely
         * @param filepath Path to the Lua file
         *
         * @return Protected function result (check with result.valid())
         */
        sol::protected_function_result LoadFile(const std::string& filepath);

        /**
         * @brief Load a Lua file without executing it
         * @param filepath Path to the Lua file
         *
         * @return Load result containing the function or error
         */
        sol::load_result LoadScript(const std::string& filepath);

        /**
         * @brief Load Lua bytecode from buffer
         * @param bytecode Compiled Lua bytecode buffer
         * @param size Size of the bytecode buffer
         * @param name Name for error messages
         *
         * @return Load result
         */
        sol::load_result LoadBytecode(const char* bytecode, size_t size, const std::string& name);

        // === Error Handling === //

        /**
         * @brief Set custom error handler
         * @param handler Function to call on Lua errors
         */
        void SetErrorHandler(std::function<void(const std::string&)> handler);

        // === State Information === //

        /**
         * @brief Check if the state is initialized
         *
         * @return true if initialized, false otherwise
         */
        bool IsInitialized() const { return initialized; }

        /**
         * @brief Force Lua garbage collection
         * @param mode GC mode (collect, stop, restart, count, step, setpause, setstepmul)
         * @param data Additional data for the GC operation
         *
         * @return Result of the GC operation
         */
        int GarbageCollect(int mode = LUA_GCCOLLECT, int data = 0);

        // === Package Path Management === //

        /**
         * @brief Set the Lua require() search path
         * @param path Semicolon-separated list of paths with ? as placeholder
         *
         * Example: "/path/to/scripts/?.lua;/path/to/libs/?.lua"
         */
        void SetRequirePath(const std::string& path);

        /**
         * @brief Set the Lua require() search path for C modules
         * @param path Semicolon-separated list of paths with ? as placeholder
         *
         * Example: "/path/to/libs/?.so;/path/to/libs/?.dll"
         */
        void SetRequireCPath(const std::string& path);

        /**
         * @brief Get the current require() search path
         *
         * @return Current package.path value
         */
        std::string GetRequirePath() const;

        /**
         * @brief Get the current require() C search path
         *
         * @return Current package.cpath value
         */
        std::string GetRequireCPath() const;

    private:
        /**
         * @brief Setup standard Lua libraries
         *
         * Opens base, package, string, table, math, io, os, debug libraries.
         */
        void SetupStandardLibraries();

        /**
         * @brief Setup custom error handling and panic handler
         *
         * Configures Sol2's error handling and sets a panic handler for
         * unrecoverable Lua errors.
         */
        void SetupErrorHandling();

        /**
         * @brief Handle Lua errors internally
         * @param error Error message
         *
         * Calls the custom error handler if set, otherwise logs the error.
         */
        void HandleError(const std::string& error);

    private:
        sol::state lua; // Sol 2 state
        std::function<void(const std::string&)> errorHandler; // Custom error handler
        bool initialized; // Initialization state
};

#endif // ALE_LUA_STATE_HPP