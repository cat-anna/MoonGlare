#include "application_settings.hpp"
#include "app_config.h"
#include "settings_provider.hpp"
#include <filesystem>
#include <iomanip>

namespace MoonGlare::Tools::RuntineModules {

namespace {} // namespace

ApplicationSettings::ApplicationSettings(SharedModuleManager modmgr) : iModule(modmgr) {
    //
    file_name = "Unknown.settings.json";
}

ApplicationSettings::~ApplicationSettings() {
}

bool ApplicationSettings::Initialize() {
    if (!GetModuleManager())
        return true;

    auto appc = GetModuleManager()->QueryModule<AppConfig>();
    file_name = appc->Get("ConfigPath") + "/" + appc->Get("AppName") + ".json";

    GetModuleManager()->ForEachInterface<iSettingsProvider>([this](auto provider, auto) {
        provider->SetSetingsModule(this);
        settings_providers.emplace_back(std::move(provider));
    });

    return true;
}

bool ApplicationSettings::PostInit() {
    return true;
}

bool ApplicationSettings::Finalize() {
    return true;
}

void ApplicationSettings::RegisterSettingsProvider(std::weak_ptr<iSettingsProvider> provider) {
    auto shared = provider.lock();
    if (shared) {
        shared->SetSetingsModule(this);
        settings_providers.emplace_back(std::move(provider));
        SortProviders();
    }
}

void ApplicationSettings::SaveAndLock() {
    Save();
    locked = true;
}

//-----------------------------------------

void ApplicationSettings::SortProviders() {
    std::sort(settings_providers.begin(), settings_providers.end(), [](auto a, auto b) {
        auto s_a = a.lock();
        if (!s_a) {
            return true;
        }
        auto s_b = b.lock();
        if (!s_b) {
            return false;
        }
        return s_a->GetLoadOrder() < s_b->GetLoadOrder();
    });
}

void ApplicationSettings::Load() {
    settings_json = std::make_unique<nlohmann::json>();
    if (std::filesystem::is_regular_file(file_name)) {
        std::ifstream input_file(file_name);
        input_file >> *settings_json;
    }

    // auto customRoot = CustomSettingsRoot();
    // for (auto mod : GetModuleManager()->QueryInterfaces<CustomConfigSet>()) {
    //     auto name = mod.m_Interface->GetName();
    //     auto node = customRoot.child(name.c_str());
    //     mod.m_Interface->LoadSettings(node);
    // }

    SortProviders();

    decltype(settings_providers) providers_copy = settings_providers;
    for (auto &item : providers_copy) {
        auto shared = item.lock();
        if (shared) {
            shared->RequestSettingsReload();
        }
    }
}

void ApplicationSettings::Save() {
    if (locked) {
        return;
    }
    // {
    //     auto root = CustomSettingsRoot();
    //     root.parent().remove_child(root);
    // }

    // auto customRoot = CustomSettingsRoot();
    // for (auto mod : GetModuleManager()->QueryInterfaces<CustomConfigSet>()) {
    //     auto name = mod.m_Interface->GetName();
    //     auto node = customRoot.child(name.c_str());
    //     if (!node)
    //         node = customRoot.append_child(name.c_str());
    //     mod.m_Interface->SaveSettings(node);
    // }

    GetModuleManager()->ForEachInterface<iSettingsProvider>(
        [this](auto provider, auto) { provider->RequestSettingsSave(); });

    for (auto &item : settings_providers) {
        auto shared = item.lock();
        if (shared) {
            shared->RequestSettingsSave();
        }
    }

    std::ofstream output_file(file_name);
    output_file << std::setw(4) << *settings_json;
}

void ApplicationSettings::SaveSettings(iSettingsProvider *user) {
    auto &group = user->GetSettingId();
    if (group.empty())
        return;

    auto &root = GetSettingsProviderRoot();
    if (!root.contains(group)) {
        root[group] = {};
    }
    user->DoSaveSettings(root[group]);
}

void ApplicationSettings::LoadSettings(iSettingsProvider *user) {
    auto &group = user->GetSettingId();
    if (group.empty())
        return;
    auto &root = GetSettingsProviderRoot();
    if (!root.contains(group))
        return;
    user->DoLoadSettings(root[group]);
}

nlohmann::json &ApplicationSettings::GetSettingsProviderRoot() {
    if (settings_json->contains("settings_provider")) {
        return (*settings_json)["settings_provider"];
    }
    (*settings_json)["settings_provider"] = {};
    return GetSettingsProviderRoot();
}

// pugi::xml_node ApplicationSettings::CustomSettingsRoot() {
//     auto root = m_SettingsDoc->document_element();
//     if (!root) {
//         root = m_SettingsDoc->append_child("Settings");
//     }
//     auto Static = root.child("Custom");
//     if (!Static) {
//         Static = root.append_child("Custom");
//     }
//     return Static;
// }

// pugi::xml_node ApplicationSettings::DynamicSettingsRoot() {
//     auto root = m_SettingsDoc->document_element();
//     if (!root) {
//         root = m_SettingsDoc->append_child("Settings");
//     }
//     auto dynamic = root.child("Dynamic");
//     if (!dynamic) {
//         dynamic = root.append_child("Dynamic");
//     }
//     return dynamic;
// }

} // namespace MoonGlare::Tools::RuntineModules
