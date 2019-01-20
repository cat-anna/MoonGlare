#include PCH_HEADER

#include "DockWindowInfo.h"
#include "DockWindow.h"

#include <ToolBase/Module.h>

#include "MiscIfs.h"

namespace MoonGlare {
namespace QtShared {

BaseDockWindowModule::BaseDockWindowModule(SharedModuleManager modmgr): iModule(std::move(modmgr)) {
	m_DisplayName = "{?}";
	m_DisableMainMenu = true;
}

BaseDockWindowModule::~BaseDockWindowModule() {
}

std::shared_ptr<DockWindow> BaseDockWindowModule::GetInstance(QWidget *parent) {
	if (!m_Instance) {
	    if (!parent) {
		    //TODO: this is workaround
		    auto module = dynamic_cast<iModule*>(this);
		    if (module) {
			    auto provider = module->GetModuleManager()->QuerryModule<MainWindowProvider>();
			    parent = provider->GetMainWindowWidget();
		    }
	    }
		m_Instance = CreateInstance(parent);
		connect(m_Instance.get(), SIGNAL(WindowClosed(DockWindow*)), SLOT(WindowClosed(DockWindow*)));
		m_Instance->LoadSettings();
	}
	return m_Instance;
}

bool BaseDockWindowModule::Finalize() {
    ReleaseInstance();
    return true;
}

void BaseDockWindowModule::ReleaseInstance() {
	if (!m_Instance)
		return;
	m_Instance->SaveSettings();
	m_Instance.reset();
}

void BaseDockWindowModule::WindowClosed(DockWindow* Sender) {
    visible = false;
	ReleaseInstance();
}

void BaseDockWindowModule::Show() {
    visible = true;
	GetInstance(nullptr)->show();
}

bool BaseDockWindowModule::DoSaveSettings(pugi::xml_node node) const {
    XML::UniqeChild(node, "BaseDockWindowModule:Visible").text() = visible;
	return true;
}

bool BaseDockWindowModule::DoLoadSettings(const pugi::xml_node node) {
    if (node.child("BaseDockWindowModule:Visible").text().as_bool()) {
        visible = true;
        Show();
    }
	return false;
}

} //namespace QtShared
} //namespace MoonGlare
