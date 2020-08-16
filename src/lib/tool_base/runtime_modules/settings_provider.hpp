#pragma once

#include <nlohmann/json.hpp>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class ApplicationSettings;

class iSettingsProvider {
    friend class ApplicationSettings;

public:
    virtual ~iSettingsProvider() {}

    void SetSetingsModule(ApplicationSettings *module) { app_settings = module; }
    void RequestSettingsReload();
    void RequestSettingsSave();

    const std::string &GetSettingId() const { return settings_id; }
    void SetSettingId(std::string v, int load_order = 0) {
        settings_id.swap(v);
        this->load_order = load_order;
    }

    int GetLoadOrder() const { return load_order; }

protected:
    virtual void DoSaveSettings(nlohmann::json &json) const;
    virtual void DoLoadSettings(const nlohmann::json &json);

private:
    std::string settings_id;
    ApplicationSettings *app_settings = nullptr;
    int load_order = 0;
};

} // namespace MoonGlare::Tools::RuntineModules
