#include "entity_editor.hpp"
#include <change_container.hpp>
#include <editor_provider.h>
#include <file_icon_info.hpp>
#include <issue_reporter.hpp>
#include <main_window_tabs.hpp>
#include <memory>
#include <qt_gui_icons.h>
#include <runtime_modules.h>

namespace MoonGlare::Tools::Editor::Modules {

struct EntityEditorModule : public iModule, public iFileIconInfo, public iEditorInfo, public iEditorFactory {
    EntityEditorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    std::weak_ptr<iMainWindowTabsControl> tab_control;

    bool Initialize() override {
        tab_control = GetModuleManager()->QueryModule<iMainWindowTabsControl>();
        return true;
    };

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{".entity", ICON_16_ENTITYPATTERN_RESOURCE},
        };
    }

    virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{".entity", ICON_16_ENTITYPATTERN_RESOURCE, "Entity pattern...", "create.entity"},
        };
    }

    virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{".entity", ICON_16_ENTITYPATTERN_RESOURCE, "Edit entity pattern", "open.entity"},
        };
    }

    SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method,
                           const EditorRequestOptions &options) override {
        auto locked_tab_control = tab_control.lock();
        if (!locked_tab_control) {
            return nullptr;
        }

        // if (method.method_id == "create.entity") {
        std::string tab_name = "entity|" + options.full_path;
        if (locked_tab_control->TabExists(tab_name)) {
            locked_tab_control->ActivateTab(tab_name);
            auto tab = locked_tab_control->GetTab(tab_name);
            return std::dynamic_pointer_cast<iEditor>(tab);
        }

        auto editor = std::make_shared<EntityEditor>(locked_tab_control->GetTabParentWidget(), GetModuleManager());
        locked_tab_control->AddTab(tab_name, editor);
        locked_tab_control->SetTabIcon(editor.get(), QIcon(QString::fromStdString(method.icon)));
        return editor;
        // }

        return nullptr;
    }
};

ModuleClassRegister::Register<EntityEditorModule> EntityEditorReg("EntityEditor");

} // namespace MoonGlare::Tools::Editor::Modules
