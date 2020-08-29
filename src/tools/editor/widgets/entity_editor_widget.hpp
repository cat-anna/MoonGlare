#pragma once

#include <QMenu>
#include <QStandardItemModel>
#include <QWidget>
#include <custom_type_editor.hpp>
#include <editable_entity.hpp>
#include <editable_type.hpp>
#include <memory>
#include <runtime_modules.h>
#include <string>
#include <user_questions.hpp>

namespace Ui {
class EntityEditorWidget;
}

namespace MoonGlare::Tools::Editor::Widgets {

class EntityEditorWidget : public QWidget, public UserQuestions {
    Q_OBJECT;

public:
    EntityEditorWidget(QWidget *parent);
    virtual ~EntityEditorWidget();

    struct UserRoles {
        enum {
            EditableItemInfo = Qt::UserRole + 1,
            EditableComponentValueInfo,
        };
    };

    struct EditableComponentValueInfo {
        std::shared_ptr<EditableComponent> component;
        std::shared_ptr<iEditableType> editable_type;
        std::shared_ptr<iAttributeBase> attribute;
        std::shared_ptr<EditableEntity> entity;
        QStandardItem *view_item = nullptr;

        operator bool() const { return view_item && component && attribute && entity && editable_type; }
    };

    struct EditableItemInfo {
        QStandardItem *view_item = nullptr;
        std::shared_ptr<EditableEntity> entity;
        std::shared_ptr<EditableEntity> parent;
        bool reference_mode = false;

        operator bool() const { return view_item && entity; }
    };

    void SetEntity(std::shared_ptr<EditableEntity> new_root) {
        editable_entity.swap(new_root);
        Refresh();
    }

    void SetModuleManager(SharedModuleManager mm);

signals:
    void Changed();
public slots:
    void Refresh();
    void RefreshDetails();
protected slots:
    void EntityClicked(const QModelIndex &index);
    void ComponentClicked(const QModelIndex &index);
    void ComponentChanged(QStandardItem *item);
    void EntityChanged(QStandardItem *item);
    void ComponentContextMenu(const QPoint &);
    void EntityContextMenu(const QPoint &);

    void CutEntity();
    void CopyEntity();
    void PasteEntity();

    void ShowAddComponentMenu();

private:
    std::unique_ptr<Ui::EntityEditorWidget> ui;
    std::unique_ptr<QStandardItemModel> entity_model;
    std::unique_ptr<QStandardItemModel> component_model;
    std::unique_ptr<QMenu> add_component_menu;

    EditableItemInfo current_item;
    EditableComponentValueInfo current_component;

    std::shared_ptr<EditableEntity> editable_entity;

    SharedModuleManager shared_module_manager;
    std::shared_ptr<iCustomTypeEditorProvider> custom_type_editor_provider;

    // std::string m_CurrentPatternFile;
};

} // namespace MoonGlare::Tools::Editor::Widgets

Q_DECLARE_METATYPE(MoonGlare::Tools::Editor::Widgets::EntityEditorWidget::EditableItemInfo);
Q_DECLARE_METATYPE(MoonGlare::Tools::Editor::Widgets::EntityEditorWidget::EditableComponentValueInfo);
