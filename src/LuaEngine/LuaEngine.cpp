/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "Hooks.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "Chat.h"
#include "ALECompat.h"
#include "ALEEventMgr.h"
#include "ALEIncludes.h"
#include "ALETemplate.h"
#include "ALEUtility.h"
#include "ALECreatureAI.h"
#include "ALEInstanceAI.h"

#if AC_PLATFORM == AC_PLATFORM_WINDOWS
#define ALE_WINDOWS
#endif

// Some dummy includes containing BOOST_VERSION:
// ObjectAccessor.h Config.h Log.h
#define USING_BOOST

#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <ctime>
#include <sys/stat.h>
#include <unordered_map>

extern "C"
{
// Base lua libraries
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// Additional lua libraries
};

ALE::ScriptList ALE::lua_scripts;
ALE::ScriptList ALE::lua_extensions;
std::string ALE::lua_folderpath;
std::string ALE::lua_requirepath;
std::string ALE::lua_requirecpath;
ALE* ALE::GALE = NULL;
bool ALE::reload = false;
bool ALE::initialized = false;
ALE::LockType ALE::lock;
std::unique_ptr<ALEFileWatcher> ALE::fileWatcher;

// Global bytecode cache that survives ALE reloads
static std::unordered_map<std::string, GlobalCacheEntry> globalBytecodeCache;
static std::unordered_map<std::string, std::time_t> timestampCache;
static std::mutex globalCacheMutex;

extern void RegisterFunctions(ALE* E);

void ALE::Initialize()
{
    LOCK_ALE;
    ASSERT(!IsInitialized());

    // For instance data the data column needs to be able to hold more than 255 characters (tinytext)
    // so we change it to TEXT automatically on startup
    CharacterDatabase.DirectExecute("ALTER TABLE `instance` CHANGE COLUMN `data` `data` TEXT NOT NULL");

    LoadScriptPaths();

    // Must be before creating GALE
    // This is checked on ALE creation
    initialized = true;

    // Create global ALE
    GALE = new ALE();

    // Start file watcher if enabled
    if (ALEConfig::GetInstance().IsAutoReloadEnabled())
    {
        uint32 watchInterval = eConfigMgr->GetOption<uint32>("ALE.AutoReloadInterval", 1);
        fileWatcher = std::make_unique<ALEFileWatcher>();
        fileWatcher->StartWatching(lua_folderpath, watchInterval);
    }
}

void ALE::Uninitialize()
{
    LOCK_ALE;
    ASSERT(IsInitialized());

    // Stop file watcher
    if (fileWatcher)
    {
        fileWatcher->StopWatching();
        fileWatcher.reset();
    }

    delete GALE;
    GALE = NULL;

    lua_scripts.clear();
    lua_extensions.clear();

    // Clear global cache on shutdown
    ClearGlobalCache();

    initialized = false;
}

void ALE::LoadScriptPaths()
{
    uint32 oldMSTime = ALEUtil::GetCurrTime();

    lua_scripts.clear();
    lua_extensions.clear();

    lua_folderpath = ALEConfig::GetInstance().GetScriptPath();
    const std::string& lua_path_extra = static_cast<std::string>(ALEConfig::GetInstance().GetRequirePath());
    const std::string& lua_cpath_extra = static_cast<std::string>(ALEConfig::GetInstance().GetRequireCPath());

#ifndef ALE_WINDOWS
    if (lua_folderpath[0] == '~')
        if (const char* home = getenv("HOME"))
            lua_folderpath.replace(0, 1, home);
#endif
    ALE_LOG_INFO("[ALE]: Searching scripts from `{}`", lua_folderpath);

    // clear all cache variables
    lua_requirepath.clear();
    lua_requirecpath.clear();

    GetScripts(lua_folderpath);

    // append our custom require paths and cpaths if the config variables are not empty
    if (!lua_path_extra.empty())
        lua_requirepath += lua_path_extra;

    if (!lua_cpath_extra.empty())
        lua_requirecpath += lua_cpath_extra;

    // Erase last ;
    if (!lua_requirepath.empty())
        lua_requirepath.erase(lua_requirepath.end() - 1);

    if (!lua_requirecpath.empty())
        lua_requirecpath.erase(lua_requirecpath.end() - 1);

    ALE_LOG_DEBUG("[ALE]: Loaded {} scripts in {} ms", lua_scripts.size() + lua_extensions.size(), ALEUtil::GetTimeDiff(oldMSTime));
}

void ALE::_ReloadALE()
{
    LOCK_ALE;
    ASSERT(IsInitialized());

    if (eConfigMgr->GetOption<bool>("ALE.PlayerAnnounceReload", false))
        eWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, "Reloading ALE...");
    else
        ChatHandler(nullptr).SendGMText(SERVER_MSG_STRING, "Reloading ALE...");

    // Remove all timed events
    sALE->eventMgr->SetStates(LUAEVENT_STATE_ERASE);

    // Close lua
    sALE->CloseLua();

    // Reload script paths
    LoadScriptPaths();

    // Open new lua and libaraies
    sALE->OpenLua();

    // Run scripts from laoded paths
    sALE->RunScripts();

    reload = false;
}

ALE::ALE() :
event_level(0),
push_counter(0),

L(NULL),
eventMgr(NULL),
httpManager(),
queryProcessor(),

ServerEventBindings(NULL),
PlayerEventBindings(NULL),
GuildEventBindings(NULL),
GroupEventBindings(NULL),
VehicleEventBindings(NULL),
BGEventBindings(NULL),
AllCreatureEventBindings(NULL),

PacketEventBindings(NULL),
CreatureEventBindings(NULL),
CreatureGossipBindings(NULL),
GameObjectEventBindings(NULL),
GameObjectGossipBindings(NULL),
ItemEventBindings(NULL),
ItemGossipBindings(NULL),
PlayerGossipBindings(NULL),
MapEventBindings(NULL),
InstanceEventBindings(NULL),
TicketEventBindings(NULL),
SpellEventBindings(NULL),

CreatureUniqueBindings(NULL)
{
    ASSERT(IsInitialized());

    OpenLua();

    // Replace this with map insert if making multithread version

    // Set event manager. Must be after setting sALE
    // on multithread have a map of state pointers and here insert this pointer to the map and then save a pointer of that pointer to the EventMgr
    eventMgr = new EventMgr(&ALE::GALE);
}

