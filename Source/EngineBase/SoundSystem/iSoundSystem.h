#pragma once

#include <Foundation/iFileSystem.h>
#include "HandleApi.h"

namespace MoonGlare::SoundSystem {

enum class BitDepth {
    Bits16, Bits8,
};

struct Settings {
    bool enabled = true;
    float masterVolume = 1.0f;
    float musicVolume = 0.7f;
    float effectVolume = 1.0f;
    BitDepth bitDepth = BitDepth::Bits16;
};

struct FormatInfo {
    std::string fileExtension;
    std::string formatName;
    std::string decoderName;
};

class iSoundSystem {
public:
    virtual ~iSoundSystem() {}

    virtual Settings GetSettings() = 0;
    virtual void SetSettings(Settings value) = 0;

    virtual void Initialize(iFileSystem *fs) = 0;
    virtual void Finalize() = 0;

    virtual HandleApi GetHandleApi() = 0;

    virtual std::vector<FormatInfo> GetSupportedFormats() = 0;

    static iSoundSystem* CreateNew();
};

}
