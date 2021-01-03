#include "decoder.hpp"
#include "sound_system/configuration.hpp"
#include <orbit_logger.h>

#ifdef SOUNDSYSTEM_ENABLE_LIBVORBIS

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace MoonGlare::SoundSystem::Decoder {

struct OggVorbisFileDeleter {
    void operator()(OggVorbis_File *h) {
        ov_clear(h);
        delete h;
    }
};

class LibVorbisDecoder : public iDecoder {
public:
    LibVorbisDecoder() {}
    ~LibVorbisDecoder() {}

    static size_t VorbisRead(void *buffer, size_t size, size_t nmemb, void *ptr) {
        LibVorbisDecoder *decoder = (LibVorbisDecoder *)ptr;
        size_t r = std::min(decoder->fileData.size() - decoder->position, size * nmemb);
        memcpy(buffer, decoder->fileData.data() + decoder->position, r);
        decoder->position += r;
        return r;
    }

    static int VorbisSeek(void *ptr, ogg_int64_t offset, int whence) {
        LibVorbisDecoder *decoder = (LibVorbisDecoder *)ptr;
        switch (whence) {
        case SEEK_SET:
            decoder->position = offset;
            return 0;
        case SEEK_CUR:
            decoder->position += offset;
            return 0;
        case SEEK_END:
            decoder->position = decoder->fileData.size() - offset;
            return 0;
        default:
            return 1;
        }
    }
    static int VorbisClose(void *ptr) { return 0; }
    static long VorbisTell(void *ptr) {
        LibVorbisDecoder *decoder = (LibVorbisDecoder *)ptr;
        return static_cast<long>(decoder->position);
    }

    bool SetData(std::string data, const std::string &fn) override {
        fileData.swap(data);
        fileName = fn;
        handle.reset();
        return true;
    }

    bool Reset() override {
        format = 0;
        streamRate = 0;
        position = 0;
        handle.reset(new OggVorbis_File());

        ov_callbacks callbacks = {};
        callbacks.read_func = &VorbisRead;
        callbacks.close_func = &VorbisClose;
        callbacks.seek_func = &VorbisSeek;
        callbacks.tell_func = &VorbisTell;

        if (ov_open_callbacks((void *)this, handle.get(), NULL, 0, callbacks) < 0) {
            AddLogf(Error, "Not an ogg/vorbis stream! [%s]", fileName.c_str());
            handle.reset();
            return false;
        }

        vorbis_info vi = *ov_info(handle.get(), -1);
        streamRate = vi.rate;
        if (vi.channels == 2) {
            //if (v == 16)
            format = AL_FORMAT_STEREO16;
            //if (bps == 8)
            //format = AL_FORMAT_STEREO8;
        } else {
            //if (bps == 16)
            format = AL_FORMAT_MONO16;
            //if (bps == 8)
            //    format = AL_FORMAT_MONO8;
        }

        if (!decodeBuffer)
            decodeBuffer.reset(new char[kDesiredBufferSize]);

        return true;
    }

    static constexpr uint32_t StepSize = 16 * 1024;
    static constexpr int SampleSize = 2;

    DecodeState DecodeBuffer(SoundBuffer buffer, uint64_t *decodedBytes) override {
        if (!handle)
            return DecodeState::Error;

        char *buf = decodeBuffer.get();
        int bitstream = 0;
        uint32_t totalSize = 0;

        while (totalSize < kDesiredBufferSize) {
            size_t remain = kDesiredBufferSize - totalSize;
            long ret = ov_read(handle.get(), buf + totalSize, static_cast<int>(remain), 0, SampleSize, 1, &bitstream);
            if (ret > 0) {
                totalSize += ret;
                continue;
            }
            if (ret == 0) {
                break;
            }

            AddLogf(Error, "Corrupted stream! code:%d [%s]", ret, fileName.c_str());
            return DecodeState::Error;
        }
        if (decodedBytes) {
            *decodedBytes = totalSize;
        }

        if (totalSize == 0) {
            return DecodeState::Completed;
        }

        //AddLogf(Debug, "Decoded buffer size: %d", totalSize);
        alBufferData(buffer, format, buf, totalSize, streamRate);

        if (totalSize < kDesiredBufferSize)
            return DecodeState::LastBuffer;

        return DecodeState::Continue;
    }

    float GetDuration() const override { return static_cast<float>(ov_time_total(handle.get(), -1)); }

private:
    std::string fileData;
    std::string fileName;
    std::unique_ptr<OggVorbis_File, OggVorbisFileDeleter> handle;
    size_t position = 0;

    ALenum format = 0;
    ALuint streamRate = 0;

    std::unique_ptr<char[]> decodeBuffer;
};

//-------------------------------------------------------------

class LibVorbisDecoderFactory : public iDecoderFactory {
public:
    LibVorbisDecoderFactory() {}
    ~LibVorbisDecoderFactory() {}
    std::unique_ptr<iDecoder> CreateDecoder() override { return std::make_unique<LibVorbisDecoder>(); }
};

} // namespace MoonGlare::SoundSystem::Decoder

#endif

//-------------------------------------------------------------

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetLibVorbisDecoderInfo() {
#ifndef SOUNDSYSTEM_ENABLE_LIBVORBIS
    return {};
#else
    DecoderInfo di;

    static auto factory = std::make_shared<LibVorbisDecoderFactory>();
    di.decoderFactory = factory;

    di.supportedFormats = std::vector<FormatInfo>{
        FormatInfo{"ogg", "ogg", "libvorbis"},
    };
    return {di};
#endif
}

} // namespace MoonGlare::SoundSystem::Decoder