ALE::~ALE()
{
    ASSERT(IsInitialized());

    CloseLua();

    delete eventMgr;
    eventMgr = NULL;
}

void ALE::CloseLua()
{
    OnLuaStateClose();

    DestroyBindStores();

    // Must close lua state after deleting stores and mgr
    if (L)
        lua_close(L);
    L = NULL;

    instanceDataRefs.clear();
    continentDataRefs.clear();
}

void ALE::OpenLua()
{
    if (!ALEConfig::GetInstance().IsALEEnabled())
    {
        ALE_LOG_INFO("[ALE]: ALE is disabled in config");
        return;
    }

    L = luaL_newstate();

    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, ALE_STATE_PTR);

    CreateBindStores();

    // open base lua libraries
    luaL_openlibs(L);

    // open additional lua libraries

    // Register methods and functions
    RegisterFunctions(this);

    // Set lua require folder paths (scripts folder structure)
    lua_getglobal(L, "package");
    lua_pushstring(L, GetRequirePath().c_str());
    lua_setfield(L, -2, "path");
    lua_pushstring(L, GetRequireCPath().c_str());
    lua_setfield(L, -2, "cpath");

    // Set package.loaders loader for precompiled scripts
    lua_getfield(L, -1, "loaders");
    if (lua_isnil(L, -1)) {
        // Lua 5.2+ uses searchers instead of loaders
        lua_pop(L, 1);
        lua_getfield(L, -1, "searchers");
    }

    lua_pop(L, 1);
}

void ALE::CreateBindStores()
{
    DestroyBindStores();

    ServerEventBindings      = new BindingMap< EventKey<Hooks::ServerEvents> >(L);
    PlayerEventBindings      = new BindingMap< EventKey<Hooks::PlayerEvents> >(L);
    GuildEventBindings       = new BindingMap< EventKey<Hooks::GuildEvents> >(L);
    GroupEventBindings       = new BindingMap< EventKey<Hooks::GroupEvents> >(L);
    VehicleEventBindings     = new BindingMap< EventKey<Hooks::VehicleEvents> >(L);
    BGEventBindings          = new BindingMap< EventKey<Hooks::BGEvents> >(L);
    TicketEventBindings      = new BindingMap< EventKey<Hooks::TicketEvents> >(L);
    AllCreatureEventBindings = new BindingMap< EventKey<Hooks::AllCreatureEvents> >(L);

    PacketEventBindings      = new BindingMap< EntryKey<Hooks::PacketEvents> >(L);
    CreatureEventBindings    = new BindingMap< EntryKey<Hooks::CreatureEvents> >(L);
    CreatureGossipBindings   = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    GameObjectEventBindings  = new BindingMap< EntryKey<Hooks::GameObjectEvents> >(L);
    GameObjectGossipBindings = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    ItemEventBindings        = new BindingMap< EntryKey<Hooks::ItemEvents> >(L);
    ItemGossipBindings       = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    PlayerGossipBindings     = new BindingMap< EntryKey<Hooks::GossipEvents> >(L);
    MapEventBindings         = new BindingMap< EntryKey<Hooks::InstanceEvents> >(L);
    InstanceEventBindings    = new BindingMap< EntryKey<Hooks::InstanceEvents> >(L);
    SpellEventBindings       = new BindingMap< EntryKey<Hooks::SpellEvents> >(L);

    CreatureUniqueBindings   = new BindingMap< UniqueObjectKey<Hooks::CreatureEvents> >(L);
}

void ALE::DestroyBindStores()
{
    delete ServerEventBindings;
    delete PlayerEventBindings;
    delete GuildEventBindings;
    delete GroupEventBindings;
    delete VehicleEventBindings;
    delete AllCreatureEventBindings;

    delete PacketEventBindings;
    delete CreatureEventBindings;
    delete CreatureGossipBindings;
    delete GameObjectEventBindings;
    delete GameObjectGossipBindings;
    delete ItemEventBindings;
    delete ItemGossipBindings;
    delete PlayerGossipBindings;
    delete BGEventBindings;
    delete MapEventBindings;
    delete InstanceEventBindings;
    delete SpellEventBindings;

    delete CreatureUniqueBindings;

    ServerEventBindings = NULL;
    PlayerEventBindings = NULL;
    GuildEventBindings = NULL;
    GroupEventBindings = NULL;
    VehicleEventBindings = NULL;
    AllCreatureEventBindings = NULL;

    PacketEventBindings = NULL;
    CreatureEventBindings = NULL;
    CreatureGossipBindings = NULL;
    GameObjectEventBindings = NULL;
    GameObjectGossipBindings = NULL;
    ItemEventBindings = NULL;
    ItemGossipBindings = NULL;
    PlayerGossipBindings = NULL;
    BGEventBindings = NULL;
    MapEventBindings = NULL;
    InstanceEventBindings = NULL;
    SpellEventBindings = NULL;

    CreatureUniqueBindings = NULL;
}

void ALE::AddScriptPath(std::string filename, const std::string& fullpath)
{
    ALE_LOG_DEBUG("[ALE]: AddScriptPath Checking file `{}`", fullpath);

    // split file name
    std::size_t extDot = filename.find_last_of('.');
    if (extDot == std::string::npos)
        return;
    std::string ext = filename.substr(extDot);
    filename = filename.substr(0, extDot);

    // check extension and add path to scripts to load
    if (ext != ".lua" && ext != ".dll" && ext != ".so" && ext != ".ext" && ext !=".moon" && ext != ".out")
        return;
    bool extension = ext == ".ext";

    LuaScript script;
    script.fileext = ext;
    script.filename = filename;
    script.filepath = fullpath;
    script.modulepath = fullpath.substr(0, fullpath.length() - filename.length() - ext.length());
    if (extension)
        lua_extensions.push_back(script);
    else
        lua_scripts.push_back(script);
    ALE_LOG_DEBUG("[ALE]: AddScriptPath add path `{}`", fullpath);
}

std::time_t ALE::GetFileModTime(const std::string& filepath)
{
    struct stat fileInfo;
    if (stat(filepath.c_str(), &fileInfo) == 0)
        return fileInfo.st_mtime;
    return 0;
}

std::time_t ALE::GetFileModTimeWithCache(const std::string& filepath)
{
    auto it = timestampCache.find(filepath);
    if (it != timestampCache.end())
        return it->second;
    
    std::time_t modTime = GetFileModTime(filepath);
    timestampCache[filepath] = modTime;
    return modTime;
}

