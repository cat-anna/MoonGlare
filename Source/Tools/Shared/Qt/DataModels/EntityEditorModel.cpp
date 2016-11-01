/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER

#include <ui_EntityEditorModel.h>
#include <icons.h>

#include "EntityEditorModel.h"

#include <TypeEditor/x2cDataTree.h>
#include <TypeEditor/Structure.h>

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

//----------------------------------------------------------------------------------

EntityEditorModel::EntityEditorModel(QWidget * parent)
		: QWidget(parent) 
{
	m_Ui = std::make_unique<Ui::EntityEditorModel>();
	m_Ui->setupUi(this);

//	SetSettingID("EntityEditorModel");
//
	connect(m_Ui->pushButton, &QPushButton::clicked, this, &EntityEditorModel::ShowAddComponentMenu);
//
//	connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(Refresh()));
//
	m_EntityModel = std::make_unique<QStandardItemModel>();
	m_EntityModel->setHorizontalHeaderItem(0, new QStandardItem("Entity tree"));
	m_EntityModel->setHorizontalHeaderItem(1, new QStandardItem("Pattern URI"));
	connect(m_EntityModel.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(EntityChanged(QStandardItem *)));
	m_Ui->treeView->setModel(m_EntityModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	m_Ui->treeView->setColumnWidth(0, 200);
	m_Ui->treeView->setColumnWidth(1, 50);
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
	m_Ui->treeViewDetails->setItemDelegate(new TypeEditor::CustomEditorItemDelegate(this));
	m_Ui->treeViewDetails->setColumnWidth(0, 200);
	m_Ui->treeViewDetails->setColumnWidth(1, 100);
	m_Ui->treeViewDetails->setColumnWidth(2, 100);
	connect(m_Ui->treeViewDetails, SIGNAL(clicked(const QModelIndex &)), SLOT(ComponentClicked(const QModelIndex&)));
	connect(m_Ui->treeViewDetails, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ComponentContextMenu(const QPoint &)));

	m_AddComponentMenu = std::make_unique<QMenu>(this);
	std::unordered_map<std::string, QMenu*> SubMenus;
	for (auto &it : TypeEditor::ComponentInfo::GetComponents()) {
		auto info = it.second;

		QMenu *menu;

		std::string name = info->m_DisplayName;
		auto pos = name.find(".");
		if (pos != std::string::npos) {
			std::string menuname = name.substr(0, pos);
			name = name.substr(pos + 1);

			auto menuit = SubMenus.find(menuname);
			if (menuit == SubMenus.end()) {
				menu = m_AddComponentMenu->addMenu(menuname.c_str());
				SubMenus[menuname] = menu;
			} else {
				menu = menuit->second;
			}
		} else {
			menu = m_AddComponentMenu.get();
		}
			
		menu->addAction(info->m_DisplayName.c_str(), [this, info]() {
			m_CurrentItem.m_EditableEntity->AddComponent(info->m_CID);
			SetModiffiedState(true);
			RefreshDetails();
		});
	}
}

EntityEditorModel::~EntityEditorModel() {
	m_Ui.reset();
}

//----------------------------------------------------------------------------------

bool EntityEditorModel::DoSaveSettings(pugi::xml_node node) const {
	SaveState(node, m_Ui->splitter, "Splitter:State");
	SaveColumns(node, "treeView:Columns", m_Ui->treeView, 3);
	SaveColumns(node, "treeViewDetails:Columns", m_Ui->treeViewDetails, 3);
	return true;
}

bool EntityEditorModel::DoLoadSettings(const pugi::xml_node node) {
	LoadState(node, m_Ui->splitter, "Splitter:State");
	LoadColumns(node, "treeView:Columns", m_Ui->treeView, 3);
	LoadColumns(node, "treeViewDetails:Columns", m_Ui->treeViewDetails, 3);
	return true;
}

//----------------------------------------------------------------------------------

void EntityEditorModel::Refresh() {
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

		QStandardItem *URIElem = new QStandardItem(e->GetPatternURI().c_str());
		eii.m_PatternURIMode = true;
		URIElem->setData(QVariant::fromValue(eii), UserRoles::EditableItemInfo);

		QList<QStandardItem*> cols;
		cols << Elem;
		cols << URIElem;
		item->appendRow(cols);

		for (auto &child : e->GetChildrenList()) {
			buidFunc(child.get(), Elem);
		}
	};

	buidFunc(m_RootEntity.get(), root);
	m_Ui->treeView->setCurrentIndex(root->index());
	m_Ui->treeView->expandAll();
}

