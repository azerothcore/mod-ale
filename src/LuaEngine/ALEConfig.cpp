#include "ALEConfig.h"
#include "ALEUtility.h"
#include <sstream>
#include <algorithm>
#include <cctype>

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
    TokenizeAllowedMaps();
}

void ALEConfig::BuildConfigCache()
{
    SetConfigValue<bool>(ALEConfigValues::ENABLED,                    "ALE.Enabled",            "false");
    SetConfigValue<bool>(ALEConfigValues::TRACEBACK_ENABLED,          "ALE.TraceBack",          "false");
    SetConfigValue<bool>(ALEConfigValues::AUTORELOAD_ENABLED,         "ALE.AutoReload",         "false");
    SetConfigValue<bool>(ALEConfigValues::BYTECODE_CACHE_ENABLED,     "ALE.BytecodeCache",      "false");
    SetConfigValue<bool>(ALEConfigValues::COMPATIBILITY_MODE,         "ALE.CompatibilityMode",  "true");

    SetConfigValue<std::string>(ALEConfigValues::SCRIPT_PATH,         "ALE.ScriptPath",         "lua_scripts");
    SetConfigValue<std::string>(ALEConfigValues::REQUIRE_PATH,        "ALE.RequirePaths",       "");
    SetConfigValue<std::string>(ALEConfigValues::REQUIRE_CPATH,       "ALE.RequireCPaths",      "");
    SetConfigValue<std::string>(ALEConfigValues::ONLY_ON_MAPS,        "ALE.OnlyOnMaps",         "");

    SetConfigValue<uint32>(ALEConfigValues::AUTORELOAD_INTERVAL,      "ALE.AutoReloadInterval", 1);
}

bool ALEConfig::ShouldMapLoadALE(uint32 mapId) const
{
    if (m_allowedMaps.empty())
        return true;
    return m_allowedMaps.find(mapId) != m_allowedMaps.end();
}

bool ALEConfig::ShouldMapLoadALEByFolderName(const std::string& folderName, uint32 mapId) const
{
    std::string digits;
    for (char c : folderName)
    {
        if (std::isdigit(static_cast<unsigned char>(c)))
            digits += c;
        else
            break;
    }

    if (digits.empty())
        return true;

    try
    {
        uint32 folderMapId = std::stoul(digits);
        return folderMapId == mapId;
    }
    catch (std::exception&)
    {
        return true;
    }
}

void ALEConfig::TokenizeAllowedMaps()
{
    m_allowedMaps.clear();

    std::istringstream maps(static_cast<std::string>(GetConfigValue(ALEConfigValues::ONLY_ON_MAPS)));
    std::string mapIdStr;
    while (std::getline(maps, mapIdStr, ','))
    {
        mapIdStr.erase(std::remove_if(mapIdStr.begin(), mapIdStr.end(), [](char c) {
            return std::isspace(static_cast<unsigned char>(c));
        }), mapIdStr.end());

        if (mapIdStr.empty())
            continue;

        try
        {
            uint32 mapId = std::stoul(mapIdStr);
            m_allowedMaps.emplace(mapId);
        }
        catch (std::exception&)
        {
            ALE_LOG_ERROR("[ALEConfig]: Invalid map ID in ALE.OnlyOnMaps: '{}'", mapIdStr);
        }
    }
}