bool ALE::CompileScriptToGlobalCache(const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(globalCacheMutex);
    
    lua_State* tempL = luaL_newstate();
    if (!tempL)
        return false;

    int result = luaL_loadfile(tempL, filepath.c_str());
    if (result != LUA_OK)
    {
        lua_close(tempL);
        return false;
    }

    std::time_t modTime = GetFileModTime(filepath);
    
    auto& cacheEntry = globalBytecodeCache[filepath];
    cacheEntry.last_modified = modTime;
    cacheEntry.filepath = filepath;
    cacheEntry.bytecode.clear();
    cacheEntry.bytecode.reserve(1024);

    struct BytecodeWriter {
        BytecodeBuffer* buffer;
        static int writer(lua_State*, const void* p, size_t sz, void* ud) {
            BytecodeWriter* w = static_cast<BytecodeWriter*>(ud);
            const uint8* bytes = static_cast<const uint8*>(p);
            w->buffer->insert(w->buffer->end(), bytes, bytes + sz);
            return 0;
        }
    };

    BytecodeWriter writer;
    writer.buffer = &cacheEntry.bytecode;

    int dumpResult = lua_dump(tempL, BytecodeWriter::writer, &writer);
    if (dumpResult != LUA_OK || cacheEntry.bytecode.empty())
    {
        globalBytecodeCache.erase(filepath);
        lua_close(tempL);
        return false;
    }

    lua_close(tempL);
    return true;
}

bool ALE::CompileMoonScriptToGlobalCache(const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(globalCacheMutex);
    
    lua_State* tempL = luaL_newstate();
    if (!tempL)
        return false;

    luaL_openlibs(tempL);

    std::string moonscriptLoader = "return require('moonscript').loadfile([[" + filepath + "]])";
    int result = luaL_loadstring(tempL, moonscriptLoader.c_str());
    if (result != LUA_OK)
    {
        lua_close(tempL);
        return false;
    }

    result = lua_pcall(tempL, 0, 1, 0);
    if (result != LUA_OK)
    {
        lua_close(tempL);
        return false;
    }

    std::time_t modTime = GetFileModTime(filepath);
    
    auto& cacheEntry = globalBytecodeCache[filepath];
    cacheEntry.last_modified = modTime;
    cacheEntry.filepath = filepath;
    cacheEntry.bytecode.clear();
    cacheEntry.bytecode.reserve(2048);

    struct BytecodeWriter {
        BytecodeBuffer* buffer;
        static int writer(lua_State*, const void* p, size_t sz, void* ud) {
            BytecodeWriter* w = static_cast<BytecodeWriter*>(ud);
            const uint8* bytes = static_cast<const uint8*>(p);
            w->buffer->insert(w->buffer->end(), bytes, bytes + sz);
            return 0;
        }
    };

    BytecodeWriter writer;
    writer.buffer = &cacheEntry.bytecode;

    int dumpResult = lua_dump(tempL, BytecodeWriter::writer, &writer);
    if (dumpResult != LUA_OK || cacheEntry.bytecode.empty())
    {
        globalBytecodeCache.erase(filepath);
        lua_close(tempL);
        return false;
    }

    lua_close(tempL);
    return true;
}

int ALE::TryLoadFromGlobalCache(lua_State* L, const std::string& filepath)
{
    std::lock_guard<std::mutex> lock(globalCacheMutex);
    
    auto it = globalBytecodeCache.find(filepath);
    if (it == globalBytecodeCache.end() || it->second.bytecode.empty())
        return LUA_ERRFILE;
    
    std::time_t currentModTime = GetFileModTimeWithCache(filepath);
    if (it->second.last_modified != currentModTime || currentModTime == 0)
        return LUA_ERRFILE;
    
    return luaL_loadbuffer(L, reinterpret_cast<const char*>(it->second.bytecode.data()), it->second.bytecode.size(), filepath.c_str());
}

int ALE::LoadScriptWithCache(lua_State* L, const std::string& filepath, bool isMoonScript, uint32* compiledCount, uint32* cachedCount)
{
    bool cacheEnabled = ALEConfig::GetInstance().IsByteCodeCacheEnabled();
    
    if (cacheEnabled)
    {
        int result = TryLoadFromGlobalCache(L, filepath);
        if (result == LUA_OK)
        {
            if (cachedCount) (*cachedCount)++;
            return LUA_OK;
        }
        
        bool compileSuccess = isMoonScript ? 
            CompileMoonScriptToGlobalCache(filepath) : 
            CompileScriptToGlobalCache(filepath);
            
        if (compileSuccess)
        {
            if (compiledCount) (*compiledCount)++;
            std::lock_guard<std::mutex> lock(globalCacheMutex);
            auto it = globalBytecodeCache.find(filepath);
            if (it != globalBytecodeCache.end() && !it->second.bytecode.empty())
            {
                result = luaL_loadbuffer(L, reinterpret_cast<const char*>(it->second.bytecode.data()), it->second.bytecode.size(), filepath.c_str());
                if (result == LUA_OK)
                    return LUA_OK;
            }
        }
    }
    
    if (isMoonScript)
    {
        std::string str = "return require('moonscript').loadfile([[" + filepath + "]])";
        int result = luaL_loadstring(L, str.c_str());
        if (result != LUA_OK)
            return result;
        return lua_pcall(L, 0, LUA_MULTRET, 0);
    }
    else
    {
        return luaL_loadfile(L, filepath.c_str());
    }
}

void ALE::ClearGlobalCache()
{
    std::lock_guard<std::mutex> lock(globalCacheMutex);
    globalBytecodeCache.clear();
    timestampCache.clear();
    ALE_LOG_INFO("[ALE]: Global bytecode cache cleared");
}

void ALE::ClearTimestampCache()
{
    std::lock_guard<std::mutex> lock(globalCacheMutex);
    timestampCache.clear();
}

size_t ALE::GetGlobalCacheSize()
{
    std::lock_guard<std::mutex> lock(globalCacheMutex);
    return globalBytecodeCache.size();
}

int ALE::LoadCompiledScript(lua_State* L, const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
        return LUA_ERRFILE;

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    return luaL_loadbuffer(L, buffer.data(), fileSize, filepath.c_str());
}

