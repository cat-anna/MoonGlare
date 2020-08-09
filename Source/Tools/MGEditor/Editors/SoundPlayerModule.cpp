#include PCH_HEADER
//#include "iSoundPlayer.h"

#include "iEditor.h"
#include "iFileIconProvider.h"
#include <icons.h>

#include "DockWindows/iFileSystemViewerPreview.h"
#include "SoundPlayerView.h"
#include <Foundation/SoundSystem/iSoundSystem.h>

namespace MoonGlare::Editor {
using namespace QtShared;

struct SoundPlayerModule : public iModule,
                           public iFileIconInfo
    //, public iSoundPlayer
    ,
                           public QtShared::iEditorInfo,
                           public QtShared::iEditorFactory {
    std::shared_ptr<SoundSystem::iSoundSystem> soundSystem;

    SoundPlayerModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    // void Play(const std::string &uri) override {
    //}

    bool Initialize() override {
        if (!iModule::Initialize())
            return false;
        soundSystem = SoundSystem::iSoundSystem::Create();

        return true;
    }
    bool PostInit() override {
        if (!iModule::PostInit())
            return false;

        soundSystem->Initialize(GetModuleManager()->GetInterfaceMap());
        return true;
    }
    bool Finalize() override {
        if (!iModule::Finalize())
            return false;

        if (soundSystem)
            soundSystem->Finalize();
        soundSystem.reset();

        return true;
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        if (!soundSystem)
            return {};

        std::vector<FileIconInfo> ret;
        for (auto &format : soundSystem->GetSupportedFormats()) {
            ret.emplace_back(FileIconInfo{
                format.fileExtension,
                ICON_16_SOUND_RESOURCE,
            });
        }

        return std::move(ret);
    }

    std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
        if (!soundSystem)
            return {};

        std::vector<FileHandleMethodInfo> ret;
        for (auto &format : soundSystem->GetSupportedFormats()) {
            ret.emplace_back(FileHandleMethodInfo{format.fileExtension, ICON_16_SOUND_RESOURCE, "Play...", "play"});
        }

        return std::move(ret);
    }

    SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method,
                           const EditorRequestOptions &options) const override {
        auto iface = GetModuleManager()->QueryModule<DockWindows::iFileSystemViewerPreview>();
        if (!iface)
            return nullptr;
        auto player = std::make_shared<SoundPlayerView>(nullptr, soundSystem.get());
        iface->SetPreviewEditor(player);
        return player;
    }
};

ModuleClassRegister::Register<SoundPlayerModule> SoundPlayerReg("SoundPlayer");

} // namespace MoonGlare::Editor
