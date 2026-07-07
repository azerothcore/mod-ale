/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "LuaEngine.h"
#include "ALECreatureAI.h"
#include "ALEEventMgr.h"
#include "ALEInstanceAI.h"
#include "ALEScriptCache.h"
#include "Chat.h"
#include "GameEventMgr.h"
#include "GuildMgr.h"
#include "Log.h"
#include "MapMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "SpellMgr.h"
#include "StringFormat.h"
#include "WorldSessionMgr.h"

#include <boost/filesystem.hpp>

// Registers all usertypes and global functions into the state (LuaFunctions.cpp).
extern void RegisterFunctions(ALE* E);

ALE* ALE::GALE = nullptr;

ALE::ScriptList ALE::lua_scripts;
ALE::ScriptList ALE::lua_extensions;
std::string ALE::lua_folderpath;
std::string ALE::lua_requirepath;
std::string ALE::lua_requirecpath;
bool ALE::reload = false;
bool ALE::initialized = false;
ALE::LockType ALE::lock;
std::unique_ptr<ALEFileWatcher> ALE::fileWatcher;

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
        uint32 watchInterval = sConfigMgr->GetOption<uint32>("ALE.AutoReloadInterval", 1);
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
    GALE = nullptr;

    lua_scripts.clear();
    lua_extensions.clear();

    ALEScriptCache::ClearCache();

    initialized = false;
}

void ALE::LoadScriptPaths()
{
    uint32 oldMSTime = ALEUtil::GetCurrTime();

    lua_scripts.clear();
    lua_extensions.clear();

    lua_folderpath = ALEConfig::GetInstance().GetScriptPath();
    std::string lua_path_extra(ALEConfig::GetInstance().GetRequirePath());
    std::string lua_cpath_extra(ALEConfig::GetInstance().GetRequireCPath());

#ifndef ALE_WINDOWS
    if (!lua_folderpath.empty() && lua_folderpath[0] == '~')
        if (char const* home = getenv("HOME"))
            lua_folderpath.replace(0, 1, home);
#endif
    ALE_LOG_INFO("[ALE]: Searching scripts from `{}`", lua_folderpath);

    lua_requirepath.clear();
    lua_requirecpath.clear();

    GetScripts(lua_folderpath);

    // append custom require paths and cpaths if the config variables are not empty
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

    if (sConfigMgr->GetOption<bool>("ALE.PlayerAnnounceReload", false))
        sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, "Reloading ALE...");
    else
        ChatHandler(nullptr).SendGMText(SERVER_MSG_STRING, "Reloading ALE...");

    // Remove all timed events
    sALE->eventMgr->SetStates(LUAEVENT_STATE_ERASE);

    sALE->CloseLua();

    LoadScriptPaths();

    sALE->OpenLua();
    sALE->RunScripts();

    reload = false;
}

ALE::ALE() :
    eventMgr(nullptr)
{
    ASSERT(IsInitialized());

    OpenLua();

    // Must be after OpenLua()
    eventMgr = new EventMgr(&ALE::GALE);
}

ALE::~ALE()
{
    ASSERT(IsInitialized());

    CloseLua();

    delete eventMgr;
    eventMgr = nullptr;
}

void ALE::CloseLua()
{
    if (!stateOpened)
        return;

    OnLuaStateClose();

    // Everything referencing the Lua state must be released before it is
    // replaced: handlers, instance data tables, ...
    DestroyBindStores();
    instanceDataRefs.clear();
    continentDataRefs.clear();

    lua = sol::state();
    stateOpened = false;
}

void ALE::OpenLua()
{
    if (!ALEConfig::GetInstance().IsALEEnabled())
    {
        ALE_LOG_INFO("[ALE]: ALE is disabled in config");
        return;
    }

    lua = sol::state();
    lua.open_libraries();
    stateOpened = true;

    // Append a Lua stack trace to every handler error when enabled
    if (ALEConfig::GetInstance().IsTraceBackEnabled())
        sol::protected_function::set_default_handler(lua["debug"]["traceback"]);

    CreateBindStores();

    // Register usertypes and global functions
    RegisterFunctions(this);

    // Set lua require folder paths (scripts folder structure)
    lua["package"]["path"] = GetRequirePath();
    lua["package"]["cpath"] = GetRequireCPath();
}

