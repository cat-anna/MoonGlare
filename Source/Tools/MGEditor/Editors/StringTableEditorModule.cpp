#include PCH_HEADER

#include <FileSystem.h>

#include <icons.h>
#include "iFileIconProvider.h"
#include "iEditor.h"

#include "StringTableEditor.h"

#include <ToolBase/interfaces/MainWindowTabs.h>

namespace MoonGlare::Editor {
using namespace QtShared;

struct StringTableEditorModule
    : public iModule
    //, public iSoundPlayer
    , public QtShared::iEditorInfo
    , public QtShared::iEditorFactory
{
    StringTableEditorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) { }

    std::shared_ptr<FileSystem> fileSystem;
    std::shared_ptr<iMainWindowTabsCtl> tabCtl;

    bool Initialize() override {
        if (!iModule::Initialize())
            return false;

        fileSystem = GetModuleManager()->QuerryModule<FileSystem>();
        assert(fileSystem);
        tabCtl = GetModuleManager()->QuerryModule<iMainWindowTabsCtl>();
        assert(tabCtl);

        return true;
    }
    bool PostInit() override {
        if (!iModule::PostInit())
            return false;
        return true;
    }
    bool Finalize() override {
        if (!iModule::Finalize())
            return false;
        return true;
    }

    std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override { 
        std::vector<FileHandleMethodInfo> ret;
        ret.emplace_back(FileHandleMethodInfo{ "xml", ICON_16_CONFIG_FILE, "Edit string table", "edit"});
        return std::move(ret);
    }

    SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions &options) const override {
        XMLFile xfile;
        if (!fileSystem->OpenXML(xfile, options.fileURI))
            return nullptr;

        std::string root = xfile->document_element().name();
        if (root != "StringTable")
            return nullptr;

        auto delim = options.fileURI.rfind("/");
        auto name = options.fileURI.substr(delim + 1);
        auto xlen = strlen("file://");
        auto path = options.fileURI.substr(0, delim + 1);

        if (path != "file:///Tables/")
            return nullptr;

        auto delim2 = name.find(".");
        auto tname = name.substr(0, delim2);
        auto tabName = "StringTale." + tname;

        if (tabCtl->TabExists(tabName))
            return nullptr;

        auto sp = std::make_shared<StringTableEditor>(tabCtl->GetTabParentWidget(), GetModuleManager(), std::move(tname));
        tabCtl->AddTab(tabName, sp);
        return sp;
    }
};

ModuleClassRgister::Register<StringTableEditorModule> StringTableEditorReg("StringTableEditor");

}
