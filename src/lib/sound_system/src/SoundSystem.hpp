#pragma once

#include "WorkThread.hpp"
#include "sound_system/iSoundSystem.hpp"

namespace MoonGlare::SoundSystem {

class SoundSystem : public iSoundSystem,
                    // public Settings::iChangeCallback,
                    public std::enable_shared_from_this<SoundSystem> {
public:
    SoundSystem(iReadOnlyFileSystem *fs);
    ~SoundSystem() override;

    std::vector<FormatInfo> GetSupportedFormats() override;

    SoundSettings GetSettings() override;
    void SetSettings(SoundSettings value) override;

    std::unique_ptr<iHandleApi> GetHandleApi() const override;

    //iChangeCallback
    // Settings::ApplyMethod ValueChanged(const std::string &key, Settings *siface) override;
protected:
    std::unique_ptr<WorkThread> workThread;
};

} // namespace MoonGlare::SoundSystem
