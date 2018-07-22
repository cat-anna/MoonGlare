#include PCH_HEADER
#include "SoundPlayer.h"

#include <icons.h>
#include "iFileIconProvider.h"
#include "iEditor.h"

#include <EngineBase/SoundSystem/iSoundSystem.h>

namespace MoonGlare::Editor {
using namespace QtShared;


struct SoundPlayerEditor : public iEditor {
    SoundSystem::iSoundSystem *soundSystem;

    SoundPlayerEditor(SoundSystem::iSoundSystem *ss) : soundSystem(ss) { }
    ~SoundPlayerEditor() {}

    bool OpenData(const std::string &uri) override {
        auto ss = soundSystem;
        std::thread([ss, uri] {
            auto p = ss->OpenSound(uri, true);
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }).detach();
        return true;
    }
};

struct SoundPlayerModule
    : public iModule
    , public iFileIconInfo
    , public iSoundPlayer
    , public QtShared::iEditorInfo
    , public QtShared::iEditorFactory
{
    std::unique_ptr<SoundSystem::iSoundSystem> soundSystem;

    SoundPlayerModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) { }

    void Play(const std::string &uri) override {

    }

    virtual bool Initialize() {
        if (!iModule::Initialize())
            return false;

        soundSystem.reset(SoundSystem::iSoundSystem::CreateNew());
        soundSystem->Initialize(GetModuleManager()->QuerryModule<iFileSystem>().get());

        return true;
    }
    virtual bool Finalize() {
        if (!iModule::Finalize())
            return false;

        if (soundSystem)
            soundSystem->Finalize();
        soundSystem.reset();

        return true;
    }

    virtual std::vector<FileIconInfo> GetFileIconInfo() const override {
        if (!soundSystem)
            return { };

        std::vector<FileIconInfo> ret;
        for (auto &format : soundSystem->GetSupportedFormats()) {
            ret.emplace_back(FileIconInfo{ format.fileExtension, ICON_16_SOUND_RESOURCE, });
        }

        return std::move(ret);
    }

    virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override { 
        if (!soundSystem)
            return {};

        std::vector<FileHandleMethodInfo> ret;
        for (auto &format : soundSystem->GetSupportedFormats()) {
            ret.emplace_back(FileHandleMethodInfo{ format.fileExtension, ICON_16_SOUND_RESOURCE, "Play...", "play"});
        }

        return std::move(ret);
    }

    virtual SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions&options) const override {
        return std::make_shared<SoundPlayerEditor>(soundSystem.get());
    }

    //virtual QtShared::SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions&options) const {
    //    return std::make_shared<ScriptFileConstructor>(GetModuleManager());
    //}

    //virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
    //    return std::vector<FileHandleMethodInfo> {
    //        //FileHandleMethodInfo{ "lua", ICON_16_LUALOGO_RESOURCE, "Script...", "lua", },
    //    };
    //}
};
ModuleClassRgister::Register<SoundPlayerModule> SoundPlayerReg("SoundPlayer");

}
