<div align="center">

# 🔧 ALE Installation Guide

*Step-by-step instructions for installing ALE on AzerothCore*

[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.com/invite/ZKSVREE7)
[![AzerothCore](https://img.shields.io/badge/AzerothCore-Integrated-darkgreen?style=for-the-badge)](http://www.azerothcore.org/)

---
</div>

> [!IMPORTANT]
> ALE is designed specifically for **AzerothCore**. If you're looking for compatibility with other emulators, check out [ElunaAzerothCore](https://github.com/Eluna-Ports/ElunaAzerothCore) for original Eluna compatibility.

## 📋 Table of Contents

- [Requirements](#-requirements)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Updating](#-updating)
- [Troubleshooting](#-troubleshooting)
- [Next Steps](#-next-steps)

## ⚡ Requirements

Before installing ALE, ensure you have:

### System Requirements

- **AzerothCore Server**: A working AzerothCore installation
- **Git**: Version control system for cloning repositories
- **CMake**: Build system (3.16 or higher recommended)
- **Compiler with C++11 Support**:
  - Windows: Visual Studio 2019 or later
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode 10+

### Build Dependencies

ALE can use either:
- **ACE** (ADAPTIVE Communication Environment), or
- **Boost** (for filesystem library)

These should already be available if you have AzerothCore set up.

## 🚀 Installation

### Step 1: Navigate to Modules Directory

Open your terminal (or Git Bash on Windows) and navigate to your AzerothCore modules directory:

```bash
cd <azerothcore-path>/modules
```

**Example:**
```bash
cd /home/user/azerothcore/modules
```

### Step 2: Clone the Repository

Clone the mod-ale repository into your modules folder:

```bash
git clone https://github.com/azerothcore/mod-ale.git
```

This will create a `mod-ale` folder containing all necessary files.

### Step 3: Configure Build

Navigate to your AzerothCore build directory and configure with CMake:

```bash
cd <azerothcore-build-directory>
cmake ../ -DLUA_VERSION=luajit
```

#### Lua Version Options

Choose your preferred Lua version with the `-DLUA_VERSION` flag:

| Version | Flag | Notes |
|---------|------|-------|
| **LuaJIT** | `luajit` | **Recommended** - Best performance via JIT compilation |
| **Lua 5.2** | `lua52` | **Default** - Used if no version specified |
| **Lua 5.3** | `lua53` | Newer features, compatible |
| **Lua 5.4** | `lua54` | Latest version, all features |

**Examples:**

```bash
# Using LuaJIT (recommended for performance)
cmake ../ -DLUA_VERSION=luajit

# Using Lua 5.3
cmake ../ -DLUA_VERSION=lua53

# Using default (Lua 5.2)
cmake ../
```

### Step 4: Compile

Compile AzerothCore with the newly added module:

**Linux/macOS:**
```bash
make -j$(nproc)
```

**Windows:**
```bash
cmake --build . --config Release
```

> [!TIP]
> The `-j$(nproc)` flag uses all available CPU cores for faster compilation.

### Step 5: Update Configuration

> [!CAUTION]
> **Critical Step:** After compiling, you must use the newly generated configuration files!

The compilation process generates updated config files with ALE settings. Without these, ALE may not function correctly (no error messages, logging issues, etc.).

**Location of config files:**
- Usually in your server's `etc/modules` or `configs/modules` directory
- Look for files like `mod-ale.conf`

Copy the new `.conf.dist` files:

```bash
# Example - adjust paths as needed
cp mod-ale.conf.dist mod-ale.conf
```

Then edit `worldserver.conf` and configure ALE settings (see [Configuration](#-configuration) below).

## ⚙️ Configuration

### ALE Settings in mod-ale.conf

After installation, configure ALE by editing your `mod-ale.conf` file:

```ini
###################################################################################################
# ALE (AZEROTHCORE LUA ENGINE)
###################################################################################################

# Enable or disable ALE
# Default: 1 (enabled)
ALE.Enabled = 1

# Enable traceback for detailed error information
# Useful for debugging but has performance overhead
# Default: 1 (enabled)
ALE.TraceBack = 1

# Script folder location (relative to server binary)
# Default: "lua_scripts"
ALE.ScriptPath = "lua_scripts"
```

### Creating the Scripts Folder

Create the scripts folder next to your server executable:

```bash
mkdir lua_scripts
```

Place your `.lua` script files in this folder. ALE will automatically load them on server start.

## 🔄 Updating

Keep your ALE installation up to date with the latest features and bug fixes.

### Update Steps

1. **Navigate to the mod-ale directory:**

```bash
cd <azerothcore-path>/modules/mod-ale
```

2. **Pull the latest changes:**

```bash
git pull
```

3. **Navigate to your build directory:**

```bash
cd <azerothcore-build-directory>
```

4. **Reconfigure if needed (optional):**

```bash
cmake ../ -DLUA_VERSION=luajit
```

5. **Recompile:**

```bash
# Linux/macOS
make -j$(nproc)

# Windows
cmake --build . --config Release
```

6. **Restart your server** to load the updated version

> [!TIP]
> Always backup your database and scripts before updating!

## 🔧 Troubleshooting

### ALE Not Loading

**Check these things:**

1. **Config file**: Ensure you're using the new `worldserver.conf` generated after compilation
2. **Enabled setting**: Verify `ALE.Enabled = 1` in config
3. **Script path**: Ensure `lua_scripts` folder exists in the correct location
4. **Logs**: Check server logs for error messages

### No Error Messages

If you're not seeing ALE errors:

- **Solution**: You're using an old config file. Copy the new `.conf.dist` file and reconfigure.

### Compilation Errors

**"Lua headers not found":**
- ALE should automatically download Lua dependencies
- Ensure you have internet connection during CMake configuration

**C++11 errors:**
- Update your compiler to one that supports C++11 or later

**ACE/Boost errors:**
- These should be installed with AzerothCore
- Check your AzerothCore installation

### Scripts Not Loading

1. **Check file extension**: Must be `.lua`
2. **Check file names**: Must be unique across all subdirectories
3. **Check syntax**: Look for errors in the log file
4. **Check location**: Scripts must be in the configured `ScriptPath` folder

### Getting Help

If you encounter issues:

- **GitHub Issues**: [Report problems](https://github.com/azerothcore/mod-ale/issues)
- **Discord**: [Join our community](https://discord.com/invite/bx3y5Qmy)
- **AzerothCore Discord**: [Get support](http://www.azerothcore.org/)

## 📚 Next Steps

Now that ALE is installed, you're ready to start scripting!

### Recommended Reading

1. **[Usage Guide](USAGE.md)** - Learn how to write your first script
2. **[Implementation Details](IMPL_DETAILS.md)** - Advanced features and best practices
3. **[API Documentation](https://www.azerothcore.org/eluna/)** - Complete API reference
4. **[Hooks Reference](https://github.com/azerothcore/mod-ale/blob/master/src/LuaEngine/Hooks.h)** - Available event hooks

### Example Scripts

Create a test script to verify everything works:

**File:** `lua_scripts/test.lua`
```lua
local function OnServerStartup()
    print("ALE is working! Server started successfully.")
end

RegisterServerEvent(33, OnServerStartup)  -- SERVER_EVENT_ON_CONFIG_LOAD
```

Restart your server and look for the message in the console.

---

## 🌟 Acknowledgements

ALE is built upon the foundation of the [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna). We acknowledge and thank the Eluna team for their pioneering work in Lua scripting for World of Warcraft server emulators.

- **[Original Eluna Repository](https://github.com/ElunaLuaEngine/Eluna)**
- **[Eluna Discord Community](https://discord.gg/bjkCVWqqfX)**

---

<div align="center">
<sub>Developed with ❤️ by the AzerothCore and ALE community</sub>

[⬆ Back to Top](#-ale-installation-guide)
</div>
