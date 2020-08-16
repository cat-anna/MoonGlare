#pragma once

#include <QWidget>
#include <nlohmann/json.hpp>
#include <runtime_modules/settings_provider.hpp>

namespace MoonGlare::Tools::RuntineModules {

class ApplicationSettings;

class iWidgetSettingsProvider : public iSettingsProvider {

public:
    virtual ~iWidgetSettingsProvider() {}

protected:
    void DoSaveSettings(nlohmann::json &json) const override;
    void DoLoadSettings(const nlohmann::json &json) override;

    virtual std::unordered_map<std::string, QWidget *> GetStateSavableWidgets() const {
        return {};
    };
};

} // namespace MoonGlare::Tools::RuntineModules
