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


    m_FileName = appc->Get("ConfigFile");

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
	LoadStaticSettings(StaticSettingsRoot());
}

void iSettings::Save() {
	{
		auto root = StaticSettingsRoot();
		root.parent().remove_child(root);
	}
	SaveStaticSettings(StaticSettingsRoot());
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

pugi::xml_node iSettings::StaticSettingsRoot() {
	auto root = m_SettingsDoc->document_element();
	if (!root) {
		root = m_SettingsDoc->append_child("Settings");
	}
	auto Static = root.child("Settings");
	if (!Static) {
		Static = root.append_child("Settings");
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

void iSettings::SaveStaticSettings(pugi::xml_node node) {}
void iSettings::LoadStaticSettings(pugi::xml_node node) {}

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
