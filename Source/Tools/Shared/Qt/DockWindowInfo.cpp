#include PCH_HEADER

#include "DockWindow.h"
#include "DockWindowInfo.h"

namespace MoonGlare {
namespace QtShared {

//SPACERTTI_IMPLEMENT_STATIC_CLASS(DockWindowInfo);

DockWindowInfo::DockWindowInfo(QWidget *Parent) {
	m_DisplayName = "{?}";
	m_Parent = Parent;
	m_DisableMainMenu = true;
}

DockWindowInfo::~DockWindowInfo() {
	ReleaseInstance();
}

std::shared_ptr<DockWindow> DockWindowInfo::GetInstance(QWidget *parent) {
	if (!parent)
		parent = m_Parent;
	if (!m_Instance) {
		m_Instance = CreateInstance(parent);
		connect(m_Instance.get(), SIGNAL(WindowClosed(DockWindow*)), SLOT(WindowClosed(DockWindow*)));
		m_Instance->LoadSettings();
	}
	return m_Instance;
}

void DockWindowInfo::ReleaseInstance() {
	if (!m_Instance)
		return;
	m_Instance->SaveSettings();
	m_Instance.reset();
}

void DockWindowInfo::WindowClosed(DockWindow* Sender) {
	ReleaseInstance();
}

void DockWindowInfo::Show() {
	GetInstance(m_Parent)->show();
}

bool DockWindowInfo::DoSaveSettings(pugi::xml_node node) const {
	XML::UniqeChild(node, "DockWindowInfo:Visible").text() = static_cast<bool>(m_Instance);
	return true;
}

bool DockWindowInfo::DoLoadSettings(const pugi::xml_node node) {
	if (node.child("DockWindowInfo:Visible").text().as_bool())
		Show();
	return false;
}

} //namespace QtShared
} //namespace MoonGlare