void EntityEditorModel::RefreshDetails() {
	m_ComponentModel->removeRows(0, m_ComponentModel->rowCount());
	m_CurrentComponent = EditableComponentValueInfo();
	if (!m_CurrentItem)
		return;

	QStandardItem *root = m_ComponentModel->invisibleRootItem();

	for (auto &component : m_CurrentItem.m_EditableEntity->GetComponentList()) {

		QStandardItem *Elem = new QStandardItem(component->GetName().c_str());
		Elem->setFlags(Elem->flags() & ~Qt::ItemIsEditable);

		{
			EditableComponentValueInfo ecvi;
			ecvi.m_OwnerComponent = component.get();
			ecvi.m_Item = Elem;
			ecvi.m_ValueInterface = nullptr;
			ecvi.m_EditableEntity = m_CurrentItem.m_EditableEntity;
			Elem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
		}
		{
			QList<QStandardItem*> cols;
			cols << Elem;
			root->appendRow(cols);
		}

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
			auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(ecvi.m_ValueInterface->GetTypeName());
			if (einfoit) {
				ValueElem->setData(einfoit->ToDisplayText(value->GetValue()).c_str(), Qt::DisplayRole);
			} else {
				ValueElem->setData(value->GetValue().c_str(), Qt::DisplayRole);
			}
				
			CaptionElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
			ValueElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
			ValueElem->setData(QVariant::fromValue(ecvi.m_ValueInterface), TypeEditor::CustomEditorItemDelegate::QtRoles::StructureValue);

			{
				QList<QStandardItem*> cols;
				cols << CaptionElem;
				cols << ValueElem;
				Elem->appendRow(cols);
			}
		}
	}
	m_Ui->treeViewDetails->collapseAll();
}

//----------------------------------------------------------------------------------

