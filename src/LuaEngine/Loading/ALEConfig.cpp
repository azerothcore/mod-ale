#include "ALEConfig.h"

ALEConfig& ALEConfig::GetInstance()
{
    static ALEConfig instance;
    return instance;
}

ALEConfig::ALEConfig() : ConfigValueCache<ALEConfigValues>(ALEConfigValues::CONFIG_VALUE_COUNT)
{
}

void ALEConfig::Initialize(bool reload)
{
    ConfigValueCache<ALEConfigValues>::Initialize(reload);
}

void ALEConfig::BuildConfigCache()
{
    SetConfigValue<bool>(ALEConfigValues::ENABLED,                    "ALE.Enabled",            "true");
    SetConfigValue<bool>(ALEConfigValues::AUTORELOAD_ENABLED,         "ALE.AutoReload",         "false");
    SetConfigValue<bool>(ALEConfigValues::BYTECODE_CACHE_ENABLED,     "ALE.BytecodeCache",      "true");
    SetConfigValue<bool>(ALEConfigValues::PLAYER_ANNOUNCE_RELOAD,     "ALE.PlayerAnnounceReload", "false");

    SetConfigValue<std::string>(ALEConfigValues::SCRIPT_PATH,         "ALE.ScriptPath",         "lua_scripts");
    SetConfigValue<std::string>(ALEConfigValues::REQUIRE_PATH,        "ALE.RequirePaths",       "");
    SetConfigValue<std::string>(ALEConfigValues::REQUIRE_CPATH,       "ALE.RequireCPaths",      "");
    SetConfigValue<std::string>(ALEConfigValues::MIGRATION_PATH,      "ALE.MigrationPath",      "lua_scripts/migrations");
    SetConfigValue<std::string>(ALEConfigValues::DATABASE_INFO,       "ALE.DatabaseInfo",        "127.0.0.1;3306;acore;acore;acore_ale");

    SetConfigValue<uint32>(ALEConfigValues::AUTORELOAD_INTERVAL,      "ALE.AutoReloadInterval", 1);
    SetConfigValue<uint32>(ALEConfigValues::DATABASE_WORKER_THREADS,  "ALE.DatabaseWorkerThreads", 1);
    SetConfigValue<uint32>(ALEConfigValues::DATABASE_SYNCH_THREADS,   "ALE.DatabaseSynchThreads",  1);
}