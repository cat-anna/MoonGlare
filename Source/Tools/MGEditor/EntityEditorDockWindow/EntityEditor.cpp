/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "EntityEditor.h"

#include <ui_EntityEditor.h>
#include <DockWindowInfo.h>
#include <icons.h>
#include "../MainWindow.h"

#include "EditableEntity.h"

namespace MoonGlare {
namespace Editor {
namespace EntityEditor {

struct EntityEditorInfo 
		: public QtShared::DockWindowInfo
		, public QtShared::iEditorInfo {
	virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<EntityEditorWindow>(parent);
	}

	EntityEditorInfo(QWidget *Parent): QtShared::DockWindowInfo(Parent) {
		SetSettingID("EntityEditorInfo");
		SetDisplayName(tr("EntityEditor"));
		SetShortcut("F2");
	}
	std::vector<QtShared::EditableFielInfo> GetSupportedFileTypes() const override {
		return std::vector<QtShared::EditableFielInfo>{
			QtShared::EditableFielInfo{ "xep", ICON_16_ENTITYPATTERN_RESOURCE, },
		};
	}
};
QtShared::DockWindowClassRgister::Register<EntityEditorInfo> EntityEditorInfoReg("EntityEditor");

//----------------------------------------------------------------------------------

class ComponentValueItemDelegate : public QStyledItemDelegate {
//	Q_OBJECT
	EntityEditorWindow *m_Owner;
public:
	ComponentValueItemDelegate(EntityEditorWindow *parent = 0) : QStyledItemDelegate(parent), m_Owner(parent) { }
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		auto vinfo = index.data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
		if (vinfo && vinfo.m_ValueInterface) {
			auto einfoit = TypeInfoMap.find(vinfo.m_ValueInterface->GetTypeName());
			if (einfoit != TypeInfoMap.end()) {
				return einfoit->second->CreateEditor(parent)->GetWidget();
			}
		}
		return QStyledItemDelegate::createEditor(parent, option, index);
	};
	void setEditorData(QWidget *editor, const QModelIndex &index) const override {
		auto vinfo = index.data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
		CustomTypeEditor *cte = dynamic_cast<CustomTypeEditor*>(editor);
		if (vinfo && vinfo.m_ValueInterface && cte) {
			cte->SetValue(vinfo.m_ValueInterface->GetValue());
		}
		return QStyledItemDelegate::setEditorData(editor, index);
	};
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
		auto vinfo = index.data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
		CustomTypeEditor *cte = dynamic_cast<CustomTypeEditor*>(editor);
		if (vinfo && vinfo.m_ValueInterface && cte) {
			vinfo.m_ValueInterface->SetValue(cte->GetValue());
		}
		return QStyledItemDelegate::setModelData(editor, model, index);
	};
};

//----------------------------------------------------------------------------------


EntityEditorWindow::EntityEditorWindow(QWidget * parent)
	:  QtShared::DockWindow(parent) {
	SetSettingID("EntityEditorWindow");

	m_Ui = std::make_unique<Ui::EntityEditor>();
	m_Ui->setupUi(this);

	connect(m_Ui->pushButton, &QPushButton::clicked, this, &EntityEditorWindow::ShowAddComponentMenu);

	connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(RefreshView()));

	m_EntityModel = std::make_unique<QStandardItemModel>();
	m_EntityModel->setHorizontalHeaderItem(0, new QStandardItem("Entity tree"));
	connect(m_EntityModel.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(EntityChanged(QStandardItem *)));
	m_Ui->treeView->setModel(m_EntityModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_Ui->treeView, SIGNAL(clicked(const QModelIndex &)), SLOT(EntityClicked(const QModelIndex&)));
	connect(m_Ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(EntityContextMenu(const QPoint &)));

	m_ComponentModel = std::make_unique<QStandardItemModel>();
	m_ComponentModel->setHorizontalHeaderItem(0, new QStandardItem("Values tree"));
	m_ComponentModel->setHorizontalHeaderItem(1, new QStandardItem("Values"));
	m_ComponentModel->setHorizontalHeaderItem(2, new QStandardItem("Description"));
	connect(m_ComponentModel.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(ComponentChanged(QStandardItem *)));
	m_Ui->treeViewDetails->setModel(m_ComponentModel.get());
	m_Ui->treeViewDetails->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeViewDetails->setContextMenuPolicy(Qt::CustomContextMenu);
	m_Ui->treeViewDetails->setItemDelegate(new ComponentValueItemDelegate(this));
	m_Ui->treeViewDetails->setColumnWidth(0, 200);
	m_Ui->treeViewDetails->setColumnWidth(1, 100);
	m_Ui->treeViewDetails->setColumnWidth(2, 100);
	connect(m_Ui->treeViewDetails, SIGNAL(clicked(const QModelIndex &)), SLOT(ComponentClicked(const QModelIndex&)));
	connect(m_Ui->treeViewDetails, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ComponentContextMenu(const QPoint &)));
}