// Finds lua script files from given path (including subdirectories) and pushes them to scripts
void ALE::GetScripts(std::string path)
{
    ALE_LOG_DEBUG("[ALE]: GetScripts from path `{}`", path);

    boost::filesystem::path someDir(path);
    boost::filesystem::directory_iterator end_iter;

    if (boost::filesystem::exists(someDir) && boost::filesystem::is_directory(someDir))
    {
        lua_requirepath +=
            path + "/?.lua;" +
            path + "/?.moon;" +
            path + "/?.ext;";
        
        lua_requirecpath +=
            path + "/?.dll;" +
            path + "/?.so;";

        for (boost::filesystem::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
        {
            std::string fullpath = dir_iter->path().generic_string();

            // Check if file is hidden
#ifdef ALE_WINDOWS
            DWORD dwAttrib = GetFileAttributes(fullpath.c_str());
            if (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_HIDDEN))
                continue;
#else
            std::string name = dir_iter->path().filename().generic_string().c_str();
            if (name[0] == '.')
                continue;
#endif

            // load subfolder
            if (boost::filesystem::is_directory(dir_iter->status()))
            {
                GetScripts(fullpath);
                continue;
            }

            if (boost::filesystem::is_regular_file(dir_iter->status()))
            {
                // was file, try add
                std::string filename = dir_iter->path().filename().generic_string();
                AddScriptPath(filename, fullpath);
            }
        }
    }
}

static bool ScriptPathComparator(const LuaScript& first, const LuaScript& second)
{
    return first.filepath < second.filepath;
}

void ALE::RunScripts()
{
    LOCK_ALE;
    if (!ALEConfig::GetInstance().IsALEEnabled())
        return;

    uint32 oldMSTime = ALEUtil::GetCurrTime();
    uint32 count = 0;
    uint32 compiledCount = 0;
    uint32 cachedCount = 0;
    uint32 precompiledCount = 0;
    bool cacheEnabled = eConfigMgr->GetOption<bool>("ALE.BytecodeCache", true);
    
    if (cacheEnabled)
        ClearTimestampCache();

    ScriptList scripts;
    lua_extensions.sort(ScriptPathComparator);
    lua_scripts.sort(ScriptPathComparator);
    scripts.insert(scripts.end(), lua_extensions.begin(), lua_extensions.end());
    scripts.insert(scripts.end(), lua_scripts.begin(), lua_scripts.end());

    std::unordered_map<std::string, std::string> loaded; // filename, path

    lua_getglobal(L, "package");
    // Stack: package
    luaL_getsubtable(L, -1, "loaded");
    // Stack: package, modules
    int modules = lua_gettop(L);
    for (ScriptList::iterator it = scripts.begin(); it != scripts.end(); ++it)
    {
        // Check that no duplicate names exist
        if (loaded.find(it->filename) != loaded.end())
        {
            ALE_LOG_ERROR("[ALE]: Error loading `{}`. File with same name already loaded from `{}`, rename either file", it->filepath, loaded[it->filename]);
            continue;
        }
        loaded[it->filename] = it->filepath;

        lua_getfield(L, modules, it->filename.c_str());
        // Stack: package, modules, module
        if (!lua_isnoneornil(L, -1))
        {
            lua_pop(L, 1);
            ALE_LOG_DEBUG("[ALE]: `{}` was already loaded or required", it->filepath);
            continue;
        }
        lua_pop(L, 1);
        // Stack: package, modules

        if (it->fileext == ".moon")
        {
            if (LoadScriptWithCache(L, it->filepath, true, &compiledCount, &cachedCount))
            {
                // Stack: package, modules, errmsg
                ALE_LOG_ERROR("[ALE]: Error loading MoonScript `{}`", it->filepath);
                Report(L);
                // Stack: package, modules
                continue;
            }
        }
        else if (it->fileext == ".out")
        {
            if (LoadCompiledScript(L, it->filepath))
            {
                // Stack: package, modules, errmsg
                ALE_LOG_ERROR("[ALE]: Error loading compiled script `{}`", it->filepath);
                Report(L);
                // Stack: package, modules
                continue;
            }
            precompiledCount++;
        }
        else if (it->fileext == ".lua" || it->fileext == ".ext")
        {
            if (LoadScriptWithCache(L, it->filepath, false, &compiledCount, &cachedCount))
            {
                // Stack: package, modules, errmsg
                ALE_LOG_ERROR("[ALE]: Error loading `{}`", it->filepath);
                Report(L);
                // Stack: package, modules
                continue;
            }
        }
        else
        {
           if (luaL_loadfile(L, it->filepath.c_str()))
           {
               // Stack: package, modules, errmsg
               ALE_LOG_ERROR("[ALE]: Error loading `{}`", it->filepath);
               Report(L);
               // Stack: package, modules
               continue;
           }
        }

        // Stack: package, modules, filefunc
        if (ExecuteCall(0, 1))
        {
            // Stack: package, modules, result
            if (lua_isnoneornil(L, -1) || (lua_isboolean(L, -1) && !lua_toboolean(L, -1)))
            {
                // if result evaluates to false, change it to true
                lua_pop(L, 1);
                Push(L, true);
            }
            lua_setfield(L, modules, it->filename.c_str());
            // Stack: package, modules

            // successfully loaded and ran file
            ALE_LOG_DEBUG("[ALE]: Successfully loaded `{}`", it->filepath);
            ++count;
            continue;
        }
    }
    // Stack: package, modules
    lua_pop(L, 2);
    
    std::string details = "";
    if (cacheEnabled && (compiledCount > 0 || cachedCount > 0 || precompiledCount > 0))
    {
        details = fmt::format("({} compiled, {} cached, {} pre-compiled)", compiledCount, cachedCount, precompiledCount);
    }
    ALE_LOG_INFO("[ALE]: Executed {} Lua scripts in {} ms {}", count, ALEUtil::GetTimeDiff(oldMSTime), details);

    OnLuaStateOpen();
}

void ALE::InvalidateObjects()
{
    ++callstackid;
    ASSERT(callstackid && "Callstackid overflow");
}

void ALE::Report(lua_State* _L)
{
    const char* msg = lua_tostring(_L, -1);
    ALE_LOG_ERROR("{}", msg);
    lua_pop(_L, 1);
}

