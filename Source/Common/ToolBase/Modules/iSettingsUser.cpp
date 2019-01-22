#include "iSettingsUser.h"
#include "AppConfig.h"

namespace MoonGlare {

static iSettings *l_iSettingsInstance = nullptr;

iSettings::iSettings(SharedModuleManager modmgr) : iModule(modmgr) {
    l_iSettingsInstance = this;
    m_FileName = "Unknown.Settings.xml";
}

iSettings::~iSettings() {
}

bool iSettings::Initialize() {
    if (!GetModuleManager())
        return true;

    auto appc = GetModuleManager()->QuerryModule<AppConfig>();

    m_FileName = appc->Get("ConfigPath") + "/" + appc->Get("AppName") + ".xml";

    Load();
    GetModuleManager()->LoadSettigs();
    return true;
}

bool iSettings::Finalize() {
    GetModuleManager()->SaveSettigs();
    Save();
    return true;
}

//-----------------------------------------

iSettings* iSettings::GetiSettings() {
    return l_iSettingsInstance;
}

//-----------------------------------------

void iSettings::Load() {
    m_SettingsDoc = std::make_unique<pugi::xml_document>();
    m_SettingsDoc->load_file(m_FileName.c_str());

    auto customRoot = CustomSettingsRoot();
    for (auto mod : GetModuleManager()->QuerryInterfaces<CustomConfigSet>()) {
        auto name = mod.m_Interface->GetName();
        auto node = customRoot.child(name.c_str());
        mod.m_Interface->LoadSettings(node);
    }
}

void iSettings::Save() {
    {
        auto root = CustomSettingsRoot();
        root.parent().remove_child(root);
    }

    auto customRoot = CustomSettingsRoot();
    for (auto mod : GetModuleManager()->QuerryInterfaces<CustomConfigSet>()) {
        auto name = mod.m_Interface->GetName();
        auto node = customRoot.child(name.c_str());
        if (!node)
            node = customRoot.append_child(name.c_str());
        mod.m_Interface->SaveSettings(node);
    }

    m_SettingsDoc->save_file(m_FileName.c_str());
}

void iSettings::SaveSettings(iSettingsUser *user) {
    auto &group = user->GetSettingID();
    if (group.empty())
        return;
    auto root = DynamicSettingsRoot();
    auto child = root.child(group.c_str());
    if (!child) {
        child = root.append_child(group.c_str());
    }
    user->DoSaveSettings(child);
}

void iSettings::LoadSettings(iSettingsUser *user) {
    auto &group = user->GetSettingID();
    if (group.empty())
        return;
    user->DoLoadSettings(DynamicSettingsRoot().child(group.c_str()));
}

pugi::xml_node iSettings::CustomSettingsRoot() {
    auto root = m_SettingsDoc->document_element();
    if (!root) {
        root = m_SettingsDoc->append_child("Settings");
    }
    auto Static = root.child("Custom");
    if (!Static) {
        Static = root.append_child("Custom");
    }
    return Static;
}

pugi::xml_node iSettings::DynamicSettingsRoot() {
    auto root = m_SettingsDoc->document_element();
    if (!root) {
        root = m_SettingsDoc->append_child("Settings");
    }
    auto dynamic = root.child("Dynamic");
    if (!dynamic) {
        dynamic = root.append_child("Dynamic");
    }
    return dynamic;
}

//-----------------------------------------

void iSettings::iSettingsUser::SaveSettings() {
    if(m_EnableSettings)
        iSettings::GetiSettings()->SaveSettings(this);
}

void iSettings::iSettingsUser::LoadSettings() {
    if (m_EnableSettings)
        iSettings::GetiSettings()->LoadSettings(this);
}

} //namespace MoonGlare
