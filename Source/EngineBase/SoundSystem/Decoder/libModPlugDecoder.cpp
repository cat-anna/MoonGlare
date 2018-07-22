#include "libModPlugDecoder.h"
#include "../Configuration.h"


#ifndef SOUNDSYSTEM_DISABLE_LIBMODPLUG

#if !__has_include("libmodplug/modplug.h")
#define SOUNDSYSTEM_DISABLE_LIBMODPLUG
#endif

#endif

#ifndef SOUNDSYSTEM_DISABLE_LIBMODPLUG

#include <libmodplug/modplug.h>

namespace MoonGlare::SoundSystem::Decoder {

struct ModPlugFileDeleter {
    void operator()(ModPlugFile*f) {
        ModPlug_Unload(f);
    }
};

class LibModPlugDecoder : public iDecoder {
public:
    LibModPlugDecoder() { }
    ~LibModPlugDecoder() { }

    bool SetData(StarVFS::ByteTable data, const std::string &fn)  override {
        modPlugFile.reset();
        fileData.swap(data);
        fileName = fn;
        return true;
    }

    bool Reset()  override {
        modPlugFile.reset();
        if (fileData.empty())
            return false;

        modPlugFile.reset(ModPlug_Load(fileData.get(), fileData.byte_size()));
        if (!modPlugFile) {
            AddLogf(Error, "Cannot load file : %s", fileName.c_str());
            return false;
        }
        ModPlug_GetSettings(&settings);
        if(!decodeBuffer)
            decodeBuffer.reset(new char[Configuration::DesiredBufferSize]);
        return true;
    }

    DecodeState DecodeBuffer(SoundBuffer buffer, uint32_t *decodedBytes) override {
        if (!modPlugFile)
            return DecodeState::Error;

        static const uint32_t StepSize = 8 * 1024;

        char *buf = decodeBuffer.get();
        uint32_t totalSize = 0;
        while (totalSize < Configuration::DesiredBufferSize) {
            uint32_t toRead = std::min(StepSize, Configuration::DesiredBufferSize - totalSize);
            int r = ModPlug_Read(modPlugFile.get(), buf + totalSize, toRead);
            if (r == 0)
                break;
            totalSize += r;
        }
        //AddLogf(Debug, "Decoded buffer size: %u", totalSize);
        if(totalSize == 0)
            return DecodeState::Completed;

        if (decodedBytes != nullptr)
            *decodedBytes = totalSize;

        ALenum format = AL_FORMAT_STEREO16;

        alBufferData(buffer, format, buf, totalSize, settings.mFrequency);
        if (totalSize < sizeof(Configuration::DesiredBufferSize))
            return DecodeState::LastBuffer;

        return DecodeState::Continue;
    }

private:
    StarVFS::ByteTable fileData;
    std::string fileName;
    ModPlug_Settings settings = {};

    std::unique_ptr<ModPlugFile, ModPlugFileDeleter> modPlugFile;

    std::unique_ptr<char[]> decodeBuffer;
};

//-------------------------------------------------------------

class LibModPlugDecoderFactory : public iDecoderFactory {
public:
    std::unique_ptr<iDecoder> CreateDecoder() override {
        return std::make_unique<LibModPlugDecoder>();
    }
};

}

#endif

//-------------------------------------------------------------

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetLibModPlugDecoderInfo() {
#ifdef SOUNDSYSTEM_DISABLE_LIBMODPLUG
    return {};
#else
    DecoderInfo di;

    static auto factory = std::make_shared<LibModPlugDecoderFactory>();
    di.decoderFactory = factory;

    di.supportedFormats = std::vector<FormatInfo>{
        FormatInfo{ "xm", "XM", "libmodplug" },
        FormatInfo{ "mod", "mod", "libmodplug" },
        FormatInfo{ "it", "it", "libmodplug" },
    };
    return { di };
#endif
}

}
