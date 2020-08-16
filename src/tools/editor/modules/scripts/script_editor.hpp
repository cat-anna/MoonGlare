#pragma once

#include <QWidget>
#include <change_container.hpp>
#include <dock_window.hpp>
#include <editor_provider.h>
#include <issue_reporter.hpp>
#include <main_window_tabs.hpp>
#include <memory>
#include <runtime_modules.h>
#include <user_questions.hpp>
#include <writable_file_system.h>

namespace Ui {
class ScriptEditor;
}

namespace MoonGlare::Tools::Editor::Modules {

class ScriptEditor : public QWidget,
                     public iEditor,
                     public iTypedChangeContainer<ScriptEditor>,
                     public iTabViewBase,
                     public UserQuestions,
                     public std::enable_shared_from_this<ScriptEditor> {
    Q_OBJECT;

public:
    ScriptEditor(QWidget *parent, SharedModuleManager smm);
    virtual ~ScriptEditor();

    // iTabViewBase
    std::string GetTabTitle() const override;
    bool CanClose() const override;

    //iEditor
    bool OpenData(const std::string &full_path,
                  const iEditorInfo::FileHandleMethodInfo &method) override;

    //iChangeContainer
    bool SaveChanges() override;
    std::string GetInfoLine() const override;
    bool DropChanges() override;
    bool DropChangesPossible() override;
    QIcon GetInfoIcon() const override;

protected:
    std::string full_file_path;
    std::shared_ptr<iWritableFileSystem> file_system;

    IssueProducer issue_producer;

    SharedModuleManager GetModuleManager() const { return shared_module_manager; }

    bool ReloadFile();
    bool SaveFile();
protected slots:
    void ModificationChanged(bool changed);
    void RedoAvailable(bool available);
    void UndoAvailable(bool available);
    void CopyAvailable(bool available);
    void CheckCodeIssues();

private:
    std::unique_ptr<Ui::ScriptEditor> ui;
    SharedModuleManager shared_module_manager;
};

} // namespace MoonGlare::Tools::Editor::Modules
