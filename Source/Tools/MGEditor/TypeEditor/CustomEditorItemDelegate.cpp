#include PCH_HEADER

#include "CustomEditorItemDelegate.h"
#include "CustomEnum.h"
#include "CustomType.h"

namespace MoonGlare::TypeEditor {

CustomEditorItemDelegate::CustomEditorItemDelegate(SharedModuleManager moduleManager, QWidget *parent)
    : QStyledItemDelegate(parent), sharedModuleManager(moduleManager) {

    if (moduleManager) {
        customEnumProvider = moduleManager->QueryModule<QtShared::CustomEnumProvider>();
    }
}

QWidget *CustomEditorItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const {
    auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue *>();
    if (vinfo) {
        std::string fullName = vinfo->GetFullName();
        if (auto cep = customEnumProvider.lock(); cep) {
            auto einfo = cep->GetEnum(fullName);
            if (einfo) {
                return new CustomEnum(parent, einfo);
            }
        }

        if (auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(fullName); einfoit) {
            auto e = einfoit->CreateEditor(parent);
            e->SetModuleManager(GetModuleManager());
            e->SetDataSource([index](int role) -> QVariant { return index.data(role); });
            return e->GetWidget();
        }
        if (auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(vinfo->GetTypeName()); einfoit) {
            auto e = einfoit->CreateEditor(parent);
            e->SetModuleManager(GetModuleManager());
            e->SetDataSource([index](int role) -> QVariant { return index.data(role); });
            return e->GetWidget();
        }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
};

void CustomEditorItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue *>();
    auto *cte = dynamic_cast<TypeEditor::CustomTypeEditor *>(editor);
    if (vinfo && cte) {
        cte->SetValue(vinfo->GetValue());
    }
    return QStyledItemDelegate::setEditorData(editor, index);
};

void CustomEditorItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index) const {
    auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue *>();
    std::optional<std::string> displayText;
    if (vinfo) {
        auto *cte = dynamic_cast<TypeEditor::CustomTypeEditor *>(editor);
        if (cte) {
            auto data = cte->GetValue();
            displayText = data;
            vinfo->SetValue(data);
        } else {
            std::string value = index.data(Qt::DisplayRole).toString().toLocal8Bit().constData();
            displayText = value;
            vinfo->SetValue(value);
        }
    }
    QStyledItemDelegate::setModelData(editor, model, index);
    if (displayText.has_value())
        model->setData(index, displayText.value().c_str(), Qt::DisplayRole);
};

} // namespace MoonGlare::TypeEditor
