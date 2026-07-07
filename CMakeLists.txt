set(LUA_VERSION "lua52" CACHE STRING "Lua version to use")
set_property(CACHE LUA_VERSION PROPERTY STRINGS luajit lua51 lua52 lua53 lua54)
MESSAGE(STATUS "Lua version: ${LUA_VERSION}")

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

option(LUA_STATIC "link lua statically" ON)
if (LUA_STATIC)
  MESSAGE(STATUS "Lua linking: static")
else()
  MESSAGE(STATUS "Lua linking: dynamic")
endif()

if (LUA_VERSION MATCHES "luajit")
  add_subdirectory(src/lualib/luajit)
else()
  add_subdirectory(src/lualib/lua)
endif()

# sol (C++ <-> Lua binding library, header-only) is bundled in src/lualib/sol.
# Attaching it to lualib as INTERFACE propagates <sol/sol.hpp> and its compile
# definitions to every consumer of lualib, whichever Lua backend is selected.
# SYSTEM keeps the vendored header out of our warning surface: sol carries
# GCC-only pragmas that clang would otherwise reject under -Werror.
target_include_directories(lualib SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src/lualib")

# Keep sol runtime safety checks enabled in every build type: script errors must
# surface as catchable Lua errors, never as undefined behaviour in the server.
# SOL_EXCEPTIONS_SAFE_PROPAGATION=0 makes sol catch C++ exceptions at the
# binding boundary and convert them into regular Lua errors carrying their
# message; without it LuaJIT swallows them into a generic "C++ exception".
target_compile_definitions(lualib INTERFACE SOL_ALL_SAFETIES_ON=1 SOL_EXCEPTIONS_SAFE_PROPAGATION=0)
