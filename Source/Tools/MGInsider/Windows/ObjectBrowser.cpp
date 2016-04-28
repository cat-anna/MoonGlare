#include PCH_HEADER
//#include "pch.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "ui_ObjectBrowser.h"
#include "ObjectBrowser.h"
#include <DockWindowInfo.h>
#include <Notifications.h>

#include <QStandardItemModel>

#include <Streamutils.h>

//-----------------------------------------

struct HandleLess {
	bool operator () (MoonGlare::Handle a, MoonGlare::Handle b) {
		return a.GetIndex() < b.GetIndex();
	}
};

struct ObjectBrowser::ObjectData {
	struct Triplet {
		MoonGlare::Handle m_Handle;
		QStandardItem* m_ViewItem;
		InsiderApi::PayLoad_ObjectInfo *m_Info;

		Triplet(QStandardItem* item, MoonGlare::Handle h = MoonGlare::Handle(), InsiderApi::PayLoad_ObjectInfo* data = nullptr) {
			m_Handle = h;
			m_ViewItem = item;
			m_Info = data;

		}
	};

	std::map<MoonGlare::Handle, Triplet, HandleLess> m_ObjectMap;

	InsiderApi::InsiderMessageBuffer m_Data;
};

//-----------------------------------------

class ObjectBrowser::Request : public RemoteConsoleObserver {
public:
	Request(ObjectBrowser *Owner) :
			RemoteConsoleObserver(InsiderApi::MessageTypes::EnumerateObjects, Owner),
			m_Owner(Owner) {
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &inmessage) override {
		auto Objdata = m_Owner->GetData();
		auto model = m_Owner->GetModel();
		Objdata->m_ObjectMap.clear();
		model->removeRows(0, model->rowCount());

		Objdata->m_Data.CloneFrom(inmessage);

		if (Objdata->m_Data.GetHeader()->MessageType != InsiderApi::MessageTypes::ObjectList) {
			AddLog(Warning, "Enumerating objects is not possible in current state");
			m_Owner->RequestFinished(this);
			return HanderStatus::Remove;
		}
#if 0
		struct PayLoad_ObjectInfo {
			u32 ID;
			u32 ParentID;
			float Position[3];
			float Quaternion[3];
			u16 NameLen;
			u8 Name[0];
		};
#endif // 0

		auto hdr = Objdata->m_Data.GetAndPull<InsiderApi::PayLoad_ListBase>();

		using Triplet = ObjectData::Triplet;
		Objdata->m_ObjectMap.insert(std::make_pair(MoonGlare::Handle(), Triplet(model->invisibleRootItem())));

		for (unsigned i = 0; i < hdr->Count; ++i) {
			auto *item = Objdata->m_Data.GetAndPull<InsiderApi::PayLoad_ObjectInfo>();

			const char *Name = Objdata->m_Data.PullString();

			QStandardItem *qitem = new QStandardItem(Name);
			Objdata->m_ObjectMap.insert(std::make_pair(item->ObjectHandle, Triplet(qitem, item->ObjectHandle, item)));
			qitem->setData(QVariant::fromValue(item->ObjectHandle), Qt::UserRole);
			QList<QStandardItem*> cols;
			cols << qitem;

			auto parent = Objdata->m_ObjectMap.find(item->ParentHandle);
			if(parent != Objdata->m_ObjectMap.end())
				parent->second.m_ViewItem->appendRow(cols);
		}

		m_Owner->RefreshDetailsView();
//		
//		m_Parent->sortChildren(0);
//		m_SoundParent->sortChildren(0);
//		m_MusicParent->sortChildren(0);
		return HanderStatus::Remove;
	};
private:
	ObjectBrowser *m_Owner;
};

//-----------------------------------------
//-----------------------------------------

struct ObjectBrowserInfo : public DockWindowInfo {
	virtual std::shared_ptr<DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<ObjectBrowser>(parent);
	}

	ObjectBrowserInfo() {
		SetSettingID("ObjectBrowserInfo");
		SetDisplayName(tr("Object browser"));
		SetShortcut("F9");
	}
};
DockWindowClassRgister::Register<ObjectBrowserInfo> ObjectBrowserInfoReg("ObjectBrowser");

