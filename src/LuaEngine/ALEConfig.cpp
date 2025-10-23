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
    SetConfigValue<bool>(ALEConfigValues::ENABLED,                    "ALE.Enabled",            "false");
    SetConfigValue<bool>(ALEConfigValues::TRACEBACK_ENABLED,          "ALE.TraceBack",          "false");
    SetConfigValue<bool>(ALEConfigValues::AUTORELOAD_ENABLED,         "ALE.AutoReload",         "false");
    SetConfigValue<bool>(ALEConfigValues::BYTECODE_CACHE_ENABLED,     "ALE.BytecodeCache",      "false");

    SetConfigValue<std::string>(ALEConfigValues::SCRIPT_PATH,         "ALE.ScriptPath",         "lua_scripts");
    SetConfigValue<std::string>(ALEConfigValues::REQUIRE_PATH,        "ALE.RequirePaths",       "");
    SetConfigValue<std::string>(ALEConfigValues::REQUIRE_CPATH,       "ALE.RequireCPaths",      "");

    SetConfigValue<uint32>(ALEConfigValues::AUTORELOAD_INTERVAL,      "ALE.AutoReloadInterval", 1);
}
