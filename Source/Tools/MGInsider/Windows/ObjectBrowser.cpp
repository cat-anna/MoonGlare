#include PCH_HEADER
//#include "pch.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "ui_ObjectBrowser.h"
#include "ObjectBrowser.h"
#include <DockWindowInfo.h>
#include <Notifications.h>

#include <QStandardItemModel>

#include <Utils/Streamutils.h>

//-----------------------------------------

class ObjectBrowser::Request : public RemoteConsoleObserver {
public:
	Request(ObjectBrowser *Owner) :
			RemoteConsoleObserver(InsiderApi::MessageTypes::EnumerateEntities, Owner),
			m_Owner(Owner) {
	}

	using ItemMap = std::unordered_map<uint32_t, QStandardItem*>;

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &inmessage) override {
		auto hdr = inmessage.GetAndPull<InsiderApi::PayLoad_ListBase>();

		auto model = m_Owner->GetModel();
		model->removeRows(0, model->rowCount());
		ItemMap items;

		for (size_t idx = 0; idx < hdr->Count; ++idx) {
			auto *item = inmessage.GetAndPull<InsiderApi::PayLoad_EntityInfo>();
			const char *Name = inmessage.PullString();

			QStandardItem *parent;
			auto pptrit = items.find(item->ParentEntity);
			if (pptrit == items.end()) {
				parent = model->invisibleRootItem();
			} else {
				parent = pptrit->second;
			}

			QStandardItem *qitem = new QStandardItem(Name);
			items[item->SelfEntity] = qitem;
			QList<QStandardItem*> cols;
			cols << qitem;

			parent->appendRow(cols);
		}
#if 0
		auto Objdata = m_Owner->GetData();
		Objdata->m_ObjectMap.clear();
		model->removeRows(0, model->rowCount());

		Objdata->m_Data.CloneFrom(inmessage);

		if (Objdata->m_Data.GetHeader()->MessageType != InsiderApi::MessageTypes::ObjectList) {
			AddLog(Warning, "Enumerating objects is not possible in current state");
			m_Owner->RequestFinished(this);
			return HanderStatus::Remove;
		}
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
//		m_Parent->sortChildren(0);
//		m_SoundParent->sortChildren(0);
//		m_MusicParent->sortChildren(0);
#endif
		return HanderStatus::Remove;
	};
private:
	ObjectBrowser *m_Owner;
};

//-----------------------------------------
//-----------------------------------------

struct ObjectBrowserInfo : public QtShared::DockWindowInfo {
	virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<ObjectBrowser>(parent);
	}

	ObjectBrowserInfo(QWidget *Parent) : QtShared::DockWindowInfo(Parent) {
		SetSettingID("ObjectBrowserInfo");
		SetDisplayName(tr("Object browser"));
		SetShortcut("F9");
	}
};
QtShared::DockWindowClassRgister::Register<ObjectBrowserInfo> ObjectBrowserInfoReg("ObjectBrowser");

ObjectBrowser::ObjectBrowser(QWidget *parent)
	: QtShared::DockWindow(parent)
{
	SetSettingID("ObjectBrowser");
	SetQueueName("Object browser");
	m_Ui = std::make_unique<Ui::ObjectBrowser>();
	m_Ui->setupUi(this);

	m_ViewModel = std::make_unique<QStandardItemModel>();
	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Tree"));
//	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Value"));

	m_Ui->treeViewObjects->setModel(m_ViewModel.get());
	m_Ui->treeViewObjects->setColumnWidth(0, 150);
	m_Ui->treeViewObjects->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//connect(m_Ui->treeViewObjects, SIGNAL(clicked(const QModelIndex &)), SLOT(ItemClicked(const QModelIndex&)));

	connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(Refresh()));
	connect(Notifications::Get(), SIGNAL(OnEngineConnected()), SLOT(Refresh()));
	connect(m_Ui->actionRefresh, SIGNAL(triggered()), SLOT(Refresh()));

	SetAutoRefresh(true);
}

ObjectBrowser::~ObjectBrowser() {
	m_Ui.reset();
}

bool ObjectBrowser::DoSaveSettings(pugi::xml_node node) const {
	DockWindow::DoSaveSettings(node);
	//DockWindow::SaveColumns(node, "treeViewObjects:Columns", m_Ui->treeViewObjects, 1);
	return true;
}

bool ObjectBrowser::DoLoadSettings(const pugi::xml_node node) {
	DockWindow::DoLoadSettings(node);
	//DockWindow::LoadColumns(node, "treeViewObjects:Columns", m_Ui->treeViewObjects, 1);
	return true;
}

//-----------------------------------------

void ObjectBrowser::ItemClicked(const QModelIndex& index) {
#if 0
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
#endif
}

template<class T>
std::string tostring(const T& vec) {
	std::stringstream ss;
	ss << vec;
	return ss.str();
}

#if 0
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
#endif

void ObjectBrowser::Refresh() {
	if (!m_Request)
		m_Request = std::make_shared<Request>(this);
	QueueRequest(m_Request);
}
