#include PCH_HEADER
#include "InsiderLogWidget.h"
#include "ui_InsiderLogWidget.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

//-----------------------------------------

static LogWindowTabRegister::Register<InsiderLogWidget> _InsiderLogWidget("InsiderLogWidget");

InsiderLogWidget::InsiderLogWidget(QWidget *parent) : LogWindowBaseTab(parent) {
	SetName("Insider");
	ui = new Ui::InsiderLogWidget();
	ui->setupUi(this);
	
	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeView->setExpandsOnDoubleClick(false);
	ui->treeView->setRootIsDecorated(false);

	ResetTreeView();
}

InsiderLogWidget::~InsiderLogWidget() {
	ResetTreeView();
	delete ui;
}

//-----------------------------------------

void InsiderLogWidget::ResetTreeView() {
//	auto &settings = mgdtSettings::get();
    ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Allocated"));
	m_ViewModel->setHorizontalHeaderItem(2, new QStandardItem("Capacity"));
	m_ViewModel->setHorizontalHeaderItem(3, new QStandardItem("Element size"));

    ui->treeView->setModel(m_ViewModel.get());
	ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	  
	ui->treeView->setColumnWidth(0, 200);
	ui->treeView->setColumnWidth(1, 70);
	ui->treeView->setColumnWidth(2, 70);
	ui->treeView->setColumnWidth(3, 70);
}

//-----------------------------------------

