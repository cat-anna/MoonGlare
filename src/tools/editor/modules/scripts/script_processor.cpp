#if 0

#include <ToolBase/Module.h>
#include <iCustomEnum.h>
#include <iFileIconProvider.h>
#include <iFileProcessor.h>
#include <iIssueReporter.h>
#include <icons.h>

#include <libs/LuaWrap/src/LuaDeleter.h>
#include <libs/LuaWrap/src/LuaException.h>

#include "../FileSystem.h"
#include "../ScriptProperty.h"
#include "../Windows/MainWindow.h"
#include "ScriptProcessor.h"

#include <boost/algorithm/string.hpp>

namespace MoonGlare {
namespace Editor {
namespace Processor {

//----------------------------------------------------------------------------------

struct ScriptFileProcessorInfo : public iModule,
                                 public QtShared::iFileProcessorInfo,
                                 public QtShared::iCustomEnumSupplier,
                                 public QtShared::iFileIconInfo {

    ScriptFileProcessorInfo(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    std::shared_ptr<QtShared::SetEnum> scriptListEnum = std::make_shared<QtShared::SetEnum>("string:Script.Script");

    bool PostInit() override {

        ScriptPropertySetInfo spsi;
        spsi.className = "ScriptComponent";
        auto spp = GetModuleManager()->QueryModule<ScriptPropertyProvider>();
        spp->SetScriptProperties(std::make_shared<ScriptPropertySetInfo>(std::move(spsi)));

        return true;
    }

    QtShared::SharedFileProcessor CreateFileProcessor(std::string URI) override {
        return std::make_shared<ScriptFileProcessor>(this, std::move(URI));
    }

    std::vector<std::string> GetSupportedTypes() {
        return {
            "lua",
        };
    }

    std::vector<std::shared_ptr<QtShared::iCustomEnum>> GetCustomEnums() const override {
        return {
            scriptListEnum,
        };
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return {
            FileIconInfo{
                "lua",
                ICON_16_LUALOGO_RESOURCE,
            },
        };
    }
};
ModuleClassRegister::Register<ScriptFileProcessorInfo> ScriptFileProcessorInfoReg("ScriptFileProcessorInfo");

//----------------------------------------------------------------------------------

static const char InternalScript[] = R"===(

oo = { }        

function oo.Inherit(base)
    local class = { }

    class.__index = class
    class.BaseClass = base

    setmetatable(class, base)

    return class
end

function oo.Class()
    return oo.Inherit()
end

function require()
    return {}
end

)===";

//----------------------------------------------------------------------------------

ScriptFileProcessor::ScriptFileProcessor(ScriptFileProcessorInfo *Module, std::string URI)
    : QtShared::iFileProcessor(std::move(URI)), module(Module) {}

ScriptFileProcessor::ProcessResult ScriptFileProcessor::ProcessFile() {
    try {
        LoadScript();
        InitLua();
        ExecuteScript();
        CheckReferences();
        CheckProperties();
        Finalize();
    } catch (...) {
        return ProcessResult::UnknownFailure;
    }
    return ProcessResult::Success;
}

void ScriptFileProcessor::InitLua() {
    m_Lua.reset(luaL_newstate());
    auto lua = m_Lua.get();
    luaopen_base(lua);
    luaopen_math(lua);
    luaopen_bit(lua);
    luaopen_string(lua);
    luaopen_table(lua);
#ifdef DEBUG
    luaopen_debug(lua);
#endif
    lua_atpanic(lua, &LuaWrap::eLuaPanic::ThrowPanicHandler);

    int result = luaL_loadstring(lua, InternalScript);
    switch (result) {
    case 0:
        if (lua_pcall(lua, 0, 0, 0) == 0)
            return;
        AddLog(Error, "Lua error: " << lua_tostring(lua, -1));
        break;
    case LUA_ERRSYNTAX:
        AddLogf(Error, "Unable to load internal script: Error string: '%s'", lua_tostring(lua, -1));
        break;
    case LUA_ERRMEM:
        AddLog(Error, "Unable to load script: Memory allocation failed!");
        break;
    }

    throw std::runtime_error("Unable to Execute internal lua processor script!");
}

void ScriptFileProcessor::ExecuteScript() {
    auto reporter = module->GetModuleManager()->QueryModule<QtShared::IssueReporter>();

    auto ParseError = [reporter, this](const std::string &errorstr, QtShared::Issue::Type type) {
        std::regex pieces_regex(R"==(\[(.+)\]\:(\d+)\:\ (.+))==", std::regex::icase);
        std::smatch pieces_match;
        if (std::regex_match(errorstr, pieces_match, pieces_regex)) {
            QtShared::Issue issue;
            issue.fileName = m_URI;
            issue.sourceLine = std::strtol(pieces_match[2].str().c_str(), nullptr, 10);
            issue.message = pieces_match[3];
            issue.type = type;
            issue.group = "Lua";

            reporter->ReportIssue(std::move(issue));
        }
    };

    auto lua = m_Lua.get();
    int result = luaL_loadstring(lua, scriptString.c_str());
    switch (result) {
    case 0: {
        // only syntax check is working correctly
        // if (lua_pcall(lua, 0, 0, 0) == 0)
        //     return;
        // std::string errorstr = lua_tostring(lua, -1);
        // ParseError(errorstr, QtShared::Issue::Type::Warning);
        // AddLogf(Hint, "Lua script '%s' error: %s", m_URI.c_str(), errorstr.c_str());
        reporter->DeleteIssue(MakeIssueId());
        break;
    }
    case LUA_ERRSYNTAX: {
        std::string errorstr = lua_tostring(lua, -1);
        ParseError(errorstr, QtShared::Issue::Type::Error);
        AddLogf(Hint, "Unable to load '%s': Error string: '%s'", m_URI.c_str(), errorstr.c_str());
        break;
    }
    case LUA_ERRMEM:
        AddLogf(Error, "Unable to load '%s': Memory allocation failed!", m_URI.c_str());
        break;
    }
}

void ScriptFileProcessor::CheckReferences() {
    auto reporter = module->GetModuleManager()->QueryModule<QtShared::IssueReporter>();
    auto fs = module->GetModuleManager()->QueryModule<FileSystem>();

    auto uris = FindAllURI(scriptString);
    for (auto &itm : uris) {
        QtShared::Issue issue;
        issue.fileName = m_URI;
        issue.message = "File " + itm + " does not exists!";
        issue.type = QtShared::Issue::Type::Error;
        issue.group = "lua";
        issue.internalID = MakeIssueId("Error", itm);
        if (fs->FileExists(itm))
            reporter->DeleteIssue(issue.internalID);
        else
            reporter->ReportIssue(std::move(issue));
    }
}

void ScriptFileProcessor::CheckProperties() {
    std::vector<std::string> lines;
    boost::split(lines, scriptString, boost::is_any_of("\n"));

    ScriptPropertySetInfo spsi;

    {
        std::regex r(R"==(oo\.Inherit\(\"(.+?)\"\))==");
        std::smatch sm;
        std::vector<std::string> out;
        if (std::regex_search(scriptString, sm, r)) {
            spsi.parentClassName = sm[1];
        }
    }
    spsi.className = regName;

    for (auto &line : lines) {
        if (line.find("--#") == std::string::npos)
            continue;

        std::regex r(R"==(\s*(\w+)\s*\.\s*(\w+)\s*=\s*(.+?)\s*\-\-\#\s*(.+))==");
        std::smatch sm;
        if (std::regex_search(line, sm, r)) {
            std::string localType = sm[1];
            std::string member = sm[2];
            std::string defaultValue = sm[3];
            std::string propComment = sm[4];

            ScriptProperty sp;
            sp.comment = propComment;
            sp.memberName = member;
            sp.defaultValue = defaultValue;
            // sp.type = "";
            // sp.conditions = {};
            spsi.properties.emplace_back(std::make_shared<ScriptProperty>(std::move(sp)));
        }
    }
    auto spp = module->GetModuleManager()->QueryModule<ScriptPropertyProvider>();
    spp->SetScriptProperties(std::make_shared<ScriptPropertySetInfo>(std::move(spsi)));
}

void ScriptFileProcessor::LoadScript() {
    auto fs = module->GetModuleManager()->QueryModule<FileSystem>();
    StarVFS::ByteTable scriptData;
    if (!fs->GetFileData(m_URI, scriptData)) {
        // todo: log sth
        throw std::runtime_error("Unable to read file: " + m_URI);
    }
    if (scriptData.byte_size() == 0) {
        // todo: log sth
    }
    scriptString = std::string((char *)scriptData.get(), scriptData.byte_size());

    // script exists, so insert it into custom enum set
    std::regex pieces_regex(R"(file\:\/\/(\/[a-z0-9\.\/]+)\.lua)", std::regex::icase);
    std::smatch pieces_match;
    if (std::regex_match(m_URI, pieces_match, pieces_regex)) {
        regName = pieces_match[1];
        module->scriptListEnum->Add(regName);
    }
}

void ScriptFileProcessor::Finalize() { m_Lua.reset(); }

} // namespace Processor
} // namespace Editor
} // namespace MoonGlare

#endif