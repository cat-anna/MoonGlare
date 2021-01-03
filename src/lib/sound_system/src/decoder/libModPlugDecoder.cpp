#include "decoder.hpp"
#include "sound_system/configuration.hpp"
#include <orbit_logger.h>

#ifdef SOUNDSYSTEM_ENABLE_LIBMODPLUG

#include <libmodplug/modplug.h>

namespace MoonGlare::SoundSystem::Decoder {

struct ModPlugFileDeleter {
    void operator()(ModPlugFile *f) { ModPlug_Unload(f); }
};

class LibModPlugDecoder : public iDecoder {
public:
    LibModPlugDecoder() {}
    ~LibModPlugDecoder() {}

    bool SetData(std::string data, const std::string &fn) override {
        modPlugFile.reset();
        fileData.swap(data);
        fileName = fn;
        return true;
    }

    bool Reset() override {
        modPlugFile.reset();
        if (fileData.empty())
            return false;

        modPlugFile.reset(ModPlug_Load(fileData.c_str(), static_cast<int>(fileData.size())));
        if (!modPlugFile) {
            AddLogf(Error, "Cannot load file : %s", fileName.c_str());
            return false;
        }
        ModPlug_GetSettings(&settings);
        if (!decodeBuffer)
            decodeBuffer.reset(new char[kDesiredBufferSize]);
        return true;
    }

    DecodeState DecodeBuffer(SoundBuffer buffer, uint64_t *decodedBytes) override {
        if (!modPlugFile)
            return DecodeState::Error;

        static const uint32_t StepSize = 8 * 1024;

        char *buf = decodeBuffer.get();
        uint32_t totalSize = 0;
        while (totalSize < kDesiredBufferSize) {
            uint32_t toRead = std::min(StepSize, kDesiredBufferSize - totalSize);
            int r = ModPlug_Read(modPlugFile.get(), buf + totalSize, toRead);
            if (r == 0)
                break;
            totalSize += r;
        }
        //AddLogf(Debug, "Decoded buffer size: %u", totalSize);
        if (totalSize == 0)
            return DecodeState::Completed;

        if (decodedBytes != nullptr)
            *decodedBytes = totalSize;

        ALenum format = AL_FORMAT_STEREO16;

        alBufferData(buffer, format, buf, totalSize, settings.mFrequency);
        if (totalSize < sizeof(kDesiredBufferSize))
            return DecodeState::LastBuffer;

        return DecodeState::Continue;
    }

    float GetDuration() const override { return static_cast<float>(ModPlug_GetLength(modPlugFile.get())) / 1000.0f; }

private:
    std::string fileData;
    std::string fileName;
    ModPlug_Settings settings = {};

    std::unique_ptr<ModPlugFile, ModPlugFileDeleter> modPlugFile;

    std::unique_ptr<char[]> decodeBuffer;
};

//-------------------------------------------------------------

class LibModPlugDecoderFactory : public iDecoderFactory {
public:
    std::unique_ptr<iDecoder> CreateDecoder() override { return std::make_unique<LibModPlugDecoder>(); }
};

} // namespace MoonGlare::SoundSystem::Decoder

#endif

//-------------------------------------------------------------

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetLibModPlugDecoderInfo() {
#ifndef SOUNDSYSTEM_ENABLE_LIBMODPLUG
    return {};
#else
    DecoderInfo di;

    static auto factory = std::make_shared<LibModPlugDecoderFactory>();
    di.decoderFactory = factory;

    di.supportedFormats = std::vector<FormatInfo>{
        FormatInfo{"xm", "XM", "libmodplug"},
        FormatInfo{"mod", "mod", "libmodplug"},
        FormatInfo{"it", "it", "libmodplug"},
    };
    return {di};
#endif
}

} // namespace MoonGlare::SoundSystem::Decoder
