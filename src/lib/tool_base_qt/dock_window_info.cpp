#include "dock_window_info.hpp"
#include <QMainWindow>
#include <json_helpers.hpp>
#include <runtime_modules/application_settings.hpp>
#include <window_provider.hpp>

namespace MoonGlare::Tools {

void to_json(nlohmann::json &j, const BaseDockWindowModuleRuntimeData &p) {
    j = {
        {"visible", p.visible},
    };
}

void from_json(const nlohmann::json &j, BaseDockWindowModuleRuntimeData &p) {
    j.at("visible").get_to(p.visible);
}

//-----------------------------------------

BaseDockWindowModule::BaseDockWindowModule(SharedModuleManager modmgr)
    : iModule(std::move(modmgr)) {
}

BaseDockWindowModule::~BaseDockWindowModule() {
}

std::shared_ptr<DockWindow> BaseDockWindowModule::GetInstance() {
    if (!instance) {
        instance = CreateInstance();
        if (!instance)
            return nullptr;

        GetModuleManager()
            ->QueryModule<RuntineModules::ApplicationSettings>()
            ->RegisterSettingsProvider(instance);

        instance->RequestSettingsReload();

        if (parent) {
            parent->addDockWidget(Qt::AllDockWidgetAreas, instance.get());
        }

        connect(instance.get(), SIGNAL(WindowClosed(DockWindow *)),
                SLOT(WindowClosed(DockWindow *)));
    }

    return instance;
}

bool BaseDockWindowModule::Finalize() {
    ReleaseInstance();
    instance.reset();
    return true;
}

void BaseDockWindowModule::ReleaseInstance() {
    if (!instance)
        return;

    if (parent) {
        parent->removeDockWidget(instance.get());
    }

    instance->RequestSettingsSave();
    // instance.reset();
}

void BaseDockWindowModule::WindowClosed(DockWindow *Sender) {
    runtime_data.visible = false;
    ReleaseInstance();
}

void BaseDockWindowModule::Show() {
    runtime_data.visible = true;
    GetInstance();
    if (instance) {
        instance->show();
    }
}

void BaseDockWindowModule::DoSaveSettings(nlohmann::json &json) const {
    json["BaseDockWindowModuleRuntimeData"] = runtime_data;
}

void BaseDockWindowModule::DoLoadSettings(const nlohmann::json &json) {
    try_get_json_child(json, "BaseDockWindowModuleRuntimeData", runtime_data);

    if (runtime_data.visible) {
        Show();
    }
}

} // namespace MoonGlare::Tools
