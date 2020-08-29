#include "entity_editor_widget_item_delegate.hpp"
#include <QLineEdit>
#include <optional>
#include <string>

namespace MoonGlare::Tools::Editor::Widgets {

EntityEditorWidgetItemDelegate::EntityEditorWidgetItemDelegate(SharedModuleManager moduleManager, QWidget *parent)
    : QStyledItemDelegate(parent), sharedModuleManager(moduleManager) {

    if (moduleManager) {
        //     customEnumProvider = moduleManager->QueryModule<QtShared::CustomEnumProvider>();
        custom_type_editor_provider = moduleManager->QueryModule<iCustomTypeEditorProvider>();
    }
}

QWidget *EntityEditorWidgetItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                      const QModelIndex &index) const {
    auto field_info = index.data(kEditableFieldInfoRole).value<EditableFieldInfo>();
    if (field_info) {
        // std::string fullName = vinfo->GetFullName();
        // if (auto cep = customEnumProvider.lock(); cep) {
        //     auto einfo = cep->GetEnum(fullName);
        //     if (einfo) {
        //         return new CustomEnum(parent, einfo);
        //     }
        // }

        if (field_info.editor_factory) {
            auto editor = field_info.editor_factory->CreateEditor(parent);
            editor->SetModuleManager(GetModuleManager());
            editor->SetDataSource([index](int role) -> QVariant { return index.data(role); });
            return editor->GetWidget();
        }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
};

void EntityEditorWidgetItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    auto field_info = index.data(kEditableFieldInfoRole).value<EditableFieldInfo>();
    auto *cte = dynamic_cast<iCustomTypeEditor *>(editor);
    if (field_info && cte != nullptr) {
        cte->SetValue(field_info.editable_type->GetValue(field_info.attribute));
    }
    return QStyledItemDelegate::setEditorData(editor, index);
};

void EntityEditorWidgetItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                                  const QModelIndex &index) const {
    auto field_info = index.data(kEditableFieldInfoRole).value<EditableFieldInfo>();
    std::optional<std::string> displayText;
    if (field_info) {
        auto *cte = dynamic_cast<iCustomTypeEditor *>(editor);
        if (cte != nullptr) {
            auto data = cte->GetValue();
            displayText = field_info.editor_factory->ToDisplayText(data);
            field_info.editable_type->SetValue(field_info.attribute, data);
        } else {
            auto *line_edit = dynamic_cast<QLineEdit *>(editor);
            std::string value;
            if (line_edit != nullptr) {
                value = line_edit->text().toStdString();
            } else {
                value = index.data(Qt::EditRole).toString().toStdString();
            }
            displayText = value;
            field_info.editable_type->SetValue(field_info.attribute, VariantArgumentMap::VariantType(value));
        }
    }
    QStyledItemDelegate::setModelData(editor, model, index);
    if (displayText.has_value()) {
        model->setData(index, QString::fromStdString(displayText.value()), Qt::DisplayRole);
    }
};

} // namespace MoonGlare::Tools::Editor::Widgets
