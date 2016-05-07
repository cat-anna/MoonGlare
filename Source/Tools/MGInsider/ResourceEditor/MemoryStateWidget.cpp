#include PCH_HEADER
#include "MemoryStateWidget.h"
#include "ui_MemoryStateWidget.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

class MemoryStateWidget::MemoryRequest : public RemoteConsoleEnumerationObserver {
public:
	MemoryRequest(QStandardItem *parent, MemoryStateWidget *Owner):
			RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes::EnumerateMemory, ""), m_Owner(Owner) {
		m_ItemParent = parent;
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override { 
		auto hdr = message.GetAndPull<InsiderApi::PayLoad_ListBase>();

		auto *root = m_ItemParent;
		for (unsigned i = 0; i < hdr->Count; ++i) {
			auto *item = message.GetAndPull<InsiderApi::PayLoad_MemoryStatus>();
			
			const char *Name = message.PullString();
			QList<QStandardItem*> cols;
			cols << new QStandardItem(Name);
			char buffer[64];
			sprintf_s(buffer, "%d [%.2f%%]", item->Allocated, ((float)item->Allocated / (float)item->Capacity) * 100.0f);
			cols << new QStandardItem(buffer);
			cols << new QStandardItem(itoa(item->Capacity, buffer, 10));
			cols << new QStandardItem("?"); //itoa(item->MaxAllocated, buffer, 10));
			cols << new QStandardItem(itoa(item->ElementSize, buffer, 10));
			root->appendRow(cols);
		}
		if(hdr->Count > 0)
			root->sortChildren(0);
		return HanderStatus::Remove; 
	};
private:
	QStandardItem *m_ItemParent;
	MemoryStateWidget *m_Owner;
};

//-----------------------------------------
//-----------------------------------------

static ResourceEditorTabRegister::Register<MemoryStateWidget> _Register("MemoryStateWidget");

MemoryStateWidget::MemoryStateWidget(QWidget *parent) : ResourceEditorBaseTab(parent) {
	SetName("Memory");
	ui = new Ui::MemoryStateWidget();
	ui->setupUi(this);
	
	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeView->setExpandsOnDoubleClick(false);
	ui->treeView->setRootIsDecorated(false);

	ResetTreeView();
}

MemoryStateWidget::~MemoryStateWidget() {
	ResetTreeView();
	delete ui;
}

//-----------------------------------------

void MemoryStateWidget::ResetTreeView() {
//	auto &settings = mgdtSettings::get();
    ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Allocated"));
	m_ViewModel->setHorizontalHeaderItem(2, new QStandardItem("Capacity"));
	m_ViewModel->setHorizontalHeaderItem(3, new QStandardItem("Peek"));
	m_ViewModel->setHorizontalHeaderItem(4, new QStandardItem("Element size"));

    ui->treeView->setModel(m_ViewModel.get());
	ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	  
	ui->treeView->setColumnWidth(0, 200);
	ui->treeView->setColumnWidth(1, 70);
	ui->treeView->setColumnWidth(2, 70);
	ui->treeView->setColumnWidth(3, 70);
	ui->treeView->setColumnWidth(4, 70);
}

//-----------------------------------------

void MemoryStateWidget::Refresh() {
	CancelRequests();
	ResetTreeView();

	QueueRequest(std::make_shared<MemoryRequest>(m_ViewModel->invisibleRootItem(), this));
}

//-----------------------------------------

