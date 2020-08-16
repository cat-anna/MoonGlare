#include "script_editor.hpp"
#include <regex>
#include <ui_script_editor.h>

// #include <ToolBase/UserQuestions.h>
// #include <iFileIconProvider.h>
// #include <iFileProcessor.h>
// #include <icons.h>
// #include "../Windows/MainWindow.h"

namespace MoonGlare::Tools::Editor::Modules {

ScriptEditor::ScriptEditor(QWidget *parent, SharedModuleManager smm) : shared_module_manager(smm) {
    // SetSettingId("ScriptEditor");
    ui = std::make_unique<Ui::ScriptEditor>();
    ui->setupUi(this);

    file_system = GetModuleManager()->QueryModule<iWritableFileSystem>();

    // SetChangesName("String table " + this->tableName);
}

ScriptEditor::~ScriptEditor() {
    ui.reset();
}

bool ScriptEditor::OpenData(const std::string &full_path,
                            const iEditorInfo::FileHandleMethodInfo &method) {
    if (full_file_path == full_path) {
        return true;
    }
    //todo: check if changed
    full_file_path = full_path;
    //todo: reload file

    std::string file_data;
    if (file_system->ReadFileByPath(full_file_path, file_data)) {
        AddLogf(Warning, "Failed to read file %s", full_file_path.c_str());
        ui->plainTextEdit->setPlainText(QString::fromStdString(file_data));

        auto controller = GetController();
        if (controller) {
            controller->SetTabTitle(this, full_path);
        }
        return true;
    }

    return false;
}

} // namespace MoonGlare::Tools::Editor::Modules
