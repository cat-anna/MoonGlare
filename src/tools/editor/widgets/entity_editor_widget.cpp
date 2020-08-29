#include "entity_editor_widget.hpp"
#include "entity_editor_widget_item_delegate.hpp"
#include <QClipboard>
#include <qt_gui_icons.h>
#include <ui_entity_editor_widget.h>

namespace MoonGlare::Tools::Editor::Widgets {

//---------------------------------\-------------------------------------------------

EntityEditorWidget::EntityEditorWidget(QWidget *parent) : QWidget(parent) {
    ui = std::make_unique<Ui::EntityEditorWidget>();
    ui->setupUi(this);

    //	SetSettingID("EntityEditorWidget");

    connect(ui->pushButton, &QPushButton::clicked, this, &EntityEditorWidget::ShowAddComponentMenu);

    entity_model = std::make_unique<QStandardItemModel>();
    entity_model->setHorizontalHeaderItem(0, new QStandardItem("Entity tree"));
    entity_model->setHorizontalHeaderItem(1, new QStandardItem("Pattern URI"));
    connect(entity_model.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(EntityChanged(QStandardItem *)));
    ui->treeView->setModel(entity_model.get());
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setColumnWidth(0, 200);
    ui->treeView->setColumnWidth(1, 50);
    connect(ui->treeView, SIGNAL(clicked(const QModelIndex &)), SLOT(EntityClicked(const QModelIndex &)));
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(EntityContextMenu(const QPoint &)));

    component_model = std::make_unique<QStandardItemModel>();
    component_model->setHorizontalHeaderItem(0, new QStandardItem("Values tree"));
    component_model->setHorizontalHeaderItem(1, new QStandardItem("Values"));
    component_model->setHorizontalHeaderItem(2, new QStandardItem("Description"));
    connect(component_model.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(ComponentChanged(QStandardItem *)));
    ui->treeViewDetails->setModel(component_model.get());
    ui->treeViewDetails->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeViewDetails->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeViewDetails->setColumnWidth(0, 200);
    ui->treeViewDetails->setColumnWidth(1, 100);
    ui->treeViewDetails->setColumnWidth(2, 100);
    // ui->treeViewDetails->setItemDelegate(new EntityEditorWidgetItemDelegate(moduleManager, this));

    connect(ui->treeViewDetails, SIGNAL(clicked(const QModelIndex &)), SLOT(ComponentClicked(const QModelIndex &)));
    connect(ui->treeViewDetails, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(ComponentContextMenu(const QPoint &)));
}

EntityEditorWidget::~EntityEditorWidget() {
    entity_model.reset();
    component_model.reset();
    ui.reset();
}

void EntityEditorWidget::SetModuleManager(SharedModuleManager mm) {
    shared_module_manager.swap(mm);

    // iChangeContainer::SetModuleManager(moduleManager);
    ui->treeViewDetails->setItemDelegate(new EntityEditorWidgetItemDelegate(shared_module_manager, this));
    custom_type_editor_provider = shared_module_manager->QueryModule<iCustomTypeEditorProvider>();

    add_component_menu = std::make_unique<QMenu>(this);
    std::unordered_map<std::string, QMenu *> sub_menus;
    for (auto &info : shared_module_manager->QueryModule<iComponentRegister>()->GetComponents()) {
        QMenu *menu;

        std::string name = info->name;
        auto pos = name.find(".");
        if (pos != std::string::npos) {
            std::string menuname = name.substr(0, pos);
            name = name.substr(pos + 1);

            auto menuit = sub_menus.find(menuname);
            if (menuit == sub_menus.end()) {
                menu = add_component_menu->addMenu(menuname.c_str());
                sub_menus[menuname] = menu;
            } else {
                menu = menuit->second;
            }
        } else {
            menu = add_component_menu.get();
        }

        menu->addAction(name.c_str(), [this, info]() {
            current_item.entity->AddComponent(info->id);
            Changed();
            RefreshDetails();
        });
    }
}

//----------------------------------------------------------------------------------

void EntityEditorWidget::Refresh() {
    entity_model->removeRows(0, entity_model->rowCount());
    current_item = EditableItemInfo();

    if (!editable_entity)
        return;

    std::function<void(std::shared_ptr<EditableEntity>, QStandardItem *)> buidFunc;
    buidFunc = [&buidFunc](std::shared_ptr<EditableEntity> e, QStandardItem *item) {
        auto name = e->GetName().value_or("");
        if (!e->IsEnabled()) {
            name = "[D] " + name;
        }
        QStandardItem *view_item = new QStandardItem(name.c_str());

        EditableItemInfo eii;
        eii.entity = e;
        eii.view_item = view_item;
        eii.parent = e->GetParent();
        view_item->setData(QVariant::fromValue(eii), UserRoles::EditableItemInfo);

        QStandardItem *URIElem = new QStandardItem(); //(e->GetPatternURI().c_str());
        eii.reference_mode = false;
        URIElem->setData(QVariant::fromValue(eii), UserRoles::EditableItemInfo);

        QList<QStandardItem *> cols;
        cols << view_item;
        cols << URIElem;
        item->appendRow(cols);

        e->ForEachChildren([&](auto child) { buidFunc(child, view_item); });
    };

    QStandardItem *root = entity_model->invisibleRootItem();
    buidFunc(editable_entity, root);
    ui->treeView->setCurrentIndex(root->index());
    ui->treeView->expandAll();
}

