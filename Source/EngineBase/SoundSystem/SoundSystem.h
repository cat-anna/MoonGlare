#pragma once

#include "iSoundSystem.h"
#include "WorkThread.h"

#include <EngineBase/Settings.h>

namespace MoonGlare::SoundSystem {

class SoundSystem 
    : public iSoundSystem
    , public Settings::iChangeCallback
    , public std::enable_shared_from_this<SoundSystem> {
public:
    SoundSystem();
    virtual ~SoundSystem();

    SoundSettings GetSettings() override;
    void SetSettings(SoundSettings value) override;
    
    void Initialize(InterfaceMap &ifmap) override;
    void Finalize() override;

    HandleApi GetHandleApi() override;
    
    std::vector<FormatInfo> GetSupportedFormats() override;

    //iChangeCallback
    Settings::ApplyMethod ValueChanged(const std::string &key, Settings* siface) override;
protected:
    std::unique_ptr<WorkThread> workThread;
};

}
