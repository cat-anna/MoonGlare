#include PCH_HEADER
#include "DockWindow.h"

DockWindow::DockWindow(QWidget *parent)
	: QDockWidget(parent)
{
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
