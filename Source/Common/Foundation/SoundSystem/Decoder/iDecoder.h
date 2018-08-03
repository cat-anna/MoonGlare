#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../iSoundSystem.h"
#include "../OpenAl.h"

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

    virtual void Initialize() { }
    virtual void finalize() { }
    virtual void SetSetings(const SoundSettings& settings) {}
protected:
};

enum class DecodeState {
    Continue, LastBuffer, Completed, Error,
};
                   
class iDecoder {
public:
    virtual ~iDecoder() {}

    virtual bool SetData(StarVFS::ByteTable data, const std::string &fileName) = 0;
    virtual bool Reset() = 0;
    virtual DecodeState DecodeBuffer(SoundBuffer buffer, uint32_t *decodedBytes = nullptr) = 0;
    virtual float GetDuration() const = 0;
};

}
         