void EntityEditorModel::EntityClicked(const QModelIndex& index) {
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

void EntityEditorModel::ComponentClicked(const QModelIndex & index) {
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

void EntityEditorModel::ComponentChanged(QStandardItem * item) {
	if (!item)
		return;
	//EditableComponentValueInfo info = item->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
	//if (!info)
	//	return;
	//std::string value = item->data(Qt::DisplayRole).toString().toLocal8Bit().constData();
	//info.m_ValueInterface->SetValue(value);
	SetModiffiedState(true);
}

void EntityEditorModel::EntityChanged(QStandardItem * item) {
	if (!item)
		return;

	EditableItemInfo info = item->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
	if (!info)
		return;

	auto value = item->data(Qt::DisplayRole).toString().toLocal8Bit().constData();
	if(info.m_PatternURIMode)
		info.m_EditableEntity->SetPatternURI(std::move(value));
	else
		info.m_EditableEntity->GetName() = value;

	SetModiffiedState(true);
}

void EntityEditorModel::ComponentContextMenu(const QPoint & pos) {
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

void EntityEditorModel::EntityContextMenu(const QPoint &pos) {
	if (!m_CurrentItem)
		return;

	QMenu menu(this);
	auto EntityInfo = m_CurrentItem;

	bool Deletable = EntityInfo.m_EditableEntity->IsDeletable();
	bool Movable = EntityInfo.m_EditableEntity->IsDeletable();

	menu.addAction(ICON_16_ARROW_UP, "Move up", [this, EntityInfo]() {
		EntityInfo.m_Parent->MoveUp(EntityInfo.m_EditableEntity);
		SetModiffiedState(true);
		Refresh();
	})->setEnabled(Movable);
	menu.addAction(ICON_16_ARROW_DOWN, "Move down", [this, EntityInfo]() {
		EntityInfo.m_Parent->MoveDown(EntityInfo.m_EditableEntity);
		SetModiffiedState(true);
		Refresh();
	})->setEnabled(Movable);

	menu.addSeparator();

	menu.addAction(ICON_16_CREATE_RESOURCE, "Add child", [this, EntityInfo]() {
		EntityInfo.m_EditableEntity->AddChild();
		SetModiffiedState(true);
		Refresh();
	});

	menu.addAction(ICON_16_DELETE, "Delete child", [this, EntityInfo]() {
		if (!AskForPermission())
			return;
		EntityInfo.m_EditableEntity->Delete();
		SetModiffiedState(true);
		Refresh();
	})->setEnabled(Deletable);

	menu.addSeparator();

	menu.addAction(ICON_16_COPY, "Copy", this, &EntityEditorModel::CopyEntity);
	menu.addAction(ICON_16_PASE, "Paste", this, &EntityEditorModel::PasteEntity);
	menu.addAction(ICON_16_CUT, "Cut", this, &EntityEditorModel::CutEntity)->setEnabled(Deletable);

	menu.addSeparator();

	menu.exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

void EntityEditorModel::CutEntity() {
	if (!m_CurrentItem)
		return;

	std::string txt;
	if (!m_CurrentItem.m_EditableEntity->Serialize(txt)) {
		//todo
		return;
	}
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(txt.c_str());
	m_CurrentItem.m_EditableEntity->Delete();
}

void EntityEditorModel::CopyEntity() {
	if (!m_CurrentItem)
		return;

	std::string txt;
	m_CurrentItem.m_EditableEntity->Serialize(txt);
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(txt.c_str());
}

void EntityEditorModel::PasteEntity() {
	if (!m_CurrentItem)
		return;

	QClipboard *clipboard = QApplication::clipboard();
	std::string txt = clipboard->text().toLocal8Bit().constData();
	m_CurrentItem.m_EditableEntity->DeserializeToChild(txt);
	Refresh();
}

//----------------------------------------------------------------------------------

void EntityEditorModel::ShowAddComponentMenu() {
	if (!m_CurrentItem)
		return;

	m_AddComponentMenu->exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

#if 0
void EntityEditorModel::ProjectChanged(Module::SharedDataModule datamod) {
//	m_Module = datamod;
}

#endif

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

using Core::Component::ComponentID;
using Core::Component::ComponentID;

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

EditableEntity::EditableEntity(EditableEntity *Parent)
	: m_Parent(Parent) {}

EditableEntity::~EditableEntity() {}

bool EditableEntity::Read(pugi::xml_node node, const char *NodeName) {
	return Read(node.child(NodeName ? NodeName : "Entity"));
}

bool EditableEntity::Read(pugi::xml_node node) {
	m_Name = node.attribute("Name").as_string("");
	m_PatternURI = node.attribute("Pattern").as_string("");

	for (auto it = node.first_child(); it; it = it.next_sibling()) {
		const char *nodename = it.name();
		auto hash = Space::Utils::MakeHash32(nodename);

		switch (hash) {

		case "Component"_Hash32:
		{
			auto child = EditableComponent::CreateComponent(this, it);
			if (!child) {
				//TODO: log sth
				continue;
			}
			m_Components.emplace_back(std::move(child));
			break;
		}

		//case "Entity"_Hash32:
		//{
		//auto pattern = it.attribute("Pattern").as_string(nullptr);
		//if (pattern) {
		//	FileSystem::XMLFile xdoc;
		//	Entity child;
		//	std::string paturi = pattern;
		//	if (!GetFileSystem()->OpenXML(xdoc, paturi, DataPath::URI)) {
		//		AddLogf(Error, "Failed to open pattern: %s", pattern);
		//		continue;
		//	}
		//
		//	auto c = BuildChild(Owner, xdoc->document_element(), child);
		//	if (c == 0) {
		//		AddLogf(Error, "Failed to load child!");
		//		continue;
		//	}
		//	count += c;
		//	continue;
		//}
		//}
		//no break;
		//[[fallthrough]]
		case "Entity"_Hash32:
		case "Child"_Hash32:
		{
			UniqueEditableEntity child(new EditableEntity(this));
			if (!child->Read(it)) {
				//TODO: log sth
				continue;
			}
			m_Children.emplace_back(std::move(child));
		}
		continue;
		default:
			AddLogf(Warning, "Unknown node: %s", nodename);
			continue;
		}
	}

	return true;
}

bool EditableEntity::Write(pugi::xml_node node, const char *NodeName) {
	return Write(node.append_child(NodeName ? NodeName : "Entity"));
}

bool EditableEntity::Write(pugi::xml_node node) {
	if (!m_Name.empty())
		node.append_attribute("Name") = m_Name.c_str();
	if (!m_PatternURI.empty())
		node.append_attribute("Pattern") = m_PatternURI.c_str();

	bool ret = true;
	for (auto &it : m_Components) {
		auto cnode = node.append_child("Component");
		cnode.append_attribute("Name") = it->GetName().c_str();
		ret = ret && it->Write(cnode);
	}
	for (auto &it : m_Children) {
		ret = ret && it->Write(node, "Entity");
	}
	return ret;
}

void EditableEntity::MoveDown(EditableComponent *c) {
	auto idx = Find(c);
	if (idx < 0)
		return;
	if (idx + 1 >= m_Components.size())
		return;
	m_Components[idx].swap(m_Components[idx + 1]);
}

void EditableEntity::MoveUp(EditableComponent *c) {
	auto idx = Find(c);
	if (idx <= 0)
		return;
	m_Components[idx].swap(m_Components[idx - 1]);
}

void EditableEntity::MoveDown(EditableEntity * c) {
	auto idx = Find(c);
	if (idx < 0)
		return;
	if (idx + 1 >= m_Children.size())
		return;
	m_Children[idx].swap(m_Children[idx + 1]);
}

void EditableEntity::MoveUp(EditableEntity * c) {
	auto idx = Find(c);
	if (idx <= 0)
		return;
	m_Children[idx].swap(m_Children[idx - 1]);
}

EditableEntity *EditableEntity::AddChild() {
	UniqueEditableEntity child(new EditableEntity(this));
	child->GetName() = "child";
	auto rawptr = child.get();
	m_Children.emplace_back(std::move(child));
	return rawptr;
}

void EditableEntity::DeleteChild(EditableEntity * c) {
	auto idx = Find(c);
	if (idx < 0)
		return;

	m_Children[idx].reset();
	for (; idx < (int)m_Children.size() - 1; ++idx)
		m_Children[idx].swap(m_Children[idx + 1]);;

	m_Children.pop_back();
}

EditableComponent* EditableEntity::AddComponent(Core::ComponentID cid) {
	auto child = EditableComponent::CreateComponent(this, cid);
	if (!child) {
		//TODO: log sth
		return nullptr;
	}
	auto rawptr = child.get();
	m_Components.emplace_back(std::move(child));
	return rawptr;
}

void EditableEntity::DeleteComponent(EditableComponent * c) {
	auto idx = Find(c);
	if (idx < 0)
		return;

	m_Components[idx].reset();
	for (; idx < (int)m_Components.size() - 1; ++idx)
		m_Components[idx].swap(m_Components[idx + 1]);;
	m_Components.pop_back();
}

void EditableEntity::Clear() {
	m_Children.clear();
	m_Components.clear();
	m_Name = "Entity";
	m_PatternURI.clear();
}

bool EditableEntity::Serialize(std::string &out) {
	pugi::xml_document xdoc;
	if (!Write(xdoc, "Entity"))
		return false;
	std::stringstream ss;
	xdoc.save(ss);
	out = ss.str();
	return true;
}

bool EditableEntity::Deserialize(std::string &out) {
	pugi::xml_document xdoc;
	if (!xdoc.load_string(out.c_str())) {
		return false;
	}
	return Read(xdoc.document_element());
}

bool EditableEntity::DeserializeToChild(std::string &out) {
	auto ch = AddChild();
	if (ch->Deserialize(out))
		return true;
	DeleteChild(ch);
	return false;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

UniqueEditableComponent EditableComponent::CreateComponent(EditableEntity *Parent, pugi::xml_node node) {
	ComponentID cid = ComponentID::Invalid;

	auto idxml = node.attribute("Id");
	if (idxml) {
		cid = static_cast<ComponentID>(idxml.as_uint(0));
	} else {
		auto namexml = node.attribute("Name");
		if (!namexml) {
			AddLogf(Error, "Component definition without id or name!");
			return nullptr;
		}
		auto ci = TypeEditor::ComponentInfo::GetComponentInfo(namexml.as_string(""));
		if (ci)
			cid = ci->m_CID;
		if (cid == (ComponentID)ComponentID::Invalid) {
			AddLogf(Error, "Unknown component name: %s", namexml.as_string(""));
		}
	}

	if (cid == (ComponentID)ComponentID::Invalid)
		return nullptr;

	auto ret = CreateComponent(Parent, cid);
	if (!ret) {
		//already logged
		return nullptr;
	}

	if (!ret->Read(node)) {
		//TODO: log sth
		return nullptr;
	}

	return std::move(ret);
}

UniqueEditableComponent EditableComponent::CreateComponent(EditableEntity *Parent, MoonGlare::Core::ComponentID cid) {
	auto ci = TypeEditor::ComponentInfo::GetComponentInfo(cid);
	if (!ci) {
		AddLogf(Error, "Unknown component id: %d", (int)cid);
		return nullptr;
	}

	auto x2cs = ci->m_EntryStructure->m_CreateFunc(nullptr, nullptr);
	return std::make_unique<EditableComponent>(Parent, ci, std::move(x2cs));
}

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare