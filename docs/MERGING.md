<div align="center">

# 🔀 Advanced ALE Integration

*Customizing and extending ALE for your specific needs*

[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.com/invite/ZKSVREE7)
[![AzerothCore](https://img.shields.io/badge/AzerothCore-Integrated-darkgreen?style=for-the-badge)](http://www.azerothcore.org/)

---
</div>

> [!IMPORTANT]
> ALE is distributed as an AzerothCore module. Unlike traditional forks, you don't need to merge code. However, this guide covers advanced integration scenarios for custom AzerothCore builds.

## 📋 Table of Contents

- [Standard Installation](#-standard-installation)
- [Custom AzerothCore Builds](#-custom-azerothcore-builds)
- [Maintaining Custom Changes](#-maintaining-custom-changes)
- [Updating ALE](#-updating-ale)
- [Contributing Back](#-contributing-back)
- [Troubleshooting](#-troubleshooting)

## ✅ Standard Installation

For most users, installing ALE is straightforward:

```bash
# Navigate to modules directory
cd <azerothcore-path>/modules

# Clone ALE
git clone https://github.com/azerothcore/mod-ale.git

# Rebuild AzerothCore
cd <azerothcore-build-directory>
cmake ../ -DLUA_VERSION=luajit
make -j$(nproc)
```

> [!TIP]
> If you're using the standard AzerothCore setup, see the [Installation Guide](INSTALL.md) instead. This guide is for advanced scenarios only.

## 🔧 Custom AzerothCore Builds

### Working with Forked AzerothCore

If you maintain a fork of AzerothCore with custom modifications:

#### Step 1: Add ALE as a Submodule (Recommended)

```bash
cd <your-azerothcore-fork>

# Add ALE as a submodule
git submodule add https://github.com/azerothcore/mod-ale.git modules/mod-ale

# Initialize and update
git submodule init
git submodule update

# Commit the submodule addition
git add .gitmodules modules/mod-ale
git commit -m "Add mod-ale as submodule"
```

**Benefits:**
- Easy updates with `git submodule update`
- Tracks specific ALE versions
- Clean separation between your code and ALE

#### Step 2: Configure Build

Your CMake configuration should automatically detect the module:

```bash
cd build
cmake ../ -DLUA_VERSION=luajit
make -j$(nproc)
```

### Alternative: Clone Directly

If you prefer not to use submodules:

```bash
cd <your-azerothcore-fork>/modules
git clone https://github.com/azerothcore/mod-ale.git
```

> [!WARNING]
> **Not Recommended:** Avoid adding the `modules/mod-ale` directory to your fork's git repository. It makes updates more difficult.

Add to your `.gitignore`:
```
modules/mod-ale/
```

## 🛠️ Maintaining Custom Changes

### Modifying ALE for Your Server

If you need to customize ALE:

#### Option 1: Fork ALE (Recommended)

1. **Fork the repository** on GitHub
2. **Clone your fork** instead of the official repository:
   ```bash
   cd modules
   git clone https://github.com/YOUR_USERNAME/mod-ale.git
   ```
3. **Make your changes** in a feature branch
4. **Keep in sync** with upstream:
   ```bash
   git remote add upstream https://github.com/azerothcore/mod-ale.git
   git fetch upstream
   git merge upstream/master
   ```

#### Option 2: Patch Files

For small changes, create patch files:

```bash
# Make your changes to ALE
cd modules/mod-ale

# Create a patch
git diff > ../../patches/ale-custom-changes.patch

# Apply the patch after updating
git pull
git apply ../../patches/ale-custom-changes.patch
```

### Recommended Approach: Extension Scripts

Instead of modifying ALE core, create extension scripts:

**File:** `lua_scripts/extensions/custom_functions.lua`
```lua
-- Extend existing classes with custom methods
function Player:CustomMethod()
    -- Your custom functionality
end

function Creature:CustomBehavior()
    -- Your custom functionality
end
```

**Benefits:**
- No ALE core modifications needed
- Easy to update ALE
- Clean separation of custom code
- Portable across ALE versions

## 🔄 Updating ALE

### Standard Update Process

For standard installations:

```bash
cd modules/mod-ale
git pull
cd ../../build
make -j$(nproc)
```

### Updating with Submodules

If you added ALE as a submodule:

```bash
# Update to latest ALE
git submodule update --remote modules/mod-ale

# Or update all submodules
git submodule update --remote

# Commit the update
git add modules/mod-ale
git commit -m "Update mod-ale to latest version"
```

### Updating with Custom Changes

If you maintain a fork:

```bash
cd modules/mod-ale

# Fetch upstream changes
git fetch upstream

# Merge or rebase your changes
git merge upstream/master
# or
git rebase upstream/master

# Resolve any conflicts
# Then rebuild
cd ../../build
make -j$(nproc)
```

### Version Pinning

To pin a specific ALE version:

```bash
cd modules/mod-ale

# List available tags
git tag

# Checkout specific version
git checkout v1.2.3

# Or specific commit
git checkout abc123def
```

> [!TIP]
> Pin versions in production environments for stability. Use latest in development for new features.

## 🤝 Contributing Back

If you've made improvements that could benefit others:

### Preparing a Contribution

1. **Ensure it's generic**: Your change should be useful for the broader community
2. **Follow coding standards**: See [Contributing Guide](CONTRIBUTING.md)
3. **Test thoroughly**: Verify your changes don't break existing functionality
4. **Document changes**: Add comments and update documentation

### Submission Process

1. **Fork the official repository**
2. **Create a feature branch**:
   ```bash
   git checkout -b feature/my-improvement
   ```
3. **Make your changes**
4. **Push to your fork**:
   ```bash
   git push origin feature/my-improvement
   ```
5. **Open a Pull Request** on GitHub

See the [Contributing Guide](CONTRIBUTING.md) for detailed instructions.

## 🔧 Troubleshooting

### Merge Conflicts

If you encounter conflicts when updating:

```bash
# Check which files have conflicts
git status

# Edit conflicting files to resolve
# Look for conflict markers: <<<<<<<, =======, >>>>>>>

# Mark as resolved
git add <resolved-file>

# Complete the merge
git merge --continue
```

### Module Not Loading

**Check CMake configuration:**
```bash
cd build
cmake ../ -DLUA_VERSION=luajit
```

Look for output like:
```
-- ALE module found: /path/to/modules/mod-ale
```

**Verify module directory structure:**
```
modules/
└── mod-ale/
    ├── CMakeLists.txt
    ├── src/
    │   └── LuaEngine/
    └── conf/
```

### Build Errors After Update

**Clean build recommended:**
```bash
cd build
rm -rf *
cmake ../ -DLUA_VERSION=luajit
make -j$(nproc)
```

### Submodule Issues

**Submodule not initialized:**
```bash
git submodule init
git submodule update
```

**Submodule in detached HEAD state:**
```bash
cd modules/mod-ale
git checkout master
git pull
```

## 🎯 Advanced Scenarios

### Multiple ALE Versions

Testing with different ALE versions:

```bash
# Clone to different directories
cd modules
git clone https://github.com/azerothcore/mod-ale.git mod-ale-stable
git clone https://github.com/azerothcore/mod-ale.git mod-ale-dev

# Switch between them
ln -sf mod-ale-stable mod-ale  # Use stable
ln -sf mod-ale-dev mod-ale      # Use dev
```

### Custom Lua Version

Using a custom Lua installation:

```bash
cmake ../ \
  -DLUA_VERSION=custom \
  -DLUA_INCLUDE_DIR=/path/to/lua/include \
  -DLUA_LIBRARIES=/path/to/lua/lib/liblua.a
```

### Automated Updates

Create an update script:

**File:** `scripts/update-ale.sh`
```bash
#!/bin/bash

cd modules/mod-ale || exit 1

echo "Fetching latest ALE..."
git fetch upstream

echo "Current version: $(git rev-parse --short HEAD)"
echo "Latest version: $(git rev-parse --short upstream/master)"

read -p "Update to latest? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    git merge upstream/master
    cd ../../build
    make -j$(nproc)
    echo "ALE updated successfully!"
fi
```

## 📚 Additional Resources

- **[Installation Guide](INSTALL.md)** - Standard installation process
- **[Contributing Guide](CONTRIBUTING.md)** - How to contribute changes
- **[Usage Guide](USAGE.md)** - Getting started with scripting
- **[AzerothCore Documentation](http://www.azerothcore.org/)** - Core documentation

### Getting Help

- **GitHub Issues**: [Report problems](https://github.com/azerothcore/mod-ale/issues)
- **Discord**: [Join our community](https://discord.com/invite/bx3y5Qmy)
- **AzerothCore Discord**: [Get support](http://www.azerothcore.org/)

---

## 🌟 Acknowledgements

ALE is built upon the foundation of the [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna). We acknowledge and thank the Eluna team for their pioneering work in Lua scripting for World of Warcraft server emulators.

- **[Original Eluna Repository](https://github.com/ElunaLuaEngine/Eluna)**
- **[Eluna Discord Community](https://discord.gg/bjkCVWqqfX)**

---

<div align="center">
<sub>Developed with ❤️ by the AzerothCore and ALE community</sub>

[⬆ Back to Top](#-advanced-ale-integration)
</div>
