#include "settings_provider.hpp"
#include "application_settings.hpp"

namespace MoonGlare::Tools::RuntineModules {

void iSettingsProvider::DoSaveSettings(nlohmann::json &json) const {
}

void iSettingsProvider::DoLoadSettings(const nlohmann::json &json) {
}

//-----------------------------------------

void iSettingsProvider::RequestSettingsSave() {
    if (app_settings != nullptr) {
        app_settings->SaveSettings(this);
    }
}

void iSettingsProvider::RequestSettingsReload() {
    if (app_settings != nullptr) {
        app_settings->LoadSettings(this);
    }
}

} // namespace MoonGlare::Tools::RuntineModules
