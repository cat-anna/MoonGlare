#include "script_editor.hpp"
#include <qt_gui_icons.h>
#include <regex>
#include <ui_script_editor.h>

// #include <iFileIconProvider.h>
// #include <iFileProcessor.h>

namespace MoonGlare::Tools::Editor::Modules {

ScriptEditor::ScriptEditor(QWidget *parent, SharedModuleManager smm)
    : iTypedChangeContainer(smm), issue_producer(smm), shared_module_manager(smm) {
    // SetSettingId("ScriptEditor");
    ui = std::make_unique<Ui::ScriptEditor>();
    ui->setupUi(this);

    file_system = GetModuleManager()->QueryModule<iWritableFileSystem>();

    connect(ui->plainTextEdit, &QPlainTextEdit::modificationChanged, this,
            &ScriptEditor::ModificationChanged);
    connect(ui->plainTextEdit, &QPlainTextEdit::redoAvailable, this, &ScriptEditor::RedoAvailable);
    connect(ui->plainTextEdit, &QPlainTextEdit::undoAvailable, this, &ScriptEditor::UndoAvailable);
    connect(ui->plainTextEdit, &QPlainTextEdit::copyAvailable, this, &ScriptEditor::CopyAvailable);

    connect(ui->plainTextEdit, &CodeEditor::CodeCompiled, this, &ScriptEditor::CheckCodeIssues);

    connect(ui->action_save, &QAction::triggered, [this](auto) { SaveChanges(); });

    SetChangesName("Script editor");
}

ScriptEditor::~ScriptEditor() {
    ui.reset();
}

bool ScriptEditor::ReloadFile() {
    std::string file_data;
    if (file_system->ReadFileByPath(full_file_path, file_data)) {
        ui->plainTextEdit->setPlainText(QString::fromStdString(file_data));
        ModificationChanged(false);
        return true;
    } else {
        AddLogf(Warning, "Failed to read file %s", full_file_path.c_str());
        return false;
    }
}

bool ScriptEditor::SaveFile() {
    auto file_data = ui->plainTextEdit->toPlainText().toStdString();
    if (!file_system->WriteFileByPath(full_file_path, file_data)) {
        AddLogf(Error, "File save failed");
        return false;
    }
    ui->plainTextEdit->document()->setModified(false);
    return true;
}

void ScriptEditor::CheckCodeIssues() {
    issue_producer.ClearAllIssues();
    auto result = ui->plainTextEdit->GetCompilationResult();
    if (result.status == CompilationResult::Status::Success) {
        return;
    }

    for (auto &item : result.lineInfo) {
        Issue i;
        i.file_name = full_file_path;
        i.message = item.second;
        i.source_line = item.first;
        i.group = "script";
        i.type = Issue::Type::Error;
        issue_producer.ReportIssue(std::move(i));
    }
}

//----------------------------------------------------------------------------------
//iEditor

bool ScriptEditor::OpenData(const std::string &full_path,
                            const iEditorInfo::FileHandleMethodInfo &method) {
    if (full_file_path == full_path) {
        return true;
    }

    if (IsChanged() && !AskToDropChanges(full_file_path)) {
        return false;
    }

    full_file_path = full_path;
    if (!ReloadFile())
        return false;

    auto controller = GetController();
    if (controller) {
        controller->SetTabTitle(this, full_file_path);
    }

    return true;
}

//----------------------------------------------------------------------------------

void ScriptEditor::ModificationChanged(bool changed) {
    SetModifiedState(changed);
    ui->action_save->setEnabled(changed);
}

void ScriptEditor::RedoAvailable(bool available) {
}

void ScriptEditor::UndoAvailable(bool available) {
}

void ScriptEditor::CopyAvailable(bool available) {
}

//----------------------------------------------------------------------------------
//iChangeContainer

bool ScriptEditor::SaveChanges() {
    return SaveFile();
}

std::string ScriptEditor::GetInfoLine() const {
    return full_file_path;
}

bool ScriptEditor::DropChanges() {
    if (IsChanged()) {
        return ReloadFile();
    }
    SetModifiedState(false);
    return true;
}

bool ScriptEditor::DropChangesPossible() {
    return true;
}

QIcon ScriptEditor::GetInfoIcon() const {
    return ICON_16_LUALOGO;
}

//----------------------------------------------------------------------------------
// iTabViewBase

std::string ScriptEditor::GetTabTitle() const {
    return full_file_path;
}

bool ScriptEditor::CanClose() const {
    if (!IsChanged()) {
        return true;
    }
    if (!AskToDropChanges(full_file_path)) {
        return false;
    }

    return const_cast<ScriptEditor *>(this)->DropChanges();
}

} // namespace MoonGlare::Tools::Editor::Modules