EntityEditorWindow::~EntityEditorWindow() {
	m_Ui.reset();
}

//----------------------------------------------------------------------------------

bool EntityEditorWindow::DoSaveSettings(pugi::xml_node node) const {
	QtShared::DockWindow::DoSaveSettings(node);
	//node.append_child("File").text() = m_CurrentPatternFile.c_str();
	return true;
}

bool EntityEditorWindow::DoLoadSettings(const pugi::xml_node node) {
	QtShared::DockWindow::DoLoadSettings(node);
	//m_CurrentPatternFile = node.child("File").text().as_string("");
	return true;
}

//----------------------------------------------------------------------------------

bool EntityEditorWindow::CloseData() {
	m_EntityModel->removeRows(0, m_EntityModel->rowCount());
	m_ComponentModel->removeRows(0, m_ComponentModel->rowCount());
	m_RootEntity.reset();
	m_CurrentItem = EditableItemInfo();
	Refresh();
	m_CurrentPatternFile.clear();
	return true;
}

bool EntityEditorWindow::OpenData(const std::string &file) {
	if (m_RootEntity) {
		if (!AskForPermission("There is a opened pattern. Do you want to close it?"))
			return true;
		if (IsChanged() && AskForPermission("Save changes?"))
			SaveData();
	}
	CloseData();
 
	auto root = std::make_unique<EditablePattern>();
	if (!root->OpenPattern(file)) {
		CloseData();
		//ToDo: log sth
		return false;
	}
	m_RootEntity.reset(root.release());
	m_CurrentPatternFile = file;
	Refresh();
	return true;
}

bool EntityEditorWindow::SaveData() {
	if (!m_RootEntity)
		return false;
	auto pat = dynamic_cast<EditablePattern*>(m_RootEntity.get());
	
	if (pat) {
		if (!pat->SavePattern(m_CurrentPatternFile)) {
			//ToDo: log sth
		}
		SetModiffiedState(false);
		return true;
	}
	//TODO: log sth
	return false;
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::Refresh() {
	m_EntityModel->removeRows(0, m_EntityModel->rowCount());
	m_CurrentItem = EditableItemInfo();

	if (!m_RootEntity)
		return;

	QStandardItem *root = m_EntityModel->invisibleRootItem();

	std::function<void(EditableEntity*, QStandardItem*)> buidFunc;
	buidFunc = [&buidFunc](EditableEntity *e, QStandardItem *item) {
		QStandardItem *Elem = new QStandardItem(e->GetName().c_str());
		EditableItemInfo eii;
		eii.m_EditableEntity = e;
		eii.m_Item = Elem;
		eii.m_Parent = e->GetParent();
		Elem->setData(QVariant::fromValue(eii), UserRoles::EditableItemInfo);

		QList<QStandardItem*> cols;
		cols << Elem;
		item->appendRow(cols);

		for (auto &child : e->GetChildrenList()) {
			buidFunc(child.get(), Elem);
		}
	};

	buidFunc(m_RootEntity.get(), root);
	m_Ui->treeView->setCurrentIndex(root->index());
	m_Ui->treeView->expandAll();
}

void EntityEditorWindow::RefreshDetails() {
	m_ComponentModel->removeRows(0, m_ComponentModel->rowCount());
	m_CurrentComponent = EditableComponentValueInfo();
	if (!m_CurrentItem)
		return;

	QStandardItem *root = m_ComponentModel->invisibleRootItem();

	for (auto &component : m_CurrentItem.m_EditableEntity->GetComponentList()) {

		QStandardItem *Elem = new QStandardItem(component->GetName().c_str());
		Elem->setFlags(Elem->flags() & ~Qt::ItemIsEditable);

		EditableComponentValueInfo ecvi;
		ecvi.m_OwnerComponent = component.get();
		ecvi.m_Item = Elem;
		ecvi.m_ValueInterface = nullptr;
		ecvi.m_EditableEntity = m_CurrentItem.m_EditableEntity;
		Elem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);

		QList<QStandardItem*> cols;
		cols << Elem;
		root->appendRow(cols);

		for (auto &value : component->GetValues()) {
			QStandardItem *CaptionElem = new QStandardItem(value->GetName().c_str());
			CaptionElem->setFlags(CaptionElem->flags() & ~Qt::ItemIsEditable);

			EditableComponentValueInfo ecvi;
			ecvi.m_OwnerComponent = component.get();
			ecvi.m_Item = CaptionElem;
			ecvi.m_ValueInterface = value.get();
			ecvi.m_EditableEntity = m_CurrentItem.m_EditableEntity;

			QStandardItem *ValueElem = new QStandardItem();
			ValueElem->setData(value->GetValue().c_str(), Qt::EditRole);
			auto einfoit = TypeInfoMap.find(ecvi.m_ValueInterface->GetTypeName());
			if (einfoit != TypeInfoMap.end()) {
				ValueElem->setData(einfoit->second->ToDisplayText(value->GetValue()).c_str(), Qt::DisplayRole);
			} else {
				ValueElem->setData(value->GetValue().c_str(), Qt::DisplayRole);
			}
				
			CaptionElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
			ValueElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);

			QList<QStandardItem*> cols;
			cols << CaptionElem;
			cols << ValueElem;
			Elem->appendRow(cols);
		}
	}
	m_Ui->treeViewDetails->collapseAll();
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::EntityClicked(const QModelIndex& index) {
	auto row = index.row();
	auto parent = index.parent();
	auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

	auto itemptr = m_EntityModel->itemFromIndex(selectedindex);
	if (!itemptr) {
		m_CurrentItem = EditableItemInfo();
	} else {
		m_CurrentItem = itemptr->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
	}

	RefreshDetails();
}

