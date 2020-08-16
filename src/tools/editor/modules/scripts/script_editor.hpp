#pragma once

#include <QWidget>
#include <dock_window.hpp>
#include <editor_provider.h>
#include <main_window_tabs.hpp>
#include <runtime_modules.h>
#include <user_questions.hpp>
#include <writable_file_system.h>

namespace Ui {
class ScriptEditor;
}

namespace MoonGlare::Tools::Editor::Modules {

class ScriptEditor : public QWidget,
                     public iEditor,
                     //  public MoonGlare::Module::iChangeContainer,
                     public iTabViewBase,
                     public UserQuestions {
    Q_OBJECT;

public:
    ScriptEditor(QWidget *parent, SharedModuleManager smm);
    virtual ~ScriptEditor();

    // iTabViewBase
    std::string GetTabTitle() const override { return full_file_path; };
    bool CanClose() const override { return true; }

    bool OpenData(const std::string &full_path,
                  const iEditorInfo::FileHandleMethodInfo &method) override;

protected:
    std::string full_file_path;

    std::shared_ptr<iWritableFileSystem> file_system;

    SharedModuleManager GetModuleManager() const { return shared_module_manager; }
    // void Clear();
protected slots:

private:
    std::unique_ptr<Ui::ScriptEditor> ui;
    SharedModuleManager shared_module_manager;
};

} // namespace MoonGlare::Tools::Editor::Modules
