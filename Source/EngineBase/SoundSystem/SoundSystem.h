#pragma once

#include "iSoundSystem.h"
#include "WorkThread.h"

namespace MoonGlare::SoundSystem {

class SoundSystem : public iSoundSystem {
public:
    SoundSystem();
    virtual ~SoundSystem();

    Settings GetSettings() override;
    void SetSettings(Settings value) override;

    void Initialize(iFileSystem *fs) override;
    void Finalize() override;

    std::unique_ptr<iSound> OpenSound(const std::string &uri, bool start, SoundKind kind = SoundKind::Music) override;
    
    std::vector<FormatInfo> GetSupportedFormats() override;
protected:
    std::unique_ptr<WorkThread> workThread;
};

}
