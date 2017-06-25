#include PCH_HEADER


#include "CustomEditorItemDelegate.h"
#include "CustomType.h"
#include "CustomEnum.h"

namespace MoonGlare::TypeEditor {

CustomEditorItemDelegate::CustomEditorItemDelegate(QtShared::SharedModuleManager moduleManager, QWidget *parent)
    : QStyledItemDelegate(parent) {

    if (moduleManager) {
        customEnumProvider = moduleManager->QuerryModule<QtShared::CustomEnumProvider>();
    }
}

QWidget *CustomEditorItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue*>();
    if (vinfo) {
        auto cep = customEnumProvider.lock();
        if (cep) {
            std::string enumname = vinfo->GetFullName();
            auto einfo = cep->GetEnum(enumname);
            if (einfo) {
                return new CustomEnum(parent, einfo);
            }
        }

        auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(vinfo->GetTypeName());
        if (einfoit) {
            return einfoit->CreateEditor(parent)->GetWidget();
        }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
};

void CustomEditorItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue*>();
    auto *cte = dynamic_cast<TypeEditor::CustomTypeEditor*>(editor);
    if (vinfo && cte) {
        cte->SetValue(vinfo->GetValue());
    }
    return QStyledItemDelegate::setEditorData(editor, index);
};

void CustomEditorItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QStyledItemDelegate::setModelData(editor, model, index);
    auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue*>();
    if (vinfo) {
        auto *cte = dynamic_cast<TypeEditor::CustomTypeEditor*>(editor);
        if (cte) {
            vinfo->SetValue(cte->GetValue());
        }
        else {
            std::string value = index.data(Qt::DisplayRole).toString().toLocal8Bit().constData();
            vinfo->SetValue(value);
        }
    }
};

} //namespace MoonGlare::TypeEditor
