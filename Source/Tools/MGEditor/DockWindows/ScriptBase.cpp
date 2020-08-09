/*
 * Generated by cppsrc.sh
 * On 2016-07-22  0:08:40,44
 * by Paweu
 */
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "ScriptBase.h"

#include <ToolBase/UserQuestions.h>
#include <iFileIconProvider.h>
#include <iFileProcessor.h>
#include <icons.h>

#include <regex>

#include "../Windows/MainWindow.h"

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

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
            std::regex_replace(std::back_inserter(out), Pattern.begin(), Pattern.end(), pat, it.second);
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

//----------------------------------------------------------------------------------

struct ScriptBaseInfo : public iModule,
                        public QtShared::iFileIconInfo,
                        public QtShared::iEditorInfo,
                        public QtShared::iEditorFactory {
    ScriptBaseInfo(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    virtual std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{
                "lua",
                ICON_16_LUALOGO_RESOURCE,
            },
        };
    }

    virtual QtShared::SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method,
                                             const EditorRequestOptions &options) const {
        return std::make_shared<ScriptFileConstructor>(GetModuleManager());
    }

    virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{
                "lua",
                ICON_16_LUALOGO_RESOURCE,
                "Script...",
                "lua",
            },
        };
    }
};
ModuleClassRegister::Register<ScriptBaseInfo> ScriptBaseInfoReg("ScriptBase");

//----------------------------------------------------------------------------------

} // namespace DockWindows
} // namespace Editor
} // namespace MoonGlare
