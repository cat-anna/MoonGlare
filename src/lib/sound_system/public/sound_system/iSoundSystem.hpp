#pragma once

#include "iHandleApi.hpp"
#include "readonly_file_system.h"

namespace MoonGlare::SoundSystem {

enum class BitDepth {
    Bits16 = 16,
    Bits8 = 8,
};

struct SoundSettings {
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
    virtual ~iSoundSystem() = default;
    virtual std::vector<FormatInfo> GetSupportedFormats() = 0;

    virtual SoundSettings GetSettings() = 0;
    virtual void SetSettings(SoundSettings value) = 0;

    virtual std::unique_ptr<iHandleApi> GetHandleApi() const = 0;

    static std::shared_ptr<iSoundSystem> Create(iReadOnlyFileSystem *fs);
};

} // namespace MoonGlare::SoundSystem
