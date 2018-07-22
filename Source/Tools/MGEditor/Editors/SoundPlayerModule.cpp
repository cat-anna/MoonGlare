#include PCH_HEADER
//#include "iSoundPlayer.h"

#include <icons.h>
#include "iFileIconProvider.h"
#include "iEditor.h"

#include <EngineBase/SoundSystem/iSoundSystem.h>
#include "SoundPlayerView.h"
#include "DockWindows/iFileSystemViewerPreview.h"

namespace MoonGlare::Editor {
using namespace QtShared;

struct SoundPlayerModule
    : public iModule
    , public iFileIconInfo
    //, public iSoundPlayer
    , public QtShared::iEditorInfo
    , public QtShared::iEditorFactory
{
    std::unique_ptr<SoundSystem::iSoundSystem> soundSystem;

    SoundPlayerModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) { }

    //void Play(const std::string &uri) override {
    //}

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
        auto iface = GetModuleManager()->QuerryModule<DockWindows::iFileSystemViewerPreview>();
        if (!iface)
            return nullptr;
        auto player = std::make_shared<SoundPlayerView>(nullptr, soundSystem.get());
        iface->SetPreviewEditor(player);
        return player;
    }

};
ModuleClassRgister::Register<SoundPlayerModule> SoundPlayerReg("SoundPlayer");

}
