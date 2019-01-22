#include PCH_HEADER

#include <qmainwindow.h>

#include "DockWindow.h"
#include "DockWindowInfo.h"

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
        QMainWindow *mw = nullptr;
	    if (!parent) {
		    //TODO: this is workaround
		    auto module = dynamic_cast<iModule*>(this);
		    if (module) {
			    auto provider = module->GetModuleManager()->QuerryModule<MainWindowProvider>();
			    parent = mw = provider->GetMainWindowWidget();
		    }
	    }
		m_Instance = CreateInstance(parent);
        if (!m_Instance)
            return nullptr;
        if (mw)
            mw->addDockWidget(Qt::LeftDockWidgetArea, m_Instance.get());
        
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

    auto module = dynamic_cast<iModule*>(this);
    if (module) {
        auto provider = module->GetModuleManager()->QuerryModule<MainWindowProvider>();
        auto mw = provider->GetMainWindowWidget();
        mw->removeDockWidget(m_Instance.get());
    }

	m_Instance->SaveSettings();
	m_Instance.reset();
}

void BaseDockWindowModule::WindowClosed(DockWindow* Sender) {
    visible = false;
	ReleaseInstance();
}

void BaseDockWindowModule::Show() {
    visible = true;
    auto inst = GetInstance(nullptr);
    if (inst)
        inst->show();
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
