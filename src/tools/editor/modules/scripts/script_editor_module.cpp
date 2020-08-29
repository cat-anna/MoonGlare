#include "file_icon_info.hpp"
#include "script_editor.hpp"
#include <main_window_tabs.hpp>
#include <qt_gui_icons.h>
#include <regex>
#include <ui_script_editor.h>

// #include <ToolBase/UserQuestions.h>
// #include <iFileIconProvider.h>
// #include <iFileProcessor.h>
// #include <icons.h>
// #include "../Windows/MainWindow.h"

namespace MoonGlare::Tools::Editor::Modules {

static const char LuaScriptPattern[] = R"(-- {name} script

local class = oo.Inherit("ScriptComponent")

function class:OnCreate()
    --self:SetStep(false)
end

function class:OnDestroy()
end

function class:Step(data)
end

return class

)";

#if 0
struct ScriptFileConstructor : public QtShared::iEditor {
public:
    ScriptFileConstructor(SharedModuleManager modmgr) : m_ModuleManager(std::move(modmgr)) {
        m_UserQuestions = m_ModuleManager->QueryModule<UserQuestions>();
    }
    virtual ~ScriptFileConstructor() {}

    SharedModuleManager m_ModuleManager;
    std::shared_ptr<UserQuestions> m_UserQuestions;

    // iEditor
    virtual bool Create(const std::string &LocationURI,
                        const QtShared::iEditorInfo::FileHandleMethodInfo &what) override {
        std::string name;
        if (!m_UserQuestions->QueryStringInput("Enter name:", name))
            return false;

        std::string URI = LocationURI + name + ".lua";

        auto fs = m_ModuleManager->QueryModule<FileSystem>();
        if (!fs->CreateFile(URI)) {
            m_UserQuestions->ErrorMessage("Failed during creating lua file");
            AddLog(Hint, "Failed to create lua file: " << URI);
            return false;
        }

        std::unordered_map<std::string, std::string> Patterns;
        Patterns[R"(\{name\})"] = name;

        std::string Pattern = LuaScriptPattern;

        for (auto &it : Patterns) {
            std::regex pat(it.first);
            std::string out;
            out.reserve(Pattern.size() * 2);
            std::regex_replace(std::back_inserter(out), Pattern.begin(), Pattern.end(), pat,
                               it.second);
            out.swap(Pattern);
        }

        StarVFS::ByteTable bt;
        bt.from_string(Pattern);

        if (!fs->SetFileData(URI, bt)) {
            // todo: log sth
            return false;
        }

        AddLog(Hint, "created lua: " << URI);

        return true;
    }
    // virtual bool OpenData(const std::string &URI) { return false; }
    // virtual bool SaveData() { return false; }
    // virtual bool TryCloseData() { return false; }
};

#endif

//----------------------------------------------------------------------------------

struct ScriptEditorDockInfo : public iModule, public iFileIconInfo, public iEditorInfo, public iEditorFactory {
    ScriptEditorDockInfo(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    // std::any QueryInterface(const std::type_info &info) override {
    //     if (info == typeid(iFileSystemViewerPreview))
    //         return std::dynamic_pointer_cast<iFileSystemViewerPreview>(GetInstance());
    //     return {};
    // }

    std::weak_ptr<iMainWindowTabsControl> tab_control;

    bool Initialize() override {
        tab_control = GetModuleManager()->QueryModule<iMainWindowTabsControl>();
        return true;
    };

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{FileIconInfo{".lua", ICON_16_LUALOGO_RESOURCE}};
    }

    SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions &options) {

        if (method.method_id == "edit_script") {
            auto locked_tab_control = tab_control.lock();
            if (locked_tab_control) {
                std::string tab_name = "edit_script|" + options.full_path;
                if (locked_tab_control->TabExists(tab_name)) {
                    locked_tab_control->ActivateTab(tab_name);
                    auto tab = locked_tab_control->GetTab(tab_name);
                    return std::dynamic_pointer_cast<iEditor>(tab);
                }

                auto editor =
                    std::make_shared<ScriptEditor>(locked_tab_control->GetTabParentWidget(), GetModuleManager());
                locked_tab_control->AddTab(tab_name, editor);
                locked_tab_control->SetTabIcon(editor.get(), QIcon(QString::fromStdString(method.icon)));
                return editor;
            }
        }
        // if (method.method_id == "create_script") {
        // return std::make_shared<ScriptFileConstructor>(GetModuleManager());
        // }
        return nullptr;
    }

    std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            // FileHandleMethodInfo{"lua", ICON_16_LUALOGO_RESOURCE, "Script...", "create_script"},
        };
    }
    std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{".lua", ICON_16_LUALOGO_RESOURCE, "Edit script", "edit_script"},
        };
    }
};

ModuleClassRegister::Register<ScriptEditorDockInfo> ScriptEditorDockInfoReg("ScriptEditorDockInfo");

} // namespace MoonGlare::Tools::Editor::Modules
