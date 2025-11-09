<div align="center">

# 🤝 Contributing to ALE

*Help improve the AzerothCore Lua Engine*

[![Discord](https://img.shields.io/badge/Discord-Join%20Us-7289DA?style=for-the-badge&logo=discord&logoColor=white)](https://discord.com/invite/ZKSVREE7)
[![GitHub Issues](https://img.shields.io/badge/GitHub-Issues-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/azerothcore/mod-ale/issues)

---
</div>

> [!IMPORTANT]
> We welcome contributions from everyone! Whether you code or not, there are many ways to help improve ALE.

## 📋 Table of Contents

- [Ways to Contribute](#-ways-to-contribute)
- [Contributing Code](#-contributing-code)
- [Reporting Issues](#-reporting-issues)
- [Improving Documentation](#-improving-documentation)
- [Creating Scripts](#-creating-scripts)
- [Development Guidelines](#-development-guidelines)

## 🌟 Ways to Contribute

You don't need to be a developer to contribute to ALE! Here are various ways you can help:

### 💻 For Developers

- **Fix Bugs**: Browse open issues and submit fixes
- **Add Features**: Implement new functionality
- **Improve Performance**: Optimize existing code
- **Write Tests**: Help ensure code quality
- **Review Pull Requests**: Provide feedback on proposed changes

### 📚 For Writers

- **Improve Documentation**: Fix typos, clarify explanations, add examples
- **Write Tutorials**: Create guides for common use cases
- **Translate**: Help make documentation available in other languages
- **API Documentation**: Document undocumented functions

### 🎮 For Scripters

- **Create Example Scripts**: Share useful script templates
- **Report Script Issues**: Help identify API problems
- **Share Best Practices**: Document scripting patterns
- **Test New Features**: Verify new functionality works as expected

### 💬 For Community Members

- **Answer Questions**: Help others on Discord or GitHub
- **Report Bugs**: Submit detailed issue reports
- **Test Releases**: Try pre-release versions and provide feedback
- **Spread the Word**: Share ALE with other server administrators

## 💻 Contributing Code

### Getting Started

1. **[Set up Git](https://help.github.com/articles/set-up-git/)** if you haven't already

2. **[Fork the repository](https://help.github.com/articles/fork-a-repo/)** to your GitHub account:
   - Main repository: [mod-ale](https://github.com/azerothcore/mod-ale)
   - Core engine: [ALE source in mod-ale](https://github.com/azerothcore/mod-ale/tree/master/src/LuaEngine)

3. **Clone your fork** to your local machine:
   ```bash
   git clone https://github.com/YOUR_USERNAME/mod-ale.git
   cd mod-ale
   ```

4. **Add the upstream repository**:
   ```bash
   git remote add upstream https://github.com/azerothcore/mod-ale.git
   ```

### Development Workflow

1. **Create a feature branch** from `master`:
   ```bash
   git checkout master
   git pull upstream master
   git checkout -b feature/my-awesome-feature
   ```

2. **Make your changes**:
   - Write clean, readable code
   - Follow existing code style
   - Add comments where necessary
   - Test your changes thoroughly

3. **Commit your changes**:
   ```bash
   git add .
   git commit -m "Add feature: description of what you did"
   ```

   **Commit Message Guidelines:**
   - Use clear, descriptive messages
   - Start with a verb (Add, Fix, Update, Remove, etc.)
   - Keep the first line under 50 characters
   - Add detailed explanation in subsequent lines if needed

   ```
   Add new Player:GetQuestStatus method

   This method allows scripts to check the status of a player's quest
   without needing to retrieve the full quest object. This improves
   performance for common quest-checking scenarios.
   ```

4. **Push to your fork**:
   ```bash
   git push origin feature/my-awesome-feature
   ```

5. **[Open a Pull Request](https://help.github.com/articles/using-pull-requests/)**:
   - Go to your fork on GitHub
   - Click "New Pull Request"
   - Select your feature branch
   - Provide a clear description of your changes
   - Link any related issues

### Code Style Guidelines

#### C++ Code

```cpp
// Use clear, descriptive names
int GetPlayerLevel(Player* player)
{
    if (!player)
        return 0;

    return player->getLevel();
}

// Prefer modern C++ features
std::vector<uint32> GetPlayerQuests(Player* player)
{
    std::vector<uint32> quests;
    // ... implementation
    return quests;
}
```

**Key Points:**
- Use 4 spaces for indentation (no tabs)
- Opening braces on the same line for functions
- Use meaningful variable names
- Add comments for complex logic
- Follow existing patterns in the codebase

#### Lua Code (for examples/tests)

```lua
-- Use local variables
local QUEST_STATUS_COMPLETE = 1

-- Clear, descriptive function names
local function OnQuestComplete(event, player, quest)
    if not player or not quest then
        return
    end

    local questId = quest:GetId()
    print("Player", player:GetName(), "completed quest", questId)
end

-- Register with clear event IDs
RegisterPlayerEvent(8, OnQuestComplete)  -- PLAYER_EVENT_ON_QUEST_COMPLETE
```

### Testing Your Changes

Before submitting a PR:

1. **Compile the code** without errors
2. **Test basic functionality** with your changes
3. **Test edge cases** (nil values, invalid input, etc.)
4. **Verify no regressions** (existing features still work)
5. **Test on a real server** if possible

## 🐛 Reporting Issues

Good bug reports help us fix problems faster!

### Before Reporting

1. **Search existing issues** to avoid duplicates
2. **Test on latest version** to ensure it's not already fixed
3. **Gather information** about your setup and the problem

### Creating a Good Issue Report

Use this template:

```markdown
**Describe the bug**
A clear description of what the bug is.

**To Reproduce**
Steps to reproduce the behavior:
1. Load script '...'
2. Trigger event '...'
3. See error

**Expected behavior**
What you expected to happen.

**Actual behavior**
What actually happened.

**Environment:**
- AzerothCore version: [e.g., commit hash]
- ALE version: [e.g., commit hash]
- Lua version: [e.g., LuaJIT, Lua 5.2]
- OS: [e.g., Ubuntu 20.04]

**Script/Code:**
```lua
-- Minimal script that reproduces the issue
local function OnLogin(event, player)
    -- problematic code
end
RegisterPlayerEvent(3, OnLogin)
```

**Error messages:**
```
Paste any error messages from the log here
```

**Additional context**
Any other information that might be helpful.
```

## 📖 Improving Documentation

Documentation is crucial for helping users understand ALE.

### What to Document

- **API Functions**: Descriptions, parameters, return values, examples
- **Events**: When they trigger, what parameters they pass
- **Best Practices**: Common patterns and anti-patterns
- **Examples**: Real-world use cases and complete scripts

### Documentation Workflow

1. Fork the repository
2. Edit documentation files in the `docs/` folder
3. Use clear, concise language
4. Add code examples where helpful
5. Test any code examples you include
6. Submit a Pull Request

### Markdown Tips

```markdown
# Use headers for structure

## Subsections help organize content

**Bold** for emphasis

`code` for inline code

```lua
-- Code blocks for examples
```

> [!TIP]
> Use callouts for important information

- Lists
- Keep
- Things
- Organized
```

## 🎮 Creating Scripts

Share your scripts with the community!

### Where to Share

- **Discord**: [ALE Community](https://discord.com/invite/bx3y5Qmy)
- **GitHub Discussions**: For detailed explanations
- **Pull Requests**: Add example scripts to the repository

### Script Quality Guidelines

Good example scripts should:

1. **Be Well Commented**: Explain what the code does
2. **Handle Edge Cases**: Check for nil values, validate input
3. **Follow Best Practices**: Use local variables, cache data, etc.
4. **Include Usage Instructions**: How to install and configure
5. **Be Self-Contained**: Include all necessary code

### Example Template

```lua
--[[
Script Name: Player Welcome System
Description: Sends customized welcome message based on player level
Author: YourName
Version: 1.0
Date: 2025-01-09

Installation:
1. Place this file in lua_scripts folder
2. Configure MESSAGES table below
3. Restart server

Features:
- Different messages for different level ranges
- Personalized with player name
- Configurable message tiers
]]

-- Configuration
local MESSAGES = {
    [1] = "Welcome to the server, %s! Enjoy your adventure!",
    [10] = "Welcome back, %s! You've grown stronger!",
    [60] = "Greetings, veteran %s! The endgame awaits!",
    [80] = "Welcome, champion %s! You've mastered your class!"
}

-- Get appropriate message for player level
local function GetWelcomeMessage(player)
    local level = player:GetLevel()
    local message = MESSAGES[1]  -- Default message

    -- Find highest tier message player qualifies for
    for minLevel, msg in pairs(MESSAGES) do
        if level >= minLevel then
            message = msg
        end
    end

    return string.format(message, player:GetName())
end

-- Event handler
local function OnLogin(event, player)
    if not player then
        return
    end

    local message = GetWelcomeMessage(player)
    player:SendBroadcastMessage(message)
end

-- Register the event
RegisterPlayerEvent(3, OnLogin)  -- PLAYER_EVENT_ON_LOGIN
```

## 🔨 Development Guidelines

### Project Structure

```
mod-ale/
├── src/
│   └── LuaEngine/          # Core ALE source code
│       ├── docs/           # Source documentation
│       ├── methods/        # API method implementations
│       └── hooks/          # Event hook implementations
├── docs/                   # User-facing documentation
├── conf/                   # Configuration file templates
└── README.md              # Project overview
```

### Branch Strategy

- **master**: Stable, production-ready code
- **feature/**: New features (e.g., `feature/add-new-hook`)
- **fix/**: Bug fixes (e.g., `fix/player-null-check`)
- **docs/**: Documentation updates (e.g., `docs/update-install-guide`)

### Pull Request Guidelines

**Good PR:**
- Single, focused change
- Clear title and description
- Links related issues
- Passes compilation
- Includes documentation updates if needed

**PR Template:**

```markdown
## Description
Brief description of the changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement

## Testing
Describe how you tested your changes

## Related Issues
Fixes #123
Related to #456

## Checklist
- [ ] Code compiles without errors
- [ ] Tested on a running server
- [ ] Updated documentation
- [ ] Followed code style guidelines
```

---

## 🌟 Acknowledgements

ALE is built upon the foundation of the [Eluna Lua Engine](https://github.com/ElunaLuaEngine/Eluna). We acknowledge and thank the Eluna team for their pioneering work in Lua scripting for World of Warcraft server emulators.

- **[Original Eluna Repository](https://github.com/ElunaLuaEngine/Eluna)**
- **[Eluna Discord Community](https://discord.gg/bjkCVWqqfX)**

---

<div align="center">
<sub>Developed with ❤️ by the AzerothCore and ALE community</sub>

Thank you for contributing! 🎉

[⬆ Back to Top](#-contributing-to-ale)
</div>
