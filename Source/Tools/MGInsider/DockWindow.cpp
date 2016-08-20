#include PCH_HEADER
#include "DockWindow.h"
#include "Notifications.h"

DockWindow::DockWindow(QWidget *parent, bool AutoRefresh)
	: QDockWidget(parent)
{
	connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(Refresh()));
	connect(Notifications::Get(), SIGNAL(OnEngineDisconnected()), SLOT(OnEngineDisconnected()));
	connect(Notifications::Get(), SIGNAL(OnEngineConnected()), SLOT(OnEngineConnected()));

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

void DockWindow::Refresh() {
}

void DockWindow::SetAutoRefresh(bool value, unsigned Interval) {
	if (!value) {
		m_RefreshTimer.reset();
	}
	else {
		m_RefreshTimer = std::make_unique<QTimer>();
		m_RefreshTimer->setInterval(1000);
		m_RefreshTimer->setSingleShot(false);
		m_RefreshTimer->stop();
		connect(m_RefreshTimer.get(), SIGNAL(timeout()), SLOT(OnRefreshTimer()));
	}
}

void DockWindow::OnEngineDisconnected() {
	if(m_RefreshTimer)
		m_RefreshTimer->stop();
}

void DockWindow::OnEngineConnected() {
	if (m_RefreshTimer)
		m_RefreshTimer->start();
	OnRefreshTimer();
}

void DockWindow::OnRefreshTimer() {
	Refresh();
}
