#pragma once

#include <Foundation/iFileSystem.h>

namespace MoonGlare::SoundSystem {

enum class BitDepth {
    Bits16, Bits8,
};

enum class SoundKind {
    Music, Effect,
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

class iSound {
public:
    virtual ~iSound() {}

    virtual void Play() = 0;
    //virtual void Pause() = 0;
    //virtual void Stop() = 0;
    //virtual uint32_t length() const = 0; // in ms
    //virtual bool IsPlaying() const = 0;
};

class iSoundSystem {
public:
    virtual ~iSoundSystem() {}

    virtual Settings GetSettings() = 0;
    virtual void SetSettings(Settings value) = 0;

    virtual void Initialize(iFileSystem *fs) = 0;
    virtual void Finalize() = 0;

    virtual std::unique_ptr<iSound> OpenSound(const std::string &uri, bool start = true, SoundKind kind = SoundKind::Music) = 0;

    virtual std::vector<FormatInfo> GetSupportedFormats() = 0;

    static iSoundSystem* CreateNew();
};

}