// Borrowed from http://stackoverflow.com/questions/12256455/print-stacktrace-from-c-code-with-embedded-lua
int ALE::StackTrace(lua_State *_L)
{
    // Stack: errmsg
    if (!lua_isstring(_L, -1))  /* 'message' not a string? */
        return 1;  /* keep it intact */
    // Stack: errmsg, debug
    lua_getglobal(_L, "debug");
    if (!lua_istable(_L, -1))
    {
        lua_pop(_L, 1);
        return 1;
    }
    // Stack: errmsg, debug, traceback
    lua_getfield(_L, -1, "traceback");
    if (!lua_isfunction(_L, -1))
    {
        lua_pop(_L, 2);
        return 1;
    }
    lua_pushvalue(_L, -3);  /* pass error message */
    lua_pushinteger(_L, 1);  /* skip this function and traceback */
    // Stack: errmsg, debug, traceback, errmsg, 2
    lua_call(_L, 2, 1);  /* call debug.traceback */

    // dirty stack?
    // Stack: errmsg, debug, tracemsg
    sALE->OnError(std::string(lua_tostring(_L, -1)));
    return 1;
}

bool ALE::ExecuteCall(int params, int res)
{
    int top = lua_gettop(L);
    int base = top - params;

    // Expected: function, [parameters]
    ASSERT(base > 0);

    // Check function type
    if (!lua_isfunction(L, base))
    {
        ALE_LOG_ERROR("[ALE]: Cannot execute call: registered value is {}, not a function.", luaL_tolstring(L, base, NULL));
        ASSERT(false); // stack probably corrupt
    }

    bool usetrace = ALEConfig::GetInstance().IsTraceBackEnabled();
    if (usetrace)
    {
        lua_pushcfunction(L, &StackTrace);
        // Stack: function, [parameters], traceback
        lua_insert(L, base);
        // Stack: traceback, function, [parameters]
    }

    // Objects are invalidated when event_level hits 0
    ++event_level;
    int result = lua_pcall(L, params, res, usetrace ? base : 0);
    --event_level;

    if (usetrace)
    {
        // Stack: traceback, [results or errmsg]
        lua_remove(L, base);
    }
    // Stack: [results or errmsg]

    // lua_pcall returns 0 on success.
    // On error print the error and push nils for expected amount of returned values
    if (result)
    {
        // Stack: errmsg
        Report(L);

        // Force garbage collect
        lua_gc(L, LUA_GCCOLLECT, 0);

        // Push nils for expected amount of results
        for (int i = 0; i < res; ++i)
            lua_pushnil(L);
        // Stack: [nils]
        return false;
    }

    // Stack: [results]
    return true;
}

void ALE::Push(lua_State* luastate)
{
    lua_pushnil(luastate);
}
void ALE::Push(lua_State* luastate, const long long l)
{
    ALETemplate<long long>::Push(luastate, new long long(l));
}
void ALE::Push(lua_State* luastate, const unsigned long long l)
{
    ALETemplate<unsigned long long>::Push(luastate, new unsigned long long(l));
}
void ALE::Push(lua_State* luastate, const long l)
{
    Push(luastate, static_cast<long long>(l));
}
void ALE::Push(lua_State* luastate, const unsigned long l)
{
    Push(luastate, static_cast<unsigned long long>(l));
}
void ALE::Push(lua_State* luastate, const int i)
{
    lua_pushinteger(luastate, i);
}
void ALE::Push(lua_State* luastate, const unsigned int u)
{
    lua_pushunsigned(luastate, u);
}
void ALE::Push(lua_State* luastate, const double d)
{
    lua_pushnumber(luastate, d);
}
void ALE::Push(lua_State* luastate, const float f)
{
    lua_pushnumber(luastate, f);
}
void ALE::Push(lua_State* luastate, const bool b)
{
    lua_pushboolean(luastate, b);
}
void ALE::Push(lua_State* luastate, const std::string& str)
{
    lua_pushstring(luastate, str.c_str());
}
void ALE::Push(lua_State* luastate, const char* str)
{
    lua_pushstring(luastate, str);
}
void ALE::Push(lua_State* luastate, Pet const* pet)
{
    Push<Creature>(luastate, pet);
}
void ALE::Push(lua_State* luastate, TempSummon const* summon)
{
    Push<Creature>(luastate, summon);
}
void ALE::Push(lua_State* luastate, Unit const* unit)
{
    if (!unit)
    {
        Push(luastate);
        return;
    }
    switch (unit->GetTypeId())
    {
        case TYPEID_UNIT:
            Push(luastate, unit->ToCreature());
            break;
        case TYPEID_PLAYER:
            Push(luastate, unit->ToPlayer());
            break;
        default:
            ALETemplate<Unit>::Push(luastate, unit);
    }
}
void ALE::Push(lua_State* luastate, WorldObject const* obj)
{
    if (!obj)
    {
        Push(luastate);
        return;
    }
    switch (obj->GetTypeId())
    {
        case TYPEID_UNIT:
            Push(luastate, obj->ToCreature());
            break;
        case TYPEID_PLAYER:
            Push(luastate, obj->ToPlayer());
            break;
        case TYPEID_GAMEOBJECT:
            Push(luastate, obj->ToGameObject());
            break;
        case TYPEID_CORPSE:
            Push(luastate, obj->ToCorpse());
            break;
        default:
            ALETemplate<WorldObject>::Push(luastate, obj);
    }
}
void ALE::Push(lua_State* luastate, Object const* obj)
{
    if (!obj)
    {
        Push(luastate);
        return;
    }
    switch (obj->GetTypeId())
    {
        case TYPEID_UNIT:
            Push(luastate, obj->ToCreature());
            break;
        case TYPEID_PLAYER:
            Push(luastate, obj->ToPlayer());
            break;
        case TYPEID_GAMEOBJECT:
            Push(luastate, obj->ToGameObject());
            break;
        case TYPEID_CORPSE:
            Push(luastate, obj->ToCorpse());
            break;
        default:
            ALETemplate<Object>::Push(luastate, obj);
    }
}
void ALE::Push(lua_State* luastate, ObjectGuid const guid)
{
    ALETemplate<unsigned long long>::Push(luastate, new unsigned long long(guid.GetRawValue()));
}

void ALE::Push(lua_State* luastate, GemPropertiesEntry const& gemProperties)
{
    Push(luastate, &gemProperties);
}

void ALE::Push(lua_State* luastate, SpellEntry const& spell)
{
    Push(luastate, &spell);
}

void ALE::Push(lua_State* luastate, CreatureTemplate const* creatureTemplate)
{
    Push<CreatureTemplate>(luastate, creatureTemplate);
}

