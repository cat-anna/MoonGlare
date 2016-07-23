#include PCH_HEADER
#include "SubWindow.h"

SubWindow::SubWindow(QWidget *parent)
	: QMainWindow(parent)
{

}

SubWindow::~SubWindow()
{

}

//-----------------------------------------

void SubWindow::closeEvent(QCloseEvent * event) {
	event->accept();
	emit WindowClosed();
}

void SubWindow::showEvent(QShowEvent * event) {
	event->accept();
}