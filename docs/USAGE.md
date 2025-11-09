<div align="center">

# 📖 ALE Usage Guide

*Learn how to create powerful Lua scripts for your AzerothCore server*

[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.com/invite/ZKSVREE7)
[![Lua](https://img.shields.io/badge/Lua-5.2-2C2D72?style=for-the-badge&logo=lua&logoColor=white)](http://www.lua.org/manual/5.2/)

---
</div>

> [!IMPORTANT]
> This guide is for **ALE (AzerothCore Lua Engine)**, which is specifically designed for AzerothCore. Scripts and APIs may differ from other Lua engines.

## 📋 Table of Contents

- [Prerequisites](#-prerequisites)
- [Your First Script](#-your-first-script)
- [Lua Basics](#-lua-basics)
- [ALE Basics](#-ale-basics)
- [Script Reloading](#-script-reloading)
- [Getting Help](#-getting-help)

## ✅ Prerequisites

This guide assumes you have already installed ALE successfully. If you have not, see the [Installation Guide](INSTALL.md) or the [Installation section](../README.md#-installation) in the README.

## 🚀 Your First Script

Let's create a simple "Hello World" script to get you started.

### Creating the Script File

Create a file named `hello_world.lua` in your scripts folder. By default, this folder is called `lua_scripts` and is located in your server folder (the folder containing server executables).

```lua
local PLAYER_EVENT_ON_LOGIN = 3

local function OnLogin(event, player)
    player:SendBroadcastMessage("Hello world from ALE!")
end

RegisterPlayerEvent(PLAYER_EVENT_ON_LOGIN, OnLogin)
```

### Running the Script

1. Restart your AzerothCore server
2. Log in to the game
3. You should see "Hello world from ALE!" in your chat

### What Just Happened?

No core compiling was needed! Your script runs directly from the file you created.

Here's what the script does:
- The file is compiled and run by the Lua engine when the server starts
- The code registers a function to be executed when a player logs in
- When the event triggers, the function sends a message to the player

## 📚 Lua Basics

Before diving deep into ALE scripting, it's helpful to understand some Lua fundamentals.

### Learning Resources

- **Lua Users Wiki**
    - [Lua Directory](http://lua-users.org/wiki/LuaDirectory)
    - [Tutorial Directory](http://lua-users.org/wiki/TutorialDirectory)
    - [Sample Code](http://lua-users.org/wiki/SampleCode)
- **[Programming in Lua](http://www.lua.org/pil/1.html)** - Comprehensive guide
- **[Lua 5.2 Reference Manual](http://www.lua.org/manual/5.2/)** - Official documentation

### Essential Concepts

#### Debugging with Print

The `print()` function outputs to the server console - perfect for debugging:

```lua
print("Debug message here")
print("Player name:", player:GetName())
```

#### Control Structures

Learn about loops and conditionals:
- [Control Structure Tutorial](http://lua-users.org/wiki/ControlStructureTutorial)
- [Lua Manual - Control Structures](http://www.lua.org/manual/5.2/manual.html#3.3.5)

#### String Manipulation

Essential for text processing:
- [String Library Tutorial](http://lua-users.org/wiki/StringLibraryTutorial)
- [Pattern Matching](http://www.wowwiki.com/Pattern_matching)

#### Tables

Tables are Lua's only data structure (arrays and hash maps combined):
- [Table Functions](http://www.lua.org/manual/5.2/manual.html#6.5)
- [Table Semantics](http://www.lua.org/manual/5.2/manual.html#4.3)
- [Tables Tutorial](http://lua-users.org/wiki/TablesTutorial)
- [Programming in Lua - Tables](http://www.lua.org/pil/2.5.html)

#### Variable Scope

**Important:** Prefer local variables over global variables!

```lua
-- Good: Local variable
local playerName = "John"

-- Avoid: Global variable
playerName = "John"
```

Global variables can create conflicts with other scripts. Local variables outside functions are shared within the same script file (locally global).

## ⚡ ALE Basics

### Documentation Resources

- **[ALE API Documentation](https://www.azerothcore.org/eluna/)** - Complete API reference
- **[Hooks Documentation](https://github.com/azerothcore/mod-ale/blob/master/src/LuaEngine/Hooks.h)** - Available event hooks
- **[Implementation Details](IMPL_DETAILS.md)** - Advanced features and best practices

### Error Messages

ALE outputs errors to:
- Server console (real-time)
- Log file in the server folder (persistent)

If your script doesn't work, check the log file for errors. You can configure logging settings in the server configuration file.

### Global Functions

Global functions can be called from anywhere without requiring an object.

Besides standard Lua functions like `print()`, ALE provides its own global functions. Find them in the documentation under the `Global` class: [Global Functions](https://www.azerothcore.org/eluna/Global/index.html).

```lua
-- Example: Get the Lua engine name
print(GetLuaEngine())

-- Example: Get current game time
local time = GetGameTime()
```

### Member Functions (Methods)

Methods require a userdata object to run. Different object types (Player, Creature, GameObject, etc.) have different available methods.

#### Class Inheritance

Classes inherit from each other. For example:
- `Player` and `Creature` both inherit from `Unit`
- This means both players and creatures can use `Unit` methods

#### Calling Methods

Use the `:` notation to call methods on objects:

```lua
-- Get player name
local name = player:GetName()

-- Get creature level
local level = creature:GetLevel()
```

#### Example: Creature Combat Script

```lua
local entry = 6  -- Creature entry ID
local on_combat = 1  -- Event ID for combat start

local function OnCombat(event, creature, target)
    -- creature is of type Creature
    -- target is of type Creature or Player

    print("Creature level:", creature:GetLevel())
    print("Target level:", target:GetLevel())

    creature:SendUnitYell("You dare challenge me?!", 0)
end

RegisterCreatureEvent(entry, on_combat, OnCombat)
```

### Registering Event Handlers

Scripts work by registering functions to events. When the event occurs, your function executes.

#### Finding Register Functions

Search for "register" in the [ALE documentation](https://www.azerothcore.org/eluna/) to find all available register functions:
- `RegisterPlayerEvent`
- `RegisterCreatureEvent`
- `RegisterServerEvent`
- And many more...

#### Event IDs

Each register function requires an event ID. Find these IDs in the function's documentation page.

#### Function Parameters

ALE automatically passes parameters to your registered functions. The parameters are always in the same order, but you can name them anything you want.

Example: `PLAYER_EVENT_ON_LOGIN` passes:
1. Event ID (number)
2. Player object (Player)

```lua
local function OnLogin(event, player)
    -- event = event ID number
    -- player = Player object
end
```

#### Return Values

Some events allow your function to return values to modify behavior. Check the event's documentation to see what can be returned.

```lua
local function OnChat(event, player, msg, type, lang)
    if msg == "blocked" then
        return false  -- Blocks the chat message
    end
end
```

## 🔄 Script Reloading

For quick testing during development, you can reload scripts without restarting:

```
.reload ale
```

> [!WARNING]
> **Development Only:** Use `.reload ale` only for development. For production or if experiencing issues, always restart the server.

**Important Limitations:**
- Reloading doesn't trigger events like login for already-connected players
- Some state may not reset properly
- Always do final testing with a full server restart

## 💬 Getting Help

### Support Channels

- **GitHub Issues**: [Report bugs or request features](https://github.com/azerothcore/mod-ale/issues)
- **Discord Community**: [Join our Discord server](https://discord.com/invite/bx3y5Qmy)
- **AzerothCore Discord**: [Official AzerothCore support](http://www.azerothcore.org/)

### Additional Resources

- [Lua Programming Guide](http://www.lua.org/)
- [AzerothCore Documentation](http://www.azerothcore.org/)
- [Implementation Details](IMPL_DETAILS.md) - Advanced ALE features

---

## 🌟 Acknowledgements

ALE is built upon the foundation of the [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna). We acknowledge and thank the Eluna team for their pioneering work in Lua scripting for World of Warcraft server emulators.

- **[Original Eluna Repository](https://github.com/ElunaLuaEngine/Eluna)**
- **[Eluna Discord Community](https://discord.gg/bjkCVWqqfX)**

---

<div align="center">
<sub>Developed with ❤️ by the AzerothCore and ALE community</sub>

[⬆ Back to Top](#-ale-usage-guide)
</div>