std::string ALE::FormatQuery(lua_State* L, const char* query)
{
    int numArgs = lua_gettop(L);
    std::string formattedQuery = query;

    size_t position = 0;
    for (int i = 2; i <= numArgs; ++i) 
    {
        std::string arg;

        if (lua_isnumber(L, i)) 
        {
            arg = std::to_string(lua_tonumber(L, i));
        } 
        else if (lua_isstring(L, i)) 
        {
            std::string value = lua_tostring(L, i);
            for (size_t pos = 0; (pos = value.find('\'', pos)) != std::string::npos; pos += 2)
            {
                value.insert(pos, "'");
            }
            arg = "'" + value + "'";
        } 
        else 
        {
            luaL_error(L, "Unsupported argument type. Only numbers and strings are supported.");
            return "";
        }

        position = formattedQuery.find("?", position);
        if (position == std::string::npos) 
        {
            luaL_error(L, "Mismatch between placeholders and arguments.");
            return "";
        }
        formattedQuery.replace(position, 1, arg);
        position += arg.length();
    }

    return formattedQuery;
}

static int CheckIntegerRange(lua_State* luastate, int narg, int min, int max)
{
    double value = luaL_checknumber(luastate, narg);
    char error_buffer[64];

    if (value > max)
    {
        snprintf(error_buffer, 64, "value must be less than or equal to %i", max);
        return luaL_argerror(luastate, narg, error_buffer);
    }

    if (value < min)
    {
        snprintf(error_buffer, 64, "value must be greater than or equal to %i", min);
        return luaL_argerror(luastate, narg, error_buffer);
    }

    return static_cast<int>(value);
}

static unsigned int CheckUnsignedRange(lua_State* luastate, int narg, unsigned int max)
{
    double value = luaL_checknumber(luastate, narg);

    if (value < 0)
        return luaL_argerror(luastate, narg, "value must be greater than or equal to 0");

    if (value > max)
    {
        char error_buffer[64];
        snprintf(error_buffer, 64, "value must be less than or equal to %u", max);
        return luaL_argerror(luastate, narg, error_buffer);
    }

    return static_cast<unsigned int>(value);
}

template<> bool ALE::CHECKVAL<bool>(lua_State* luastate, int narg)
{
    return lua_toboolean(luastate, narg) != 0;
}
template<> float ALE::CHECKVAL<float>(lua_State* luastate, int narg)
{
    return static_cast<float>(luaL_checknumber(luastate, narg));
}
template<> double ALE::CHECKVAL<double>(lua_State* luastate, int narg)
{
    return luaL_checknumber(luastate, narg);
}
template<> signed char ALE::CHECKVAL<signed char>(lua_State* luastate, int narg)
{
    return CheckIntegerRange(luastate, narg, SCHAR_MIN, SCHAR_MAX);
}
template<> unsigned char ALE::CHECKVAL<unsigned char>(lua_State* luastate, int narg)
{
    return CheckUnsignedRange(luastate, narg, UCHAR_MAX);
}
template<> short ALE::CHECKVAL<short>(lua_State* luastate, int narg)
{
    return CheckIntegerRange(luastate, narg, SHRT_MIN, SHRT_MAX);
}
template<> unsigned short ALE::CHECKVAL<unsigned short>(lua_State* luastate, int narg)
{
    return CheckUnsignedRange(luastate, narg, USHRT_MAX);
}
template<> int ALE::CHECKVAL<int>(lua_State* luastate, int narg)
{
    return CheckIntegerRange(luastate, narg, INT_MIN, INT_MAX);
}
template<> unsigned int ALE::CHECKVAL<unsigned int>(lua_State* luastate, int narg)
{
    return CheckUnsignedRange(luastate, narg, UINT_MAX);
}
template<> const char* ALE::CHECKVAL<const char*>(lua_State* luastate, int narg)
{
    return luaL_checkstring(luastate, narg);
}
template<> std::string ALE::CHECKVAL<std::string>(lua_State* luastate, int narg)
{
    return luaL_checkstring(luastate, narg);
}
template<> long long ALE::CHECKVAL<long long>(lua_State* luastate, int narg)
{
    if (lua_isnumber(luastate, narg))
        return static_cast<long long>(CHECKVAL<double>(luastate, narg));
    return *(ALE::CHECKOBJ<long long>(luastate, narg, true));
}
template<> unsigned long long ALE::CHECKVAL<unsigned long long>(lua_State* luastate, int narg)
{
    if (lua_isnumber(luastate, narg))
        return static_cast<unsigned long long>(CHECKVAL<uint32>(luastate, narg));
    return *(ALE::CHECKOBJ<unsigned long long>(luastate, narg, true));
}
template<> long ALE::CHECKVAL<long>(lua_State* luastate, int narg)
{
    return static_cast<long>(CHECKVAL<long long>(luastate, narg));
}
template<> unsigned long ALE::CHECKVAL<unsigned long>(lua_State* luastate, int narg)
{
    return static_cast<unsigned long>(CHECKVAL<unsigned long long>(luastate, narg));
}
template<> ObjectGuid ALE::CHECKVAL<ObjectGuid>(lua_State* luastate, int narg)
{
    return ObjectGuid(uint64((CHECKVAL<unsigned long long>(luastate, narg))));
}

template<> Object* ALE::CHECKOBJ<Object>(lua_State* luastate, int narg, bool error)
{
    Object* obj = CHECKOBJ<WorldObject>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<Item>(luastate, narg, false);
    if (!obj)
        obj = ALETemplate<Object>::Check(luastate, narg, error);
    return obj;
}
template<> WorldObject* ALE::CHECKOBJ<WorldObject>(lua_State* luastate, int narg, bool error)
{
    WorldObject* obj = CHECKOBJ<Unit>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<GameObject>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<Corpse>(luastate, narg, false);
    if (!obj)
        obj = ALETemplate<WorldObject>::Check(luastate, narg, error);
    return obj;
}
template<> Unit* ALE::CHECKOBJ<Unit>(lua_State* luastate, int narg, bool error)
{
    Unit* obj = CHECKOBJ<Player>(luastate, narg, false);
    if (!obj)
        obj = CHECKOBJ<Creature>(luastate, narg, false);
    if (!obj)
        obj = ALETemplate<Unit>::Check(luastate, narg, error);
    return obj;
}

template<> ALEObject* ALE::CHECKOBJ<ALEObject>(lua_State* luastate, int narg, bool error)
{
    return CHECKTYPE(luastate, narg, NULL, error);
}

