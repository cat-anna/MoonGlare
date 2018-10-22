#include "libModPlugDecoder.h"
#include "../Configuration.h"

#ifndef SOUNDSYSTEM_DISABLE_WAVE

namespace MoonGlare::SoundSystem::Decoder {

enum class ChunkId : uint32_t {
    RIFF = 0x46464952,  //'RIFF'
    DATA = 0x61746164,  //'DATA'
    fmt  = 0x20746D66,  //'fmt '
};

struct chunk_t {
    ChunkId ID;
    unsigned long size;  //Chunk data bytes
};

struct wav_header_t {
    ChunkId chunkID; //"RIFF" = 0x46464952
    unsigned long chunkSize; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
    char format[4]; //"WAVE" = 0x45564157
    ChunkId subchunk1ID; //"fmt " = 0x20746D66
    unsigned long subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    //[WORD wExtraFormatBytes;]
    //[Extra format bytes]
};

class WaveDecoder : public iDecoder {
public:
    WaveDecoder() { }
    ~WaveDecoder() { }

    bool SetData(StarVFS::ByteTable data, const std::string &fn)  override {
        fileData.swap(data);
        fileName = fn;
        return true;
    }

    bool Reset() override {
        header = *reinterpret_cast<wav_header_t*>(fileData.get());
        position = sizeof(header);

        format = 0;
        if (header.numChannels == 2) {
            if (header.bitsPerSample == 16)
                format = AL_FORMAT_STEREO16;
            if (header.bitsPerSample == 8)
                format = AL_FORMAT_STEREO8;
        }
        else {
            if (header.bitsPerSample == 16)
                format = AL_FORMAT_MONO16;
            if (header.bitsPerSample == 8)
                format = AL_FORMAT_MONO8;
        }
        
        if (format == 0) {
            AddLogf(Error, "Unsupported wave format: %s", fileName.c_str());
            return false;
        }

        const char *data = (char*)fileData.get();
        size_t bytes = 0;
        for (size_t pos = sizeof(header); pos < fileData.byte_size(); ) {
            const chunk_t chunk = *reinterpret_cast<const chunk_t*>(data + pos);
            const char *dataPtr = data + pos + sizeof(chunk);
            pos += sizeof(chunk);
            pos += chunk.size;

            if (chunk.ID == ChunkId::DATA)
                bytes += chunk.size;
        }
        duration = static_cast<float>(bytes) / static_cast<float>(header.sampleRate * header.numChannels * (header.bitsPerSample / 8));
        return true;
    }

    DecodeState DecodeBuffer(SoundBuffer buffer, uint32_t *decodedBytes) override {
        if (!fileData || format == 0)
            return DecodeState::Error;

        bool gotData = false;

        const char *data = (char*)fileData.get();
        while (position < fileData.byte_size()) {
            const chunk_t chunk = *reinterpret_cast<const chunk_t*>(data + position);
            const char *dataPtr = data + position + sizeof(chunk);

            if (chunk.ID == ChunkId::DATA) {
                if (gotData)
                    return DecodeState::Continue;

                alBufferData(buffer, format, dataPtr, chunk.size, header.sampleRate);
                if (decodedBytes)
                    *decodedBytes = static_cast<uint32_t>(chunk.size);
                gotData = true;
                //AddLogf(Debug, "Decoded buffer size: %u", chunk.size);
            }
            position += sizeof(chunk);
            position += chunk.size;
        }

        if (!gotData)
            return DecodeState::Completed;
        return DecodeState::LastBuffer;
    }

    float GetDuration() const override {
        return duration;
    }

private:
    StarVFS::ByteTable fileData;
    std::string fileName;
    size_t position = 0;
    wav_header_t header = {};
    ALenum format = 0;
    float duration = -1;
};

//-------------------------------------------------------------

class WaveDecoderFactory : public iDecoderFactory {
public:
    std::unique_ptr<iDecoder> CreateDecoder() override {
        return std::make_unique<WaveDecoder>();
    }
};

#endif

}

//-------------------------------------------------------------

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetWaveDecoderInfo() {
#ifdef SOUNDSYSTEM_DISABLE_WAVE
    return {};
#else
    DecoderInfo di;

    static auto factory = std::make_shared<WaveDecoderFactory>();
    di.decoderFactory = factory;

    di.supportedFormats = std::vector<FormatInfo>{
        FormatInfo{ "wav", "wave", "wave decoder" },
        FormatInfo{ "wave", "wave", "wave decoder" },
    };
    return { di };
#endif
}

}
