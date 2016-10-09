#include PCH_HEADER
#include "DockWindow.h"

namespace MoonGlare {
namespace QtShared {

DockWindow::DockWindow(QWidget *parent, bool AutoRefresh)
	: QDockWidget(parent)
{
	SetAutoRefresh(AutoRefresh);
}

DockWindow::~DockWindow()
{
}

//-----------------------------------------

void DockWindow::closeEvent(QCloseEvent * event) {
	event->accept();
	emit WindowClosed(this);
}

void DockWindow::showEvent(QShowEvent * event) {
	event->accept();
}

bool DockWindow::DoSaveSettings(pugi::xml_node node) const {
	XML::UniqeChild(node, "DockWindow:Geometry").text() = saveGeometry().toHex().constData();
	return true;
}

bool DockWindow::DoLoadSettings(const pugi::xml_node node) {
	restoreGeometry(QByteArray::fromHex(node.child("DockWindow:Geometry").text().as_string()));
	return true;
}

void DockWindow::SetAutoRefresh(bool value, unsigned Interval) {
	if (!value) {
		m_RefreshTimer.reset();
	} else {
		m_RefreshTimer = std::make_unique<QTimer>();
		m_RefreshTimer->setInterval(Interval);
		m_RefreshTimer->setSingleShot(false);
		connect(m_RefreshTimer.get(), &QTimer::timeout, [this] { Refresh(); });
	}
}

void DockWindow::Refresh() {}

} //namespace QtShared
} //namespace MoonGlare