void EntityEditorWidget::RefreshDetails() {
    component_model->removeRows(0, component_model->rowCount());
    current_component = EditableComponentValueInfo();
    if (!current_item)
        return;

    QStandardItem *root = component_model->invisibleRootItem();

    current_item.entity->ForEachComponent([&](auto cid, auto component) {
        auto info = component->GetComponentInfo();
        std::string name = info->name;

        if (!component->IsEnabled()) {
            name = "[D] " + name;
        }
        // if (!component->active) {
        //     name = "[Inactive] " + name;
        // }

        QStandardItem *view_item = new QStandardItem(name.c_str());
        view_item->setFlags(view_item->flags() & ~Qt::ItemIsEditable);
        view_item->setData(ICON_16_COMPONENT, Qt::DecorationRole);

        {
            EditableComponentValueInfo ecvi;
            ecvi.component = component;
            ecvi.view_item = view_item;
            ecvi.entity = current_item.entity;
            view_item->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
        }

        {
            QList<QStandardItem *> cols;
            cols << view_item;
            root->appendRow(cols);
        }

        std::function<void(std::shared_ptr<iEditableType>, QStandardItem *)> type_enumerator;
        type_enumerator = [&](std::shared_ptr<iEditableType> type, QStandardItem *parent_item) {
            for (auto &item : type->GetAttributeInfo()) {
                QStandardItem *caption_item = new QStandardItem(QString::fromStdString(item->GetName()));
                caption_item->setFlags(caption_item->flags() & ~Qt::ItemIsEditable);
                QStandardItem *value_item = nullptr;
                if (item->IsBasicType()) {
                    EditableComponentValueInfo ecvi{};
                    ecvi.component = component;
                    ecvi.view_item = caption_item;
                    ecvi.editable_type = type;
                    ecvi.attribute = item;
                    ecvi.entity = current_item.entity;

                    value_item = new QStandardItem();
                    auto editor_info = custom_type_editor_provider->GetEditorFactory(item->GetTypeName());
                    try {
                        auto value_variant = type->GetValue(item);

                        if (editor_info) {
                            value_item->setData(QString::fromStdString(editor_info->ToDisplayText(value_variant)),
                                                Qt::DisplayRole);
                        } else {
                            auto value = VariantArgumentMap::CastVariant<std::string>(value_variant);
                            value_item->setData(QString::fromStdString(value), Qt::EditRole);
                            value_item->setData(QString::fromStdString(value), Qt::DisplayRole);
                        }
                    } catch (const std::exception &e) {
                        AddLogf(Error, "Failed to get component value as string: %s", e.what());
                    }

                    caption_item->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
                    value_item->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);

                    EntityEditorWidgetItemDelegate::EditableFieldInfo efi;
                    efi.attribute = item;
                    efi.editable_type = type;
                    efi.editor_factory = editor_info;
                    value_item->setData(QVariant::fromValue(efi),
                                        EntityEditorWidgetItemDelegate::kEditableFieldInfoRole);
                }

                {
                    QList<QStandardItem *> cols;
                    cols << caption_item;
                    if (value_item != nullptr) {
                        cols << value_item;
                    }
                    parent_item->appendRow(cols);
                }

                if (!item->IsBasicType()) {
                    type_enumerator(type->GetField(item), caption_item);
                }
            }
        };

        type_enumerator(component->GetComponentData(), view_item);
    });

    ui->treeViewDetails->collapseAll();
}

//----------------------------------------------------------------------------------

void EntityEditorWidget::EntityClicked(const QModelIndex &index) {
    auto row = index.row();
    auto parent = index.parent();
    auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

    auto itemptr = entity_model->itemFromIndex(selectedindex);
    if (!itemptr) {
        current_item = EditableItemInfo();
    } else {
        current_item = itemptr->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
    }

    RefreshDetails();
}

void EntityEditorWidget::ComponentClicked(const QModelIndex &index) {
    auto row = index.row();
    auto parent = index.parent();
    auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

    auto itemptr = component_model->itemFromIndex(selectedindex);
    if (!itemptr) {
        current_component = EditableComponentValueInfo();
    } else {
        current_component = itemptr->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
    }
}

