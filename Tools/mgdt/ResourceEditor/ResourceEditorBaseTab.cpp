#include PCH_HEADER
#include "ResourceEditorBaseTab.h"

ResourceEditorBaseTab::ResourceEditorBaseTab(QWidget *parent)
	: QWidget(parent) {

}

ResourceEditorBaseTab::~ResourceEditorBaseTab() {
}

void ResourceEditorBaseTab::showEvent(QShowEvent * event) {
	QWidget::showEvent(event);
	Refresh();
}

//-----------------------------------------
