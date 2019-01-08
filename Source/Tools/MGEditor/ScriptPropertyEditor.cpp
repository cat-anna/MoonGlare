#include PCH_HEADER

#include "ScriptProperty.h"
#include "ScriptPropertyEditor.h"

#include <DataModels/EditableEntity.h>

#include <icons.h>

namespace MoonGlare::Editor {

ScriptPropertyEditor::ScriptPropertyEditor(QWidget *Parent): TreeViewDialog(Parent, "ScriptPropertyEditor") {

    model = std::make_unique<QStandardItemModel>();

    auto tv = GetTreeView();
    tv->setColumnWidth(0, 400);
    tv->setColumnWidth(1, 200);
    tv->setModel(model.get());
    setWindowTitle("Script property editor");

    tv->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tv, &QTreeView::customContextMenuRequested, [this](const QPoint &pos) {
        if (!instance)
            return;

        QMenu menu;
        menu.addAction("Add custom property", [&]() {
            auto csp = std::make_shared<CustomScriptProperty>();
            instance->customValues[csp] = "";

            QList<QStandardItem*> cols;
            auto title = new QStandardItem("");
            title->setEditable(true);
            title->setData(QVariant::fromValue(csp), PropertyCustomTitleRole);
            title->setIcon(QIcon(ICON_16_BLUE_PLUS));
            cols << title;

            auto valueItem = new QStandardItem("");
            valueItem->setEditable(true);
            valueItem->setData(QVariant::fromValue(csp), PropertyCustomValueRole);
            cols << valueItem;
            auto root = model->invisibleRootItem();
            root->appendRow(cols);
        });

        menu.exec(pos);
    });

    connect(model.get(), &QStandardItemModel::itemChanged, [this](QStandardItem *item) {
        if (!instance)
            return;

        std::string text = item->data(Qt::DisplayRole).toString().toUtf8().constData();

        if (auto variant = item->data(PropertyValueRole); variant.isValid()) {
            auto sp = variant.value<std::shared_ptr<const ScriptProperty>>();
            instance->values[sp] = text;
            return;
        }

        if (auto variant = item->data(PropertyCustomTitleRole); variant.isValid()) {
            auto sp = variant.value<std::shared_ptr<CustomScriptProperty>>();
            if (text.empty()) {
                instance->customValues.erase(sp);
                model->removeRow(item->row());
            } else
                sp->memberName = text;
            return;
        }

        if (auto variant = item->data(PropertyCustomValueRole); variant.isValid()) {
            auto sp = variant.value<std::shared_ptr<CustomScriptProperty>>();
            if(auto it = instance->customValues.find(sp); it != instance->customValues.end()) 
                instance->customValues[sp] = text;
            return;
        }
    });
}                

void ScriptPropertyEditor::SetValue(const std::string &in) {
    if (!instance)
        return;

    instance->Deseralize(in);

    model->removeRows(0, model->rowCount());

    auto root = model->invisibleRootItem();
    for (auto &[item, value] : instance->values) {
        QList<QStandardItem*> cols;
        auto title = new QStandardItem(item->memberName.c_str());
        title->setEditable(false);
        cols << title;

        auto valueItem = new QStandardItem(value.c_str());
        valueItem->setEditable(true);
        valueItem->setData(QVariant::fromValue(item), PropertyValueRole);
        cols << valueItem;
        root->appendRow(cols);
    }

    for (auto &[item, value] : instance->customValues) {
        QList<QStandardItem*> cols;
        auto title = new QStandardItem(item->memberName.c_str());
        title->setEditable(true);
        title->setData(QVariant::fromValue(item), PropertyCustomTitleRole);
        title->setIcon(QIcon(ICON_16_BLUE_PLUS));
        cols << title;

        auto valueItem = new QStandardItem(value.c_str());
        valueItem->setEditable(true);
        valueItem->setData(QVariant::fromValue(item), PropertyCustomValueRole);
        cols << valueItem;
        root->appendRow(cols);
    }
}

std::string ScriptPropertyEditor::GetValue()  {
    if (!instance)
        return "";
    return instance->Serialize();
}

void ScriptPropertyEditor::ReloadDialog() {
    model->clear();
    model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Value"));

    auto *component = dataSouce(TypeEditor::CustomEditorItemDelegate::QtRoles::EditableComponent).value<QtShared::DataModels::EditableComponent*>();
    if (!component) {
        return;
    }

    auto valuesEditor = component->GetValuesEditor();
    auto script = valuesEditor->get("Script");

    auto spp = GetModuleManager()->QuerryModule<ScriptPropertyProvider>();
    auto properties = spp->GetScriptProperties(script);

    if (!properties) {
        return;
    }

    instance = properties->CreateInstance();
}

}