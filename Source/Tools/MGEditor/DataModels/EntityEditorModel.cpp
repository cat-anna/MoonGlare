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
#include <TypeEditor/CustomEditorItemDelegate.h>

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
    m_Ui->treeViewDetails->setItemDelegate(new TypeEditor::CustomEditorItemDelegate(moduleManager, this));
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
            m_CurrentItem.m_EditableEntity->AddComponent(info->m_Name);
            SetModiffiedState(true);
            RefreshDetails();
        });
    }
}

EntityEditorModel::~EntityEditorModel() {
    m_Ui.reset();
}

void EntityEditorModel::SetModuleManager(SharedModuleManager mm) {
    moduleManager.swap(mm);
    m_Ui->treeViewDetails->setItemDelegate(new TypeEditor::CustomEditorItemDelegate(moduleManager, this));
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

        std::string name = e->GetName();
        if (!e->enabled) {
            name = "[Disabled] " + name;
        }
        QStandardItem *Elem = new QStandardItem(name.c_str());

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

        std::string name = component->GetName();
        if (!component->enabled) {
            name = "[Disabled] " + name;
        }
        if (!component->active) {
            name = "[Inactive] " + name;
        }
        QStandardItem *Elem = new QStandardItem(name.c_str());
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

            //AddLogf(Error, "NAME: %s.%s", component->GetName().c_str(), value->GetName().c_str());

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

    if (ComponentInfo.m_OwnerComponent->enabled) {
        menu.addAction("Disable", [this, ComponentInfo]() {
            ComponentInfo.m_OwnerComponent->enabled = false;
            RefreshDetails();
            SetModiffiedState(true);
        });
    }
    else {
        menu.addAction("Enable", [this, ComponentInfo]() {
            ComponentInfo.m_OwnerComponent->enabled = true;
            RefreshDetails();
            SetModiffiedState(true);
        });
    }
    if (ComponentInfo.m_OwnerComponent->active) {
        menu.addAction("Deactivate", [this, ComponentInfo]() {
            ComponentInfo.m_OwnerComponent->active = false;
            RefreshDetails();
            SetModiffiedState(true);
        });
    } else {
        menu.addAction("Activate", [this, ComponentInfo]() {
            ComponentInfo.m_OwnerComponent->active = true;
            RefreshDetails();
            SetModiffiedState(true);
        });
    }

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

    if (EntityInfo.m_EditableEntity->enabled) {
        menu.addAction("Disable", [this, EntityInfo]() {
            EntityInfo.m_EditableEntity->enabled = false;
            Refresh();
            SetModiffiedState(true);
        });
    }
    else {
        menu.addAction("Enable", [this, EntityInfo]() {
            EntityInfo.m_EditableEntity->enabled = true;
            Refresh();
            SetModiffiedState(true);
        });
    }

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

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare
