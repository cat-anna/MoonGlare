#include "decoder.hpp"
#include "sound_system/configuration.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

#ifdef SOUNDSYSTEM_ENABLE_LIBMPG123

#include <mpg123.h>

namespace MoonGlare::SoundSystem::Decoder {

struct mpg123HandleDeleter {
    void operator()(mpg123_handle *h) { mpg123_delete(h); }
};

class LibMpg123Decoder : public iDecoder {
public:
    LibMpg123Decoder() {}
    ~LibMpg123Decoder() {}

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
        int ret;
        handle.reset(mpg123_new(nullptr, &ret));
        if (!handle) {
            AddLogf(Error, "LibMpg123 handle open error! code: %d", ret);
            return false;
        }

        if (mpg123_open_feed(handle.get()) != MPG123_OK) {
            handle.reset();
            AddLogf(Error, "LibMpg123 feed open error! code: %d", ret);
            return false;
        }

        if (!decodeBuffer)
            decodeBuffer.reset(new char[kDesiredBufferSize]);

        mpg123_param(handle.get(), MPG123_VERBOSE, 2, 0); /* Brabble a bit about the parsing/decoding. */

        return true;
    }

    static const uint64_t StepSize = 16 * 1024;

    DecodeState DecodeBuffer(SoundBuffer buffer, uint64_t *decodedBytes) override {
        if (!handle)
            return DecodeState::Error;

        char *buf = decodeBuffer.get();

        uint64_t totalSize = 0;
        bool finished = false;
        while (!finished && totalSize < kDesiredBufferSize) {
            size_t toRead = std::min(StepSize, static_cast<uint64_t>(fileData.size() - position));
            size_t remain = std::min(StepSize, kDesiredBufferSize - totalSize);

            size_t done = 0;
            int r = mpg123_decode(handle.get(), (const unsigned char *)fileData.c_str() + position, toRead,
                                  (unsigned char *)buf + totalSize, remain, &done);

            totalSize += static_cast<uint64_t>(done);
            position += toRead;

            switch (r) {
            case MPG123_NEED_MORE:
                if (toRead == 0) {
                    finished = true;
                }
            case MPG123_DONE:
            case MPG123_OK:
                continue;
            case MPG123_NEW_FORMAT: {
                if (format != 0) {
                    AddLogf(Error, "mp3 stream changed format! [%s]", fileName.c_str());
                    return DecodeState::Error;
                }
                int channels, enc;
                long rate;
                mpg123_getformat(handle.get(), &rate, &channels, &enc);
                streamRate = rate;
                if (channels == 2) {
                    if (enc & MPG123_ENC_16)
                        format = AL_FORMAT_STEREO16;
                    if (enc & MPG123_ENC_8)
                        format = AL_FORMAT_STEREO8;
                } else {
                    if (enc & MPG123_ENC_16)
                        format = AL_FORMAT_MONO16;
                    if (enc & MPG123_ENC_8)
                        format = AL_FORMAT_MONO8;
                }
                if (format == 0) {
                    AddLogf(Error, "Invalid mp3 stream format! [%s]", fileName.c_str());
                    return DecodeState::Error;
                }
                break;
            default:
                AddLogf(Error, "mp3 stream decode error! code:%d [%s]", r, fileName.c_str());
                return DecodeState::Error;
            }
            }
        }

        //AddLogf(Debug, "Decoded buffer size: %u", totalSize);
        if (totalSize == 0)
            return DecodeState::Completed;

        if (decodedBytes != nullptr)
            *decodedBytes = totalSize;

        if (format == 0 || streamRate == 0) {
            AddLogf(Error, "Invalid mp3 stream format! [%s]", fileName.c_str());
            return DecodeState::Error;
        }
        alBufferData(buffer, format, buf, static_cast<ALsizei>(totalSize), streamRate);

        if (totalSize < sizeof(kDesiredBufferSize))
            return DecodeState::LastBuffer;

        return DecodeState::Continue;
    }

    float GetDuration() const override { return -1.0f; }

private:
    std::string fileData;
    std::string fileName;
    std::unique_ptr<mpg123_handle, mpg123HandleDeleter> handle;
    ALenum format = 0;
    ALuint streamRate = 0;
    size_t position = 0;

    std::unique_ptr<char[]> decodeBuffer;
};

//-------------------------------------------------------------

class LibMpg123DecoderFactory : public iDecoderFactory {
public:
    LibMpg123DecoderFactory() {
        auto r = mpg123_init();
        if (r != MPG123_OK)
            throw std::runtime_error(fmt::format("libmpg123 initialization error! code: {}", r));
    }
    ~LibMpg123DecoderFactory() { mpg123_exit(); }
    std::unique_ptr<iDecoder> CreateDecoder() override { return std::make_unique<LibMpg123Decoder>(); }
};

} // namespace MoonGlare::SoundSystem::Decoder

#endif

//-------------------------------------------------------------

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetLibMpg123DecoderInfo() {
#ifndef SOUNDSYSTEM_ENABLE_LIBMPG123
    return {};
#else
    DecoderInfo di;

    static auto factory = std::make_shared<LibMpg123DecoderFactory>();
    di.decoderFactory = factory;

    di.supportedFormats = std::vector<FormatInfo>{
        FormatInfo{"mp3", "mp3", "libmpg123"},
    };
    return {di};
#endif
}

} // namespace MoonGlare::SoundSystem::Decoder
