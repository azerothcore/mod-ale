#ifndef ALE_CONFIG_HPP
#define ALE_CONFIG_HPP

#include "ConfigValueCache.h"

enum class ALEConfigValues : uint32
{
    // Boolean
    ENABLED = 0,
    TRACEBACK_ENABLED,
    AUTORELOAD_ENABLED,
    BYTECODE_CACHE_ENABLED,

    // String
    SCRIPT_PATH,
    REQUIRE_PATH,
    REQUIRE_CPATH,

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

        std::string_view GetScriptPath() const { return GetConfigValue(ALEConfigValues::SCRIPT_PATH); }
        std::string_view GetRequirePath() const { return GetConfigValue(ALEConfigValues::REQUIRE_PATH); }
        std::string_view GetRequireCPath() const { return GetConfigValue(ALEConfigValues::REQUIRE_CPATH); }

        uint32 GetAutoReloadInterval() const { return GetConfigValue<uint32>(ALEConfigValues::AUTORELOAD_INTERVAL); }

    protected:
        void BuildConfigCache() override;

    private:
        ALEConfig();
        ~ALEConfig() = default;
        ALEConfig(const ALEConfig&) = delete;
        ALEConfig& operator=(const ALEConfig&) = delete;
};

#endif // ALE_CONFIG_H