#pragma once

#include "OpenAl.hpp"
#include "sound_system/iSoundSystem.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MoonGlare::SoundSystem::Decoder {

class iDecoderFactory;
class iDecoder;

struct DecoderInfo {
    std::vector<FormatInfo> supportedFormats;
    std::shared_ptr<iDecoderFactory> decoderFactory;
};

class iDecoderFactory {
public:
    static std::vector<DecoderInfo> GetDecoders();
    virtual ~iDecoderFactory() {}

    virtual std::unique_ptr<iDecoder> CreateDecoder() = 0;

    virtual void SetSetings(const SoundSettings &settings) {}

protected:
};

enum class DecodeState {
    Continue,
    LastBuffer,
    Completed,
    Error,
};

class iDecoder {
public:
    virtual ~iDecoder() {}

    virtual bool SetData(std::string data, const std::string &fileName) = 0;
    virtual bool Reset() = 0;
    virtual DecodeState DecodeBuffer(SoundBuffer buffer, uint64_t *decodedBytes = nullptr) = 0;
    virtual float GetDuration() const = 0;
};

} // namespace MoonGlare::SoundSystem::Decoder
