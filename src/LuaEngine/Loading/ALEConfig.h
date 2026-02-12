#ifndef ALE_CONFIG_HPP
#define ALE_CONFIG_HPP

#include "ConfigValueCache.h"

enum class ALEConfigValues : uint32
{
    // Boolean
    ENABLED = 0,
    AUTORELOAD_ENABLED,
    BYTECODE_CACHE_ENABLED,
    PLAYER_ANNOUNCE_RELOAD,

    // String
    SCRIPT_PATH,
    REQUIRE_PATH,
    REQUIRE_CPATH,
    MIGRATION_PATH,
    DATABASE_INFO,

    // Number
    AUTORELOAD_INTERVAL,
    DATABASE_WORKER_THREADS,
    DATABASE_SYNCH_THREADS,

    CONFIG_VALUE_COUNT
};

class ALEConfig final : public ConfigValueCache<ALEConfigValues>
{
    public:
        static ALEConfig& GetInstance();

        void Initialize(bool reload = false);

        bool IsALEEnabled() const { return GetConfigValue<bool>(ALEConfigValues::ENABLED); }
        bool IsAutoReloadEnabled() const { return GetConfigValue<bool>(ALEConfigValues::AUTORELOAD_ENABLED); }
        bool IsByteCodeCacheEnabled() const { return GetConfigValue<bool>(ALEConfigValues::BYTECODE_CACHE_ENABLED); }
        bool IsPlayerAnnounceReloadEnabled() const { return GetConfigValue<bool>(ALEConfigValues::PLAYER_ANNOUNCE_RELOAD); }

        std::string_view GetScriptPath() const { return GetConfigValue(ALEConfigValues::SCRIPT_PATH); }
        std::string_view GetRequirePath() const { return GetConfigValue(ALEConfigValues::REQUIRE_PATH); }
        std::string_view GetRequireCPath() const { return GetConfigValue(ALEConfigValues::REQUIRE_CPATH); }
        std::string_view GetMigrationPath() const { return GetConfigValue(ALEConfigValues::MIGRATION_PATH); }
        std::string_view GetDatabaseInfo() const { return GetConfigValue(ALEConfigValues::DATABASE_INFO); }

        uint32 GetAutoReloadInterval() const { return GetConfigValue<uint32>(ALEConfigValues::AUTORELOAD_INTERVAL); }
        uint32 GetDatabaseWorkerThreads() const { return GetConfigValue<uint32>(ALEConfigValues::DATABASE_WORKER_THREADS); }
        uint32 GetDatabaseSynchThreads() const { return GetConfigValue<uint32>(ALEConfigValues::DATABASE_SYNCH_THREADS); }

    protected:
        void BuildConfigCache() override;

    private:
        ALEConfig();
        ~ALEConfig() = default;
        ALEConfig(const ALEConfig&) = delete;
        ALEConfig& operator=(const ALEConfig&) = delete;
};

#endif // ALE_CONFIG_H