ALEObject* ALE::CHECKTYPE(lua_State* luastate, int narg, const char* tname, bool error)
{
    if (lua_islightuserdata(luastate, narg))
    {
        if (error)
            luaL_argerror(luastate, narg, "bad argument : userdata expected, got lightuserdata");
        return NULL;
    }

    ALEObject** ptrHold = static_cast<ALEObject**>(lua_touserdata(luastate, narg));

    if (!ptrHold || (tname && (*ptrHold)->GetTypeName() != tname))
    {
        if (error)
        {
            char buff[256];
            snprintf(buff, 256, "bad argument : %s expected, got %s", tname ? tname : "ALEObject", ptrHold ? (*ptrHold)->GetTypeName() : luaL_typename(luastate, narg));
            luaL_argerror(luastate, narg, buff);
        }
        return NULL;
    }
    return *ptrHold;
}

template<typename K>
static int cancelBinding(lua_State *L)
{
    uint64 bindingID = ALE::CHECKVAL<uint64>(L, lua_upvalueindex(1));

    BindingMap<K>* bindings = (BindingMap<K>*)lua_touserdata(L, lua_upvalueindex(2));
    ASSERT(bindings != NULL);

    bindings->Remove(bindingID);

    return 0;
}

template<typename K>
static void createCancelCallback(lua_State* L, uint64 bindingID, BindingMap<K>* bindings)
{
    ALE::Push(L, bindingID);
    lua_pushlightuserdata(L, bindings);
    // Stack: bindingID, bindings

    lua_pushcclosure(L, &cancelBinding<K>, 2);
    // Stack: cancel_callback
}

