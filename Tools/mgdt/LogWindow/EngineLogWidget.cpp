#include PCH_HEADER
#include "EngineLogWidget.h"
#include "ui_EngineLogWidget.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

//-----------------------------------------

EngineLogWidget::EngineLogWidget(QWidget *parent): LogWindowBaseTab(parent) {
	SetName("Engine");
	ui = new Ui::EngineLogWidget();
	ui->setupUi(this);

	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeView->setExpandsOnDoubleClick(false);
	ui->treeView->setRootIsDecorated(false);
}

EngineLogWidget::~EngineLogWidget() {
	delete ui;
}

//-----------------------------------------
