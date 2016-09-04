#include PCH_HEADER
#include "MemoryState.h"
#include "ui_MemoryState.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

#include <queue>

class MemoryState::MemoryRequest : public RemoteConsoleEnumerationObserver {
//	SPACERTTI_CLASSINFO(MemoryRequest);
public:
	MemoryRequest(QTreeView *TreeView, MemoryState *Owner):
			RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes::EnumerateMemory, ""), m_Owner(Owner) {
		m_TreeView = TreeView;
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override { 
		auto hdr = message.GetAndPull<InsiderApi::PayLoad_ListBase>();
		auto model = m_Owner->GetModel();
		model->removeRows(0, model->rowCount());
		std::unordered_map<uint64_t, QStandardItem*> m_Owners;
		
		auto GetOwner = [&m_Owners](uint64_t id) -> QStandardItem* {
			auto it = m_Owners.find(id);
			if (it == m_Owners.end())
				return nullptr;
			return it->second;
		};

		struct Info {
			InsiderApi::PayLoad_MemoryStatus *m_Ptr;
			std::string m_Name, m_OwnerName;
			bool m_Processed = false;
		};

		std::queue<Info> Infos;
		for (unsigned i = 0; i < hdr->Count; ++i) {
			auto *item = message.GetAndPull<InsiderApi::PayLoad_MemoryStatus>();
			const char *Name = message.PullString();
			const char *OwnerName = message.PullString();

			Info info;
			info.m_Ptr = item;
			info.m_Name = Name;
			info.m_OwnerName = OwnerName;
			Infos.push(std::move(info));
		}

		auto *root = model->invisibleRootItem();
		while(!Infos.empty()) {
			Info info = Infos.front();
			Infos.pop();
			auto *item = info.m_Ptr;

			char buffer[128];
			auto Owner = GetOwner(item->ID);
			if (!Owner) {
				if (item->ParentID) {
					auto Parent = GetOwner(item->ParentID);
					if (!Parent) {
						if (info.m_Processed) {
							AddLogf(Error, "Unable to process memory status entry: %s/%s parent: %llx", info.m_OwnerName.c_str(), info.m_Name.c_str(), item->ParentID);
							continue;
						}
						info.m_Processed = true;
						Infos.push(info);
						continue;
					}
					QList<QStandardItem*> xcols;
					xcols << (Owner = new QStandardItem("Children"));
					m_Owners[item->ID] = Owner;
					Parent->appendRow(xcols);
				} else {
					QList<QStandardItem*> xcols;
					xcols << (Owner = new QStandardItem(info.m_OwnerName.c_str()));
					m_Owners[item->ID] = Owner;
					root->appendRow(xcols);
				}
			}

			QList<QStandardItem*> cols;
			cols << new QStandardItem(info.m_Name.c_str());
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

	QueueRequest(std::make_shared<MemoryRequest>(m_Ui->treeView, this));
}

//-----------------------------------------
