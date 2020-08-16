#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class iSettingsProvider;

class ApplicationSettings : public iModule {
public:
    ApplicationSettings(SharedModuleManager modmgr);
    virtual ~ApplicationSettings() override;

    bool Initialize() override;
    bool PostInit() override;
    bool Finalize() override;

    void Save();
    void Load();

    void SaveSettings(iSettingsProvider *user);
    void LoadSettings(iSettingsProvider *user);

    void RegisterSettingsProvider(std::weak_ptr<iSettingsProvider> provider);

    void SaveAndLock();

protected:
    std::unique_ptr<nlohmann::json> settings_json;
    std::string file_name;

    std::vector<std::weak_ptr<iSettingsProvider>> settings_providers;
    void SortProviders();

    bool locked = false;

    nlohmann::json &GetSettingsProviderRoot();
};

} // namespace MoonGlare::Tools::RuntineModules