void ALE::CreateBindStores()
{
    DestroyBindStores();

    ServerEventBindings      = std::make_unique<BindingMap<EventKey<Hooks::ServerEvents>>>();
    PlayerEventBindings      = std::make_unique<BindingMap<EventKey<Hooks::PlayerEvents>>>();
    GuildEventBindings       = std::make_unique<BindingMap<EventKey<Hooks::GuildEvents>>>();
    GroupEventBindings       = std::make_unique<BindingMap<EventKey<Hooks::GroupEvents>>>();
    VehicleEventBindings     = std::make_unique<BindingMap<EventKey<Hooks::VehicleEvents>>>();
    BGEventBindings          = std::make_unique<BindingMap<EventKey<Hooks::BGEvents>>>();
    TicketEventBindings      = std::make_unique<BindingMap<EventKey<Hooks::TicketEvents>>>();
    AllCreatureEventBindings = std::make_unique<BindingMap<EventKey<Hooks::AllCreatureEvents>>>();

    PacketEventBindings      = std::make_unique<BindingMap<EntryKey<Hooks::PacketEvents>>>();
    CreatureEventBindings    = std::make_unique<BindingMap<EntryKey<Hooks::CreatureEvents>>>();
    CreatureGossipBindings   = std::make_unique<BindingMap<EntryKey<Hooks::GossipEvents>>>();
    GameObjectEventBindings  = std::make_unique<BindingMap<EntryKey<Hooks::GameObjectEvents>>>();
    GameObjectGossipBindings = std::make_unique<BindingMap<EntryKey<Hooks::GossipEvents>>>();
    ItemEventBindings        = std::make_unique<BindingMap<EntryKey<Hooks::ItemEvents>>>();
    ItemGossipBindings       = std::make_unique<BindingMap<EntryKey<Hooks::GossipEvents>>>();
    PlayerGossipBindings     = std::make_unique<BindingMap<EntryKey<Hooks::GossipEvents>>>();
    MapEventBindings         = std::make_unique<BindingMap<EntryKey<Hooks::InstanceEvents>>>();
    InstanceEventBindings    = std::make_unique<BindingMap<EntryKey<Hooks::InstanceEvents>>>();
    SpellEventBindings       = std::make_unique<BindingMap<EntryKey<Hooks::SpellEvents>>>();

    CreatureUniqueBindings   = std::make_unique<BindingMap<UniqueObjectKey<Hooks::CreatureEvents>>>();
}