void EntityEditorWindow::ComponentClicked(const QModelIndex & index) {
	auto row = index.row();
	auto parent = index.parent();
	auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

	auto itemptr = m_ComponentModel->itemFromIndex(selectedindex);
	if (!itemptr) {
		m_CurrentComponent = EditableComponentValueInfo();
	} else {
		m_CurrentComponent = itemptr->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
	}
}

void EntityEditorWindow::ComponentChanged(QStandardItem * item) {
	if (!item)
		return;

	EditableComponentValueInfo info = item->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
	if (!info)
		return;

	std::string value = item->data(Qt::DisplayRole).toString().toLocal8Bit().constData();
	info.m_ValueInterface->SetValue(value);
	SetModiffiedState(true);
}

void EntityEditorWindow::EntityChanged(QStandardItem * item) {
	if (!item)
		return;

	EditableItemInfo info = item->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
	if (!info)
		return;

	auto value = item->data(Qt::DisplayRole).toString().toLocal8Bit().constData();
	info.m_EditableEntity->GetName() = value;
	SetModiffiedState(true);
}

void EntityEditorWindow::ComponentContextMenu(const QPoint & pos) {
	if (!m_CurrentComponent)
		return;

	QMenu menu(this);
	auto ComponentInfo = m_CurrentComponent;

	if (ComponentInfo.m_ValueInterface) {
		return;
	}

	menu.addAction("Move up", [this, ComponentInfo]() {
		ComponentInfo.m_EditableEntity->MoveUp(ComponentInfo.m_OwnerComponent);
		SetModiffiedState(true);
		RefreshDetails();
	});
	menu.addAction("Move down", [this, ComponentInfo]() {
		ComponentInfo.m_EditableEntity->MoveDown(ComponentInfo.m_OwnerComponent);
		SetModiffiedState(true);
		RefreshDetails();
	});
	menu.addSeparator();
	menu.addAction("Delete component", [this, ComponentInfo]() {
		ComponentInfo.m_EditableEntity->DeleteComponent(ComponentInfo.m_OwnerComponent);
		SetModiffiedState(true);
		RefreshDetails();
	});

	menu.exec(QCursor::pos());
}

void EntityEditorWindow::EntityContextMenu(const QPoint &pos) {
	if (!m_CurrentItem)
		return;

	QMenu menu(this);
	auto EntityInfo = m_CurrentItem;

	if (EntityInfo.m_Parent) {
		menu.addAction("Move up", [this, EntityInfo]() {
			EntityInfo.m_Parent->MoveUp(EntityInfo.m_EditableEntity);
			SetModiffiedState(true);
			Refresh();
		});
		menu.addAction("Move Down", [this, EntityInfo]() {
			EntityInfo.m_Parent->MoveDown(EntityInfo.m_EditableEntity);
			SetModiffiedState(true);
			Refresh();
		});
		menu.addSeparator();
	}

	if (EntityInfo.m_Parent) {
		menu.addAction("Delete child", [this, EntityInfo]() {
			if (!AskForPermission())
				return;
			EntityInfo.m_Parent->DeleteChild(EntityInfo.m_EditableEntity);
			SetModiffiedState(true);
			Refresh();
		});
	}

	menu.addAction("Add child", [this, EntityInfo]() {
		EntityInfo.m_EditableEntity->AddChild();
		SetModiffiedState(true);
		Refresh();
	});
	menu.addSeparator();

	menu.exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::ShowAddComponentMenu() {
	if (!m_CurrentItem)
		return;

	QMenu menu(this);
	auto CurrentItem = m_CurrentItem;

	for (auto &it : ComponentInfoMap) {
		auto &info = it.second;

		menu.addAction(info.m_Name.c_str(), [this, CurrentItem, &info]() {
			CurrentItem.m_EditableEntity->AddComponent(info.m_CID);
			SetModiffiedState(true);
			RefreshDetails();
		});
	}

	menu.exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::ProjectChanged(Module::SharedDataModule datamod) {
//	m_Module = datamod;
}

} //namespace EntityEditor 
} //namespace Editor 
} //namespace MoonGlare 
