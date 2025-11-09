<div align="center">

# ⚙️ ALE Implementation Details

*Advanced features and technical documentation for ALE*

[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.com/invite/ZKSVREE7)
[![AzerothCore](https://img.shields.io/badge/AzerothCore-Integrated-darkgreen?style=for-the-badge)](http://www.azerothcore.org/)

---
</div>

> [!IMPORTANT]
> This document covers advanced implementation details and best practices for ALE (AzerothCore Lua Engine). For basic usage, see the [Usage Guide](USAGE.md).

## 📋 Table of Contents

- [Configuration](#-configuration)
- [Script Management](#-script-management)
- [Advanced Features](#-advanced-features)
- [Database Integration](#-database-integration)
- [Performance Tips](#-performance-tips)
- [Debugging](#-debugging)

## ⚙️ Configuration

### Server Configuration File

ALE settings are located in the AzerothCore server configuration file.

> [!WARNING]
> **Important:** Always use the new configuration file generated after compiling with ALE. Without it, error logging and output may not function correctly.

### Available Settings

- **Enable/Disable ALE**: Toggle the Lua engine on or off
- **Traceback Function**: Enable detailed debug information in error messages
- **Script Folder Location**: Configure where ALE looks for script files
- **Logging Settings**: Control log verbosity and output destinations

## 🔄 Script Management

### Script Reloading

Reload scripts during development with:

```
.reload ale
```

> [!CAUTION]
> **Development Only:** This command is for testing purposes only. For production use or troubleshooting, always restart the server.

**Limitations:**
- Events are not re-triggered for existing entities (e.g., logged-in players)
- Some state may persist from the previous load
- Race conditions may occur with active scripts

### Script Loading

#### Default Behavior

- **Default Folder**: `lua_scripts` (configurable in server config)
- **Hidden Folders**: Ignored during loading
- **File Names**: Must be unique across all subdirectories
- **Loading Order**: Not guaranteed to be alphabetical

#### Load Priority

Files with `.ext` extension load before standard `.lua` files:
- `init.ext` loads before `script.lua`

> [!TIP]
> Instead of using `.ext`, prefer the standard Lua `require()` function for better maintainability.

#### Using Require

The entire script folder structure is added to Lua's require path:

```lua
-- Require file: lua_scripts/utilities/helpers.lua
require("utilities/helpers")

-- Or simply (if in root)
require("helpers")
```

**Note:** Omit the `.lua` extension when using `require()`.

## 🎯 Advanced Features

### Automatic Type Conversion

In C++, you must explicitly cast between types:
```cpp
Unit* unit = ...;
Player* player = unit->ToPlayer();  // Manual cast required
```

In ALE, this happens automatically:
```lua
-- unit is automatically converted to the most specific type
-- No manual casting needed!
local name = unit:GetName()  -- Works for Unit, Player, Creature, etc.
```

All objects are automatically converted to their most specific type, giving you full access to all available methods.

### Storing Userdata Objects

> [!CAUTION]
> **Critical:** Never store C++-managed userdata objects in global variables or across events!

#### The Problem

C++ manages object lifetimes. A stored pointer can become invalid when:
- A player logs out
- A creature despawns
- An object is deleted by the core

Accessing invalid pointers causes crashes.

#### The Solution

Objects are automatically set to `nil` when they become unsafe (usually when the hook function ends).

**Instead of storing objects:**

```lua
-- ❌ WRONG: Don't do this
local savedPlayer = nil

local function OnLogin(event, player)
    savedPlayer = player  -- Bad! Will be nil after function ends
end

local function OnLogout(event, player)
    savedPlayer:SendMessage("Test")  -- CRASH! savedPlayer is nil
end
```

**Store GUIDs instead:**

```lua
-- ✅ CORRECT: Store GUID and retrieve object when needed
local playerGUID = nil

local function OnLogin(event, player)
    playerGUID = player:GetGUID()
end

local function SomeLaterEvent(event, ...)
    local player = GetPlayerByGUID(playerGUID)
    if player then
        player:SendMessage("Test")  -- Safe!
    end
end
```

#### Safe to Store

These userdata objects are Lua-managed and safe to store:
- Query results (`ALEQuery`)
- World packets (`WorldPacket`)
- 64-bit numbers (`uint64`, `int64`)

### Userdata Metamethods

#### ToString Support

All userdata objects implement `tostring`:

```lua
print(player)  -- Outputs: Player (Name: "John", GUID: 123456)
print(creature)  -- Outputs: Creature (Entry: 1234, GUID: 789012)
```

#### Global Metatables

Each class has a global table containing its methods:

```lua
-- These global tables exist:
Player = { GetName = function(...) end, ... }
Creature = { GetEntry = function(...) end, ... }
GameObject = { GetDisplayId = function(...) end, ... }
```

#### Custom Methods

You can extend classes with custom methods:

```lua
function Player:CustomGreeting()
    self:SendBroadcastMessage("Welcome, " .. self:GetName() .. "!")
end

function GameObject:IsChest()
    return self:GetGoType() == 3
end

-- Usage:
player:CustomGreeting()
if gameobject:IsChest() then
    print("Found a chest!")
end
```

> [!WARNING]
> Avoid modifying or deleting global class tables in normal code, as this can break other scripts.

## 🗄️ Database Integration

### Query Performance

> [!IMPORTANT]
> Database queries are slow! The entire server waits while data is fetched from disk.

#### Synchronous vs Asynchronous

**Use `Execute` for non-SELECT queries:**
```lua
-- Asynchronous - doesn't block server
WorldDBExecute("UPDATE creature SET level = 80 WHERE entry = 1234")
```

**Use `Query` only when you need results:**
```lua
-- Synchronous - blocks server until complete
local result = WorldDBQuery("SELECT name FROM creature_template WHERE entry = 1234")
```

#### Best Practices

1. **Cache at Startup**: Load data once during server start or script load
2. **Use Tables**: Store frequently accessed data in Lua tables
3. **Batch Operations**: Combine multiple queries when possible
4. **Async When Possible**: Use `Execute` instead of `Query` if you don't need results

```lua
-- ✅ Good: Cache data at startup
local creatureNames = {}

local function LoadCreatureNames()
    local query = WorldDBQuery("SELECT entry, name FROM creature_template")
    if query then
        repeat
            local entry = query:GetUInt32(0)
            local name = query:GetString(1)
            creatureNames[entry] = name
        until not query:NextRow()
    end
end

-- Call once at server start
RegisterServerEvent(33, LoadCreatureNames)  -- SERVER_EVENT_ON_CONFIG_LOAD

-- Now use cached data
local function OnSpawn(event, creature)
    local name = creatureNames[creature:GetEntry()]
    print("Spawned:", name)
end
```

### Database Types

> [!CAUTION]
> **Critical:** Use the correct getter function for each database type!

MySQL performs math in specific formats. Using the wrong getter can return incorrect values on different systems.

#### Type Mapping Table

| Base Type                 | Defined Type | Database Type         | Query Getter      |
|---------------------------|--------------|-----------------------|-------------------|
| char                      | int8         | tinyint(3)            | `GetInt8()`       |
| short int                 | int16        | smallint(5)           | `GetInt16()`      |
| (long int / int)          | int32        | mediumint(8)          | `GetInt32()`      |
| (long int / int)          | int32        | int(10)               | `GetInt32()`      |
| long long int             | int64        | bigint(20)            | `GetInt64()`      |
| unsigned char             | uint8        | tinyint(3) unsigned   | `GetUInt8()`      |
| unsigned short int        | uint16       | smallint(5) unsigned  | `GetUInt16()`     |
| unsigned (long int / int) | uint32       | mediumint(8) unsigned | `GetUInt32()`     |
| unsigned (long int / int) | uint32       | int(10) unsigned      | `GetUInt32()`     |
| unsigned long long int    | uint64       | bigint(20) unsigned   | `GetUInt64()`     |
| float                     | float        | float                 | `GetFloat()`      |
| double                    | double       | double, decimal       | `GetDouble()`     |
| std::string               | std::string  | varchar, text, etc.   | `GetString()`     |

#### Example

```lua
-- ❌ WRONG: Can return 0 or 1 depending on system
local result = WorldDBQuery("SELECT 1")
local value = result:GetUInt32(0)  -- Incorrect type!

-- ✅ CORRECT: Always returns 1
local result = WorldDBQuery("SELECT 1")
local value = result:GetInt64(0)  -- Correct type for literal numbers
```

## ⚡ Performance Tips

### Variable Scope

```lua
-- ✅ Fast: Local variables
local count = 0
for i = 1, 1000 do
    count = count + 1
end

-- ❌ Slow: Global variables
count = 0
for i = 1, 1000 do
    count = count + 1
end
```

### Table Efficiency

```lua
-- ❌ Avoid: Creating tables in loops
for i = 1, 1000 do
    local data = {i, i*2, i*3}  -- 1000 table allocations!
end

-- ✅ Better: Reuse tables
local data = {}
for i = 1, 1000 do
    data[1], data[2], data[3] = i, i*2, i*3
end
```

### Cache Frequently Used Values

```lua
-- ❌ Avoid: Repeated method calls
for i = 1, 100 do
    player:GetName()  -- Calls C++ function 100 times
end

-- ✅ Better: Cache the value
local playerName = player:GetName()
for i = 1, 100 do
    -- Use playerName
end
```

### Minimize Database Access

```lua
-- ❌ Bad: Query in a loop
for entry = 1, 100 do
    local query = WorldDBQuery("SELECT name FROM creature_template WHERE entry = " .. entry)
end

-- ✅ Good: Single query with IN clause
local query = WorldDBQuery("SELECT entry, name FROM creature_template WHERE entry BETWEEN 1 AND 100")
```

## 🐛 Debugging

### Print Debugging

```lua
-- Basic output
print("Debug: Function called")

-- With variables
print("Player:", player:GetName(), "Level:", player:GetLevel())

-- Object inspection
print(player)  -- Uses tostring metamethod
```

### Error Logs

Check these locations for errors:
- **Server Console**: Real-time output
- **Log File**: Persistent record in server folder

### Traceback

Enable traceback in the server config for detailed error information:
```
ALE.TraceBack = 1
```

This adds call stack information to errors.

### Incremental Testing

1. **Start Small**: Test basic functionality first
2. **Add Gradually**: Implement features one at a time
3. **Test Each Step**: Verify each addition works before moving on
4. **Use Reload**: Use `.reload ale` for quick iteration (dev only)
5. **Full Restart**: Always do final testing with a server restart

### Common Issues

**Objects becoming nil:**
- You're storing userdata objects instead of GUIDs
- See [Storing Userdata Objects](#storing-userdata-objects)

**Wrong database values:**
- Using incorrect getter function for database type
- See [Database Types](#database-types)

**Script not loading:**
- Check for duplicate filenames
- Check log for syntax errors
- Verify script folder configuration

---

## 🌟 Acknowledgements

ALE is built upon the foundation of the [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna). We acknowledge and thank the Eluna team for their pioneering work in Lua scripting for World of Warcraft server emulators.

- **[Original Eluna Repository](https://github.com/ElunaLuaEngine/Eluna)**
- **[Eluna Discord Community](https://discord.gg/bjkCVWqqfX)**

---

<div align="center">
<sub>Developed with ❤️ by the AzerothCore and ALE community</sub>

[⬆ Back to Top](#-ale-implementation-details)
</div>
