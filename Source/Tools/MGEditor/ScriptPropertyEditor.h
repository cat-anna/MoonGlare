#pragma once

#include <string>
#include <vector>
#include <map>

#include <ToolBase/Module.h>
#include <ToolBase/UserQuestions.h>
#include "TypeEditor/CustomType.h"  
#include <Windows/TreeViewDialog.h>

namespace MoonGlare::Editor {

class TreeViewDialog;

class ScriptPropertyEditor : public TypeEditor::CustomTypeEditor, public TreeViewDialog, public UserQuestions {
public:
    ScriptPropertyEditor(QWidget *Parent);

    void SetValue(const std::string &in) override;
    std::string GetValue() override;

    static std::string ToDisplayText(const std::string &in) {
        return in;
        //return "<properties>";
    }

    void SetDataSource(std::function<QVariant(int)> func) override {
        dataSouce = func;
        ReloadDialog();
    };

    static constexpr int PropertyValueRole = Qt::UserRole + 1;
    static constexpr int PropertyCustomValueRole = Qt::UserRole + 2;
    static constexpr int PropertyCustomTitleRole = Qt::UserRole + 3;
protected:                          
private:
    std::function<QVariant(int)> dataSouce;

    std::unique_ptr<QStandardItemModel> model;
    std::shared_ptr<ScriptPropertyInstance> instance;

    void ReloadDialog();
};

}

