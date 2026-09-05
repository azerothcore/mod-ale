#ifndef ALE_CONFIG_HPP
#define ALE_CONFIG_HPP

#include "ConfigValueCache.h"
#include <unordered_set>

enum class ALEConfigValues : uint32
{
    // Boolean
    ENABLED = 0,
    TRACEBACK_ENABLED,
    AUTORELOAD_ENABLED,
    BYTECODE_CACHE_ENABLED,
    COMPATIBILITY_MODE,

    // String
    SCRIPT_PATH,
    REQUIRE_PATH,
    REQUIRE_CPATH,
    ONLY_ON_MAPS,

    // Number
    AUTORELOAD_INTERVAL,

    CONFIG_VALUE_COUNT
};

class ALEConfig final : public ConfigValueCache<ALEConfigValues>
{
    public:
        static ALEConfig& GetInstance();

        void Initialize(bool reload = false);

        bool IsALEEnabled() const { return GetConfigValue<bool>(ALEConfigValues::ENABLED); }
        bool IsTraceBackEnabled() const { return GetConfigValue<bool>(ALEConfigValues::TRACEBACK_ENABLED); }
        bool IsAutoReloadEnabled() const { return GetConfigValue<bool>(ALEConfigValues::AUTORELOAD_ENABLED); }
        bool IsByteCodeCacheEnabled() const { return GetConfigValue<bool>(ALEConfigValues::BYTECODE_CACHE_ENABLED); }
        bool IsCompatibilityModeEnabled() const { return GetConfigValue<bool>(ALEConfigValues::COMPATIBILITY_MODE); }

        std::string_view GetScriptPath() const { return GetConfigValue(ALEConfigValues::SCRIPT_PATH); }
        std::string_view GetRequirePath() const { return GetConfigValue(ALEConfigValues::REQUIRE_PATH); }
        std::string_view GetRequireCPath() const { return GetConfigValue(ALEConfigValues::REQUIRE_CPATH); }

        uint32 GetAutoReloadInterval() const { return GetConfigValue<uint32>(ALEConfigValues::AUTORELOAD_INTERVAL); }

        bool ShouldMapLoadALE(uint32 mapId) const;
        bool ShouldMapLoadALEByFolderName(const std::string& folderName, uint32 mapId) const;

    protected:
        void BuildConfigCache() override;

    private:
        ALEConfig();
        ~ALEConfig() = default;
        ALEConfig(const ALEConfig&) = delete;
        ALEConfig& operator=(const ALEConfig&) = delete;

        void TokenizeAllowedMaps();

        std::unordered_set<uint32> m_allowedMaps;
};

#endif // ALE_CONFIG_H