ObjectBrowser::ObjectBrowser(QWidget *parent)
	: DockWindow(parent)
{
	SetSettingID("ObjectBrowser");
	SetQueueName("Object browser");
	m_Data = std::make_unique<ObjectData>();
	m_Ui = std::make_unique<Ui::ObjectBrowser>();
	m_Ui->setupUi(this);

	m_ViewModel = std::make_unique<QStandardItemModel>();
	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
//	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Value"));

	m_Ui->treeViewObjects->setModel(m_ViewModel.get());
	m_Ui->treeViewObjects->setColumnWidth(0, 150);
	m_Ui->treeViewObjects->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_DetailsModel = std::make_unique<QStandardItemModel>();
	m_Ui->treeViewDetails->setModel(m_DetailsModel.get());
	m_DetailsModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_DetailsModel->setHorizontalHeaderItem(1, new QStandardItem("Value"));

	m_Ui->treeViewDetails->setColumnWidth(0, 100);
	m_Ui->treeViewDetails->setColumnWidth(1, 150);
	m_Ui->treeViewDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(RefreshView()));
	connect(Notifications::Get(), SIGNAL(OnEngineConnected()), SLOT(RefreshView()));
	connect(m_Ui->actionRefresh, SIGNAL(triggered()), SLOT(RefreshView()));

//	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
//	ui->treeView->setExpandsOnDoubleClick(false);
//	ui->treeView->addAction(ui->actionExecute_quick_action);
//	auto a = new QAction(this); a->setSeparator(true);
//	ui->treeView->addAction(a);
//	ui->treeView->addAction(ui->actionModify_quick_action);
//	ui->treeView->addAction(ui->actionRemove_quick_action);
	connect(m_Ui->treeViewObjects, SIGNAL(clicked(const QModelIndex &)), SLOT(ItemClicked(const QModelIndex&)));
//	connect(ui->actionNew_quick_action, SIGNAL(triggered()), SLOT(NewAction()));
//	connect(ui->actionRemove_quick_action, SIGNAL(triggered()), SLOT(RemoveAction()));
//	connect(ui->actionModify_quick_action, SIGNAL(triggered()), SLOT(ModifyAction()));
//	connect(ui->actionExecute_quick_action, SIGNAL(triggered()), SLOT(ExecuteAction()));
}

ObjectBrowser::~ObjectBrowser() {
	m_Data.reset();
	m_Ui.reset();
}

bool ObjectBrowser::DoSaveSettings(pugi::xml_node node) const {
	DockWindow::DoSaveSettings(node);
	SaveState(node, m_Ui->splitter, "Splitter:State");
	return true;
}

bool ObjectBrowser::DoLoadSettings(const pugi::xml_node node) {
	DockWindow::DoLoadSettings(node);
	LoadState(node, m_Ui->splitter, "Splitter:State");
	return true;
}

//-----------------------------------------

void ObjectBrowser::ItemClicked(const QModelIndex& index) {
	auto row = index.row();
	auto parent = index.parent();
	auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

	auto itemptr = m_ViewModel->itemFromIndex(selectedindex);
	if (!itemptr) {
		m_SelectedItem = MoonGlare::Handle();
		m_DetailsModel->removeRows(0, m_DetailsModel->rowCount());
		return;
	}

	m_SelectedItem = itemptr->data(Qt::UserRole).value<MoonGlare::Handle>();

	RefreshDetailsView();
}

template<class T>
std::string tostring(const T& vec) {
	std::stringstream ss;
	ss << vec;
	return ss.str();
}

void ObjectBrowser::RefreshDetailsView() {
	m_DetailsModel->removeRows(0, m_DetailsModel->rowCount());

	auto itemit = m_Data->m_ObjectMap.find(m_SelectedItem);
	if (itemit == m_Data->m_ObjectMap.end()) {
		return;
	}

	auto &item = itemit->second;
	auto root = m_DetailsModel->invisibleRootItem();

	auto put = [root](const char *Name, const std::string &Value, QStandardItem* parent = nullptr) {
		QList<QStandardItem*> cols;
		QStandardItem *itm;
		cols << (itm = new QStandardItem(Name));
		cols << new QStandardItem(Value.c_str());
		if(parent)
			parent->appendRow(cols);
		else
			root->appendRow(cols);
		return itm;
	};

	put("Name", (char*)item.m_Info->Name);
	put("Position", tostring(*((glm::vec3*)item.m_Info->Position)));
	put("Quaternion", tostring(*((glm::vec4*)item.m_Info->Quaternion)));

	auto hitm = put("Handle", tostring(item.m_Handle));
	put("Index", tostring(item.m_Handle.GetIndex()), hitm);
	put("Generation", tostring(item.m_Handle.GetGeneration()), hitm);
	put("Type", tostring(item.m_Handle.GetType()), hitm);

}

void ObjectBrowser::RefreshView() {
	QueueRequest(std::make_shared<Request>(this));
}
