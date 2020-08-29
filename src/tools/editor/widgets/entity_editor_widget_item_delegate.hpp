#pragma once

// #include "../iCustomEnum.h"

#include <QString>
#include <QStyledItemDelegate>
#include <custom_type_editor.hpp>
#include <editable_type.hpp>
#include <runtime_modules.h>

namespace MoonGlare::Tools::Editor::Widgets {

struct EntityEditorWidgetItemDelegate : public QStyledItemDelegate {
    constexpr static int kEditableFieldInfoRole = Qt::UserRole + 1000;

    struct EditableFieldInfo {
        std::shared_ptr<iEditableType> editable_type;
        std::shared_ptr<iAttributeBase> attribute;
        std::shared_ptr<iCustomTypeEditorFactory> editor_factory;
        operator bool() const { return attribute && editable_type; }
    };

    EntityEditorWidgetItemDelegate(SharedModuleManager moduleManager, QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
    SharedModuleManager GetModuleManager() const { return sharedModuleManager; }

private:
    // std::weak_ptr<QtShared::CustomEnumProvider> customEnumProvider;
    std::shared_ptr<iCustomTypeEditorProvider> custom_type_editor_provider;
    SharedModuleManager sharedModuleManager;
};

} // namespace MoonGlare::Tools::Editor::Widgets

Q_DECLARE_METATYPE(MoonGlare::Tools::Editor::Widgets::EntityEditorWidgetItemDelegate::EditableFieldInfo);