void EntityEditorWidget::ComponentChanged(QStandardItem *item) {
    if (!item)
        return;
    EditableComponentValueInfo info =
        item->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
    if (!info)
        return;
    Refresh();
    Changed();
}

void EntityEditorWidget::EntityChanged(QStandardItem *item) {
    if (!item)
        return;

    EditableItemInfo info = item->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
    if (!info)
        return;

    auto value = item->data(Qt::DisplayRole).toString().toStdString();
    if (info.reference_mode) {
        // info.entity->SetPatternURI(std::move(value));
    } else {
        info.entity->SetName(value);
    }

    Changed();
    RefreshDetails();
}

void EntityEditorWidget::ComponentContextMenu(const QPoint &pos) {
    if (!current_component.component)
        return;

    QMenu menu(this);
    auto ComponentInfo = current_component;

    menu.addSeparator();

    menu.addAction(ComponentInfo.component->IsEnabled() ? "Disable" : "Enable", [this, ComponentInfo]() {
        ComponentInfo.component->SetEnabled(!ComponentInfo.component->IsEnabled());
        Refresh();
        RefreshDetails();
        Changed();
    });

    // if (ComponentInfo.component->active) {
    //     menu.addAction("Deactivate", [this, ComponentInfo]() {
    //         ComponentInfo.component->active = false;
    //         RefreshDetails();
    //         Changed();
    //     });
    // } else {
    //     menu.addAction("Activate", [this, ComponentInfo]() {
    //         ComponentInfo.component->active = true;
    //         RefreshDetails();
    //         Changed();
    //     });
    // }

    menu.addSeparator();

    menu.addAction("Delete component", [this, ComponentInfo]() {
        ComponentInfo.entity->DeleteComponent(ComponentInfo.component);
        Changed();
        Refresh();
        RefreshDetails();
    });

    menu.exec(QCursor::pos());
}

void EntityEditorWidget::EntityContextMenu(const QPoint &pos) {
    if (!current_item)
        return;

    QMenu menu(this);
    auto EntityInfo = current_item;

    bool Deletable = EntityInfo.entity->IsDeletable();
    bool Movable = EntityInfo.entity->IsDeletable();

    menu.addAction(ICON_16_ARROW_UP, "Move up",
                   [this, EntityInfo]() {
                       EntityInfo.parent->MoveUp(EntityInfo.entity);
                       Changed();
                       Refresh();
                   })
        ->setEnabled(Movable);
    menu.addAction(ICON_16_ARROW_DOWN, "Move down",
                   [this, EntityInfo]() {
                       EntityInfo.parent->MoveDown(EntityInfo.entity);
                       Changed();
                       Refresh();
                   })
        ->setEnabled(Movable);

    menu.addSeparator();

    menu.addAction(ICON_16_CREATE_RESOURCE, "Add child", [this, EntityInfo]() {
        EntityInfo.entity->AddChild();
        Changed();
        Refresh();
    });

    menu.addAction(ICON_16_DELETE, "Delete child",
                   [this, EntityInfo]() {
                       if (!AskForConfirmation()) {
                           return;
                       }
                       EntityInfo.parent->DeleteChild(EntityInfo.entity);
                       Changed();
                       current_item = {};
                       Refresh();
                   })
        ->setEnabled(Deletable);

    menu.addSeparator();

    menu.addAction(ICON_16_COPY, "Copy", this, &EntityEditorWidget::CopyEntity);
    menu.addAction(ICON_16_PASE, "Paste", this, &EntityEditorWidget::PasteEntity);
    menu.addAction(ICON_16_CUT, "Cut", this, &EntityEditorWidget::CutEntity)->setEnabled(Deletable);

    menu.addSeparator();

    menu.addAction(EntityInfo.entity->IsEnabled() ? "Disable" : "Enable", [this, EntityInfo]() {
        EntityInfo.entity->SetEnabled(!EntityInfo.entity->IsEnabled());
        Refresh();
        Changed();
    });

    menu.exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

void EntityEditorWidget::CutEntity() {
    if (!current_item)
        return;

    std::string txt = current_item.entity->SerializeToJson();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(txt.c_str());
    current_item.parent->DeleteChild(current_item.entity);
    current_item = {};
    Changed();
}

void EntityEditorWidget::CopyEntity() {
    if (!current_item)
        return;

    std::string txt = current_item.entity->SerializeToJson();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(txt.c_str());
}

void EntityEditorWidget::PasteEntity() {
    if (!current_item)
        return;

    QClipboard *clipboard = QApplication::clipboard();
    std::string txt = clipboard->text().toStdString();
    current_item.entity->DeserializeToChild(txt);
    Refresh();
    Changed();
}

//----------------------------------------------------------------------------------

void EntityEditorWidget::ShowAddComponentMenu() {
    if (!current_item)
        return;

    if (!add_component_menu) {
        return;
    }

    add_component_menu->exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

} // namespace MoonGlare::Tools::Editor::Widgets