// Saves the function reference ID given to the register type's store for given entry under the given event
int ALE::Register(lua_State* L, uint8 regtype, uint32 entry, ObjectGuid guid, uint32 instanceId, uint32 event_id, int functionRef, uint32 shots)
{
    uint64 bindingID;

    switch (regtype)
    {
        case Hooks::REGTYPE_SERVER:
            if (event_id < Hooks::SERVER_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::ServerEvents>((Hooks::ServerEvents)event_id);
                bindingID = ServerEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, ServerEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_PLAYER:
            if (event_id < Hooks::PLAYER_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::PlayerEvents>((Hooks::PlayerEvents)event_id);
                bindingID = PlayerEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, PlayerEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GUILD:
            if (event_id < Hooks::GUILD_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::GuildEvents>((Hooks::GuildEvents)event_id);
                bindingID = GuildEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GuildEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GROUP:
            if (event_id < Hooks::GROUP_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::GroupEvents>((Hooks::GroupEvents)event_id);
                bindingID = GroupEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GroupEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_VEHICLE:
            if (event_id < Hooks::VEHICLE_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::VehicleEvents>((Hooks::VehicleEvents)event_id);
                bindingID = VehicleEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, VehicleEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_BG:
            if (event_id < Hooks::BG_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::BGEvents>((Hooks::BGEvents)event_id);
                bindingID = BGEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, BGEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_PACKET:
            if (event_id < Hooks::PACKET_EVENT_COUNT)
            {
                if (entry >= NUM_MSG_TYPES)
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a creature with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::PacketEvents>((Hooks::PacketEvents)event_id, entry);
                bindingID = PacketEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, PacketEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_CREATURE:
            if (event_id < Hooks::CREATURE_EVENT_COUNT)
            {
                if (entry != 0)
                {
                    if (!eObjectMgr->GetCreatureTemplate(entry))
                    {
                        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                        luaL_error(L, "Couldn't find a creature with (ID: %d)!", entry);
                        return 0; // Stack: (empty)
                    }

                    auto key = EntryKey<Hooks::CreatureEvents>((Hooks::CreatureEvents)event_id, entry);
                    bindingID = CreatureEventBindings->Insert(key, functionRef, shots);
                    createCancelCallback(L, bindingID, CreatureEventBindings);
                }
                else
                {
                    if (guid.IsEmpty())
                    {
                        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                        luaL_error(L, "guid was 0!");
                        return 0; // Stack: (empty)
                    }

                    auto key = UniqueObjectKey<Hooks::CreatureEvents>((Hooks::CreatureEvents)event_id, guid, instanceId);
                    bindingID = CreatureUniqueBindings->Insert(key, functionRef, shots);
                    createCancelCallback(L, bindingID, CreatureUniqueBindings);
                }
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_CREATURE_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!eObjectMgr->GetCreatureTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a creature with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = CreatureGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, CreatureGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GAMEOBJECT:
            if (event_id < Hooks::GAMEOBJECT_EVENT_COUNT)
            {
                if (!eObjectMgr->GetGameObjectTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a gameobject with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GameObjectEvents>((Hooks::GameObjectEvents)event_id, entry);
                bindingID = GameObjectEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GameObjectEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_GAMEOBJECT_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!eObjectMgr->GetGameObjectTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a gameobject with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = GameObjectGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, GameObjectGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_ITEM:
            if (event_id < Hooks::ITEM_EVENT_COUNT)
            {
                if (!eObjectMgr->GetItemTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a item with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::ItemEvents>((Hooks::ItemEvents)event_id, entry);
                bindingID = ItemEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, ItemEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_ITEM_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!eObjectMgr->GetItemTemplate(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a item with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = ItemGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, ItemGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_PLAYER_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                auto key = EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry);
                bindingID = PlayerGossipBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, PlayerGossipBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_MAP:
            if (event_id < Hooks::INSTANCE_EVENT_COUNT)
            {
                auto key = EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)event_id, entry);
                bindingID = MapEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, MapEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_INSTANCE:
            if (event_id < Hooks::INSTANCE_EVENT_COUNT)
            {
                auto key = EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)event_id, entry);
                bindingID = InstanceEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, InstanceEventBindings);
                return 1; // Stack: callback
            }
            break;

      case Hooks::REGTYPE_TICKET:
            if (event_id < Hooks::TICKET_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::TicketEvents>((Hooks::TicketEvents)event_id);
                bindingID = TicketEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, TicketEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_SPELL:
            if (event_id < Hooks::SPELL_EVENT_COUNT)
            {
                if (!sSpellMgr->GetSpellInfo(entry))
                {
                    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
                    luaL_error(L, "Couldn't find a spell with (ID: %d)!", entry);
                    return 0; // Stack: (empty)
                }

                auto key = EntryKey<Hooks::SpellEvents>((Hooks::SpellEvents)event_id, entry);
                bindingID = SpellEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, SpellEventBindings);
                return 1; // Stack: callback
            }
            break;

        case Hooks::REGTYPE_ALL_CREATURE:
            if (event_id < Hooks::ALL_CREATURE_EVENT_COUNT)
            {
                auto key = EventKey<Hooks::AllCreatureEvents>((Hooks::AllCreatureEvents)event_id);
                bindingID = AllCreatureEventBindings->Insert(key, functionRef, shots);
                createCancelCallback(L, bindingID, AllCreatureEventBindings);
                return 1; // Stack: callback
            }
            break;
    }
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    std::ostringstream oss;
    oss << "regtype " << static_cast<uint32>(regtype) << ", event " << event_id << ", entry " << entry << ", guid " << guid.GetRawValue() << ", instance " << instanceId;
    luaL_error(L, "Unknown event type (%s)", oss.str().c_str());
    return 0;
}

/*
 * Cleans up the stack, effectively undoing all Push calls and the Setup call.
 */
void ALE::CleanUpStack(int number_of_arguments)
{
    // Stack: event_id, [arguments]

    lua_pop(L, number_of_arguments + 1); // Add 1 because the caller doesn't know about `event_id`.
    // Stack: (empty)

    if (event_level == 0)
        InvalidateObjects();
}

/*
 * Call a single event handler that was put on the stack with `Setup` and removes it from the stack.
 *
 * The caller is responsible for keeping track of how many times this should be called.
 */
int ALE::CallOneFunction(int number_of_functions, int number_of_arguments, int number_of_results)
{
    ++number_of_arguments; // Caller doesn't know about `event_id`.
    ASSERT(number_of_functions > 0 && number_of_arguments > 0 && number_of_results >= 0);
    // Stack: event_id, [arguments], [functions]

    int functions_top        = lua_gettop(L);
    int first_function_index = functions_top - number_of_functions + 1;
    int arguments_top        = first_function_index - 1;
    int first_argument_index = arguments_top - number_of_arguments + 1;

    // Copy the arguments from the bottom of the stack to the top.
    for (int argument_index = first_argument_index; argument_index <= arguments_top; ++argument_index)
    {
        lua_pushvalue(L, argument_index);
    }
    // Stack: event_id, [arguments], [functions], event_id, [arguments]

    ExecuteCall(number_of_arguments, number_of_results);
    --functions_top;
    // Stack: event_id, [arguments], [functions - 1], [results]

    return functions_top + 1; // Return the location of the first result (if any exist).
}

CreatureAI* ALE::GetAI(Creature* creature)
{
    if (!ALEConfig::GetInstance().IsALEEnabled())
        return NULL;

    for (int i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
    {
        Hooks::CreatureEvents event_id = (Hooks::CreatureEvents)i;

        auto entryKey = EntryKey<Hooks::CreatureEvents>(event_id, creature->GetEntry());
        auto uniqueKey = UniqueObjectKey<Hooks::CreatureEvents>(event_id, creature->GET_GUID(), creature->GetInstanceId());

        if (CreatureEventBindings->HasBindingsFor(entryKey) ||
            CreatureUniqueBindings->HasBindingsFor(uniqueKey))
            return new ALECreatureAI(creature);
    }

    return NULL;
}

InstanceData* ALE::GetInstanceData(Map* map)
{
    if (!ALEConfig::GetInstance().IsALEEnabled())
        return NULL;

    for (int i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
    {
        Hooks::InstanceEvents event_id = (Hooks::InstanceEvents)i;

        auto key = EntryKey<Hooks::InstanceEvents>(event_id, map->GetId());

        if (MapEventBindings->HasBindingsFor(key) ||
            InstanceEventBindings->HasBindingsFor(key))
            return new ALEInstanceAI(map);
    }

    return NULL;
}

bool ALE::HasInstanceData(Map const* map)
{
    if (!map->Instanceable())
        return continentDataRefs.find(map->GetId()) != continentDataRefs.end();
    else
        return instanceDataRefs.find(map->GetInstanceId()) != instanceDataRefs.end();
}

void ALE::CreateInstanceData(Map const* map)
{
    ASSERT(lua_istable(L, -1));
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (!map->Instanceable())
    {
        uint32 mapId = map->GetId();

        // If there's another table that was already stored for the map, unref it.
        auto mapRef = continentDataRefs.find(mapId);
        if (mapRef != continentDataRefs.end())
        {
            luaL_unref(L, LUA_REGISTRYINDEX, mapRef->second);
        }

        continentDataRefs[mapId] = ref;
    }
    else
    {
        uint32 instanceId = map->GetInstanceId();

        // If there's another table that was already stored for the instance, unref it.
        auto instRef = instanceDataRefs.find(instanceId);
        if (instRef != instanceDataRefs.end())
        {
            luaL_unref(L, LUA_REGISTRYINDEX, instRef->second);
        }

        instanceDataRefs[instanceId] = ref;
    }
}

/*
 * Unrefs the instanceId related events and data
 * Does all required actions for when an instance is freed.
 */
void ALE::FreeInstanceId(uint32 instanceId)
{
    LOCK_ALE;

    if (!ALEConfig::GetInstance().IsALEEnabled())
        return;

    for (int i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
    {
        auto key = EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)i, instanceId);

        if (MapEventBindings->HasBindingsFor(key))
            MapEventBindings->Clear(key);

        if (InstanceEventBindings->HasBindingsFor(key))
            InstanceEventBindings->Clear(key);

        if (instanceDataRefs.find(instanceId) != instanceDataRefs.end())
        {
            luaL_unref(L, LUA_REGISTRYINDEX, instanceDataRefs[instanceId]);
            instanceDataRefs.erase(instanceId);
        }
    }
}

void ALE::PushInstanceData(lua_State* L, ALEInstanceAI* ai, bool incrementCounter)
{
    // Check if the instance data is missing (i.e. someone reloaded ALE).
    if (!HasInstanceData(ai->instance))
        ai->Reload();

    // Get the instance data table from the registry.
    if (!ai->instance->Instanceable())
        lua_rawgeti(L, LUA_REGISTRYINDEX, continentDataRefs[ai->instance->GetId()]);
    else
        lua_rawgeti(L, LUA_REGISTRYINDEX, instanceDataRefs[ai->instance->GetInstanceId()]);

    ASSERT(lua_istable(L, -1));

    if (incrementCounter)
        ++push_counter;
}
