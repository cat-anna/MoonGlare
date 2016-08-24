#include PCH_HEADER
#include "MemoryState.h"
#include "ui_MemoryState.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

class MemoryState::MemoryRequest : public RemoteConsoleEnumerationObserver {
	SPACERTTI_CLASSINFO(MemoryRequest);
public:
	MemoryRequest(QStandardItemModel *Model, QTreeView *TreeView, MemoryState *Owner):
			RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes::EnumerateMemory, ""), m_Owner(Owner) {
		m_Model = Model;
		m_ItemParent = m_Model->invisibleRootItem();
		m_TreeView = TreeView;
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override { 
		auto hdr = message.GetAndPull<InsiderApi::PayLoad_ListBase>();
		m_Model->clear();
		std::unordered_map<uint32_t, QStandardItem*> m_Owners;
		auto *root = m_ItemParent;
		for (unsigned i = 0; i < hdr->Count; ++i) {
			auto *item = message.GetAndPull<InsiderApi::PayLoad_MemoryStatus>();
			
			const char *Name = message.PullString();
			const char *OwnerName = message.PullString();
			char buffer[128];

			auto Owner = m_Owners[item->OwnerID];
			if (!Owner) {
				QList<QStandardItem*> xcols;
				xcols << (Owner = new QStandardItem(OwnerName));
				m_Owners[item->OwnerID] = Owner;
				root->appendRow(xcols);
			}

			QList<QStandardItem*> cols;
			cols << new QStandardItem(Name);
			sprintf_s(buffer, "%d [%.2f%%]", item->Allocated, ((float)item->Allocated / (float)item->Capacity) * 100.0f);
			cols << new QStandardItem(buffer);
			cols << new QStandardItem(std::to_string(item->Capacity).c_str());
			cols << new QStandardItem("?"); //itoa(item->MaxAllocated, buffer, 10));
			cols << new QStandardItem(std::to_string(item->ElementSize).c_str());

			Owner->appendRow(cols);
		}
		if(hdr->Count > 0)
			root->sortChildren(0);
	//	root->expand();
	//	m_Owner->m_ViewModel->
		m_TreeView->expandToDepth(2);
		return HanderStatus::Remove; 
	};
private:
	QStandardItemModel *m_Model;
	QStandardItem *m_ItemParent;
	QTreeView *m_TreeView;
	MemoryState *m_Owner;
};

//-----------------------------------------
//-----------------------------------------

struct MemoryStateInfo : public DockWindowInfo {
	virtual std::shared_ptr<DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<MemoryState>(parent);
	}

	MemoryStateInfo() {
		SetSettingID("MemoryStateInfo");
		SetDisplayName(tr("Memory state"));
		SetShortcut("F10");
	}
};
DockWindowClassRgister::Register<MemoryStateInfo> MemoryStateInfoReg("MemoryState");

MemoryState::MemoryState(QWidget *parent) 
		: DockWindow(parent) {

	SetSettingID("MemoryState");
	SetQueueName("MemoryState");
	m_Ui = std::make_unique<Ui::MemoryState>();
	m_Ui->setupUi(this);

	m_Ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	//ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//ui->treeView->setExpandsOnDoubleClick(false);
	//ui->treeView->setRootIsDecorated(false);

	ResetTreeView();
	SetAutoRefresh(true);
}

MemoryState::~MemoryState() {
	ResetTreeView();
	m_Ui.reset();
}

//-----------------------------------------

bool MemoryState::DoSaveSettings(pugi::xml_node node) const {
	DockWindow::DoSaveSettings(node);
	//	SaveState(node, m_Ui->splitter, "Splitter:State");
	return true;
}

bool MemoryState::DoLoadSettings(const pugi::xml_node node) {
	DockWindow::DoLoadSettings(node);
	//	LoadState(node, m_Ui->splitter, "Splitter:State");
	return true;
}

void MemoryState::ResetTreeView() {
//	auto &settings = mgdtSettings::get();
	m_Ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Allocated"));
	m_ViewModel->setHorizontalHeaderItem(2, new QStandardItem("Capacity"));
	m_ViewModel->setHorizontalHeaderItem(3, new QStandardItem("Peek"));
	m_ViewModel->setHorizontalHeaderItem(4, new QStandardItem("Element size"));

    m_Ui->treeView->setModel(m_ViewModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	
	m_Ui->treeView->setColumnWidth(0, 200);
	m_Ui->treeView->setColumnWidth(1, 70);
	m_Ui->treeView->setColumnWidth(2, 70);
	m_Ui->treeView->setColumnWidth(3, 70);
	m_Ui->treeView->setColumnWidth(4, 70);
}

//-----------------------------------------

void MemoryState::Refresh() {
	CancelRequests();

	QueueRequest(std::make_shared<MemoryRequest>(m_ViewModel.get(), m_Ui->treeView, this));
}

//-----------------------------------------