void ALE::DestroyBindStores()
{
    ServerEventBindings.reset();
    PlayerEventBindings.reset();
    GuildEventBindings.reset();
    GroupEventBindings.reset();
    VehicleEventBindings.reset();
    BGEventBindings.reset();
    TicketEventBindings.reset();
    AllCreatureEventBindings.reset();

    PacketEventBindings.reset();
    CreatureEventBindings.reset();
    CreatureGossipBindings.reset();
    GameObjectEventBindings.reset();
    GameObjectGossipBindings.reset();
    ItemEventBindings.reset();
    ItemGossipBindings.reset();
    PlayerGossipBindings.reset();
    MapEventBindings.reset();
    InstanceEventBindings.reset();
    SpellEventBindings.reset();

    CreatureUniqueBindings.reset();
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
    if (ext != ".lua" && ext != ".dll" && ext != ".so" && ext != ".ext" && ext != ".moon" && ext != ".out")
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

// Finds lua script files from given path (including subdirectories) and pushes them to scripts
void ALE::GetScripts(std::string path)
{
    ALE_LOG_DEBUG("[ALE]: GetScripts from path `{}`", path);

    boost::filesystem::path someDir(path);
    boost::filesystem::directory_iterator end_iter;

    if (!boost::filesystem::exists(someDir) || !boost::filesystem::is_directory(someDir))
        return;

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
        std::string name = dir_iter->path().filename().generic_string();
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

static bool ScriptPathComparator(const LuaScript& first, const LuaScript& second)
{
    return first.filepath < second.filepath;
}

void ALE::RunScripts()
{
    LOCK_ALE;
    if (!stateOpened)
        return;

    uint32 oldMSTime = ALEUtil::GetCurrTime();
    uint32 count = 0;
    uint32 compiledCount = 0;
    uint32 cachedCount = 0;
    uint32 precompiledCount = 0;
    bool cacheEnabled = ALEConfig::GetInstance().IsByteCodeCacheEnabled();

    if (cacheEnabled)
        ALEScriptCache::ClearTimestamps();

    ScriptList scripts;
    lua_extensions.sort(ScriptPathComparator);
    lua_scripts.sort(ScriptPathComparator);
    scripts.insert(scripts.end(), lua_extensions.begin(), lua_extensions.end());
    scripts.insert(scripts.end(), lua_scripts.begin(), lua_scripts.end());

    std::unordered_map<std::string, std::string> loaded; // filename, path

    sol::table packageLoaded = lua["package"]["loaded"];

    for (LuaScript const& script : scripts)
    {
        // Check that no duplicate names exist
        if (loaded.find(script.filename) != loaded.end())
        {
            ALE_LOG_ERROR("[ALE]: Error loading `{}`. File with same name already loaded from `{}`, rename either file", script.filepath, loaded[script.filename]);
            continue;
        }
        loaded[script.filename] = script.filepath;

        // Skip anything require() already pulled in
        if (packageLoaded[script.filename].valid() && packageLoaded[script.filename] != sol::nil)
        {
            ALE_LOG_DEBUG("[ALE]: `{}` was already loaded or required", script.filepath);
            continue;
        }

        // Load the file as a callable chunk
        sol::protected_function chunk;
        if (script.fileext == ".out")
        {
            chunk = ALEScriptCache::LoadCompiled(lua, script.filepath);
            if (chunk.valid())
                ++precompiledCount;
        }
        else
            chunk = ALEScriptCache::Load(lua, script.filepath, script.fileext == ".moon", compiledCount, cachedCount);

        if (!chunk.valid())
            continue;

        // Run it and remember its result the way require() would
        // (the guard covers the rest of the iteration, which is harmless).
        DispatchGuard guard(this);
        sol::protected_function_result result = chunk();

        if (!result.valid())
        {
            Report(sol::error(result));
            continue;
        }

        sol::object value = result.get<sol::object>(0);
        if (!value.valid() || value == sol::nil || (value.is<bool>() && !value.as<bool>()))
            value = sol::make_object(lua, true);
        packageLoaded[script.filename] = value;

        ALE_LOG_DEBUG("[ALE]: Successfully loaded `{}`", script.filepath);
        ++count;
    }

    std::string details;
    if (cacheEnabled && (compiledCount > 0 || cachedCount > 0 || precompiledCount > 0))
        details = fmt::format("({} compiled, {} cached, {} pre-compiled)", compiledCount, cachedCount, precompiledCount);

    ALE_LOG_INFO("[ALE]: Executed {} Lua scripts in {} ms {}", count, ALEUtil::GetTimeDiff(oldMSTime), details);

    OnLuaStateOpen();
}

void ALE::Report(sol::error const& error)
{
    ALE_LOG_ERROR("{}", error.what());
    OnError(std::string(error.what()));
}

/*
 * Saves the handler to the register type's store for the given entry/guid
 * under the given event. Returns a callable that cancels the registration.
 */
sol::object ALE::Register(uint8 regtype, uint32 entry, ObjectGuid guid, uint32 instanceId,
    uint32 event_id, sol::protected_function callback, uint32 shots)
{
    // Inserts the handler and builds the Lua-side cancel callable.
    auto bind = [&](auto& bindings, auto key) -> sol::object
    {
        uint64 bindingID = bindings->Insert(key, std::move(callback), shots);
        auto* store = bindings.get();
        return sol::make_object(lua, [store, bindingID]() { store->Remove(bindingID); });
    };

    switch (regtype)
    {
        case Hooks::REGTYPE_SERVER:
            if (event_id < Hooks::SERVER_EVENT_COUNT)
                return bind(ServerEventBindings, EventKey<Hooks::ServerEvents>((Hooks::ServerEvents)event_id));
            break;

        case Hooks::REGTYPE_PLAYER:
            if (event_id < Hooks::PLAYER_EVENT_COUNT)
                return bind(PlayerEventBindings, EventKey<Hooks::PlayerEvents>((Hooks::PlayerEvents)event_id));
            break;

        case Hooks::REGTYPE_GUILD:
            if (event_id < Hooks::GUILD_EVENT_COUNT)
                return bind(GuildEventBindings, EventKey<Hooks::GuildEvents>((Hooks::GuildEvents)event_id));
            break;

        case Hooks::REGTYPE_GROUP:
            if (event_id < Hooks::GROUP_EVENT_COUNT)
                return bind(GroupEventBindings, EventKey<Hooks::GroupEvents>((Hooks::GroupEvents)event_id));
            break;

        case Hooks::REGTYPE_VEHICLE:
            if (event_id < Hooks::VEHICLE_EVENT_COUNT)
                return bind(VehicleEventBindings, EventKey<Hooks::VehicleEvents>((Hooks::VehicleEvents)event_id));
            break;

        case Hooks::REGTYPE_BG:
            if (event_id < Hooks::BG_EVENT_COUNT)
                return bind(BGEventBindings, EventKey<Hooks::BGEvents>((Hooks::BGEvents)event_id));
            break;

        case Hooks::REGTYPE_PACKET:
            if (event_id < Hooks::PACKET_EVENT_COUNT)
            {
                if (entry >= NUM_MSG_TYPES)
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find an opcode with (ID: {})!", entry));

                return bind(PacketEventBindings, EntryKey<Hooks::PacketEvents>((Hooks::PacketEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_CREATURE:
            if (event_id < Hooks::CREATURE_EVENT_COUNT)
            {
                if (entry != 0)
                {
                    if (!sObjectMgr->GetCreatureTemplate(entry))
                        throw std::invalid_argument(Acore::StringFormat("Couldn't find a creature with (ID: {})!", entry));

                    return bind(CreatureEventBindings, EntryKey<Hooks::CreatureEvents>((Hooks::CreatureEvents)event_id, entry));
                }

                if (guid.IsEmpty())
                    throw std::invalid_argument("guid was 0!");

                return bind(CreatureUniqueBindings, UniqueObjectKey<Hooks::CreatureEvents>((Hooks::CreatureEvents)event_id, guid, instanceId));
            }
            break;

        case Hooks::REGTYPE_CREATURE_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!sObjectMgr->GetCreatureTemplate(entry))
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find a creature with (ID: {})!", entry));

                return bind(CreatureGossipBindings, EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_GAMEOBJECT:
            if (event_id < Hooks::GAMEOBJECT_EVENT_COUNT)
            {
                if (!sObjectMgr->GetGameObjectTemplate(entry))
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find a gameobject with (ID: {})!", entry));

                return bind(GameObjectEventBindings, EntryKey<Hooks::GameObjectEvents>((Hooks::GameObjectEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_GAMEOBJECT_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!sObjectMgr->GetGameObjectTemplate(entry))
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find a gameobject with (ID: {})!", entry));

                return bind(GameObjectGossipBindings, EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_ITEM:
            if (event_id < Hooks::ITEM_EVENT_COUNT)
            {
                if (!sObjectMgr->GetItemTemplate(entry))
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find an item with (ID: {})!", entry));

                return bind(ItemEventBindings, EntryKey<Hooks::ItemEvents>((Hooks::ItemEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_ITEM_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
            {
                if (!sObjectMgr->GetItemTemplate(entry))
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find an item with (ID: {})!", entry));

                return bind(ItemGossipBindings, EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_PLAYER_GOSSIP:
            if (event_id < Hooks::GOSSIP_EVENT_COUNT)
                return bind(PlayerGossipBindings, EntryKey<Hooks::GossipEvents>((Hooks::GossipEvents)event_id, entry));
            break;

        case Hooks::REGTYPE_MAP:
            if (event_id < Hooks::INSTANCE_EVENT_COUNT)
                return bind(MapEventBindings, EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)event_id, entry));
            break;

        case Hooks::REGTYPE_INSTANCE:
            if (event_id < Hooks::INSTANCE_EVENT_COUNT)
                return bind(InstanceEventBindings, EntryKey<Hooks::InstanceEvents>((Hooks::InstanceEvents)event_id, entry));
            break;

        case Hooks::REGTYPE_TICKET:
            if (event_id < Hooks::TICKET_EVENT_COUNT)
                return bind(TicketEventBindings, EventKey<Hooks::TicketEvents>((Hooks::TicketEvents)event_id));
            break;

        case Hooks::REGTYPE_SPELL:
            if (event_id < Hooks::SPELL_EVENT_COUNT)
            {
                if (!sSpellMgr->GetSpellInfo(entry))
                    throw std::invalid_argument(Acore::StringFormat("Couldn't find a spell with (ID: {})!", entry));

                return bind(SpellEventBindings, EntryKey<Hooks::SpellEvents>((Hooks::SpellEvents)event_id, entry));
            }
            break;

        case Hooks::REGTYPE_ALL_CREATURE:
            if (event_id < Hooks::ALL_CREATURE_EVENT_COUNT)
                return bind(AllCreatureEventBindings, EventKey<Hooks::AllCreatureEvents>((Hooks::AllCreatureEvents)event_id));
            break;
    }

    throw std::invalid_argument(Acore::StringFormat(
        "Unknown event type (regtype {}, event {}, entry {}, guid {}, instance {})",
        regtype, event_id, entry, guid.GetRawValue(), instanceId));
}

CreatureAI* ALE::GetAI(Creature* creature)
{
    if (!ALEConfig::GetInstance().IsALEEnabled())
        return nullptr;

    for (int i = 1; i < Hooks::CREATURE_EVENT_COUNT; ++i)
    {
        Hooks::CreatureEvents event_id = (Hooks::CreatureEvents)i;

        auto entryKey = EntryKey<Hooks::CreatureEvents>(event_id, creature->GetEntry());
        auto uniqueKey = UniqueObjectKey<Hooks::CreatureEvents>(event_id, creature->GetGUID(), creature->GetInstanceId());

        if (CreatureEventBindings->HasBindingsFor(entryKey) ||
            CreatureUniqueBindings->HasBindingsFor(uniqueKey))
            return new ALECreatureAI(creature);
    }

    return nullptr;
}

InstanceData* ALE::GetInstanceData(Map* map)
{
    if (!ALEConfig::GetInstance().IsALEEnabled())
        return nullptr;

    for (int i = 1; i < Hooks::INSTANCE_EVENT_COUNT; ++i)
    {
        Hooks::InstanceEvents event_id = (Hooks::InstanceEvents)i;

        auto key = EntryKey<Hooks::InstanceEvents>(event_id, map->GetId());

        if (MapEventBindings->HasBindingsFor(key) ||
            InstanceEventBindings->HasBindingsFor(key))
            return new ALEInstanceAI(map);
    }

    return nullptr;
}

bool ALE::HasInstanceData(Map const* map)
{
    if (!map->Instanceable())
        return continentDataRefs.find(map->GetId()) != continentDataRefs.end();

    return instanceDataRefs.find(map->GetInstanceId()) != instanceDataRefs.end();
}

void ALE::CreateInstanceData(Map const* map, sol::table data)
{
    // Overwriting an existing entry releases the previous table automatically.
    if (!map->Instanceable())
        continentDataRefs[map->GetId()] = std::move(data);
    else
        instanceDataRefs[map->GetInstanceId()] = std::move(data);
}

sol::table ALE::GetInstanceData(ALEInstanceAI* ai)
{
    // Check if the instance data is missing (i.e. someone reloaded ALE).
    if (!HasInstanceData(ai->instance))
        ai->Reload();

    if (!ai->instance->Instanceable())
        return continentDataRefs[ai->instance->GetId()];

    return instanceDataRefs[ai->instance->GetInstanceId()];
}

/*
 * Releases the instanceId related events and data.
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

        MapEventBindings->Clear(key);
        InstanceEventBindings->Clear(key);
    }

    instanceDataRefs.erase(instanceId);
}
