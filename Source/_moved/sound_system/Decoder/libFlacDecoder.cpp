#include "libModPlugDecoder.h"
#include "../Configuration.h"


#ifndef SOUNDSYSTEM_DISABLE_LIBFLAC

#if !__has_include("FLAC/stream_decoder.h")
#define SOUNDSYSTEM_DISABLE_LIBFLAC
#endif

#endif

#ifndef SOUNDSYSTEM_DISABLE_LIBFLAC

#include "FLAC/stream_decoder.h"


namespace MoonGlare::SoundSystem::Decoder {

struct FLACStreamDecoderFileDeleter {
    void operator()(FLAC__StreamDecoder *h) {
        FLAC__stream_decoder_delete(h);
    }
};

class LibFlacDecoder : public iDecoder {
public:
    LibFlacDecoder() { }
    ~LibFlacDecoder() { }

    static FLAC__StreamDecoderReadStatus FlacRead(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data) {
        LibFlacDecoder *This = (LibFlacDecoder*)client_data;
        size_t remain = This->fileData.byte_size() - This->position;
        if (remain == 0)
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;

        if (*bytes > 0) {
            *bytes = std::min(*bytes, remain);
            memcpy(buffer, This->fileData.get() + This->position, *bytes);
            This->position += *bytes;
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    static void FlacMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
        LibFlacDecoder *This = (LibFlacDecoder*)client_data;

        if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
            //unsigned total_samples = metadata->data.stream_info.total_samples;
            unsigned channels = metadata->data.stream_info.channels;
            unsigned bps = metadata->data.stream_info.bits_per_sample;
            auto streamRate = metadata->data.stream_info.sample_rate;
            auto totalSamples = metadata->data.stream_info.total_samples;
            This->duration = static_cast<float>(totalSamples) / static_cast<float>(streamRate);
            int format = 0;
            if (channels == 2) {
                if (bps == 16)
                    format = AL_FORMAT_STEREO16;
                if (bps == 8)
                    format = AL_FORMAT_STEREO8;
            }
            else {
                if (bps == 16)
                    format = AL_FORMAT_MONO16;
                if (bps == 8)
                    format = AL_FORMAT_MONO8;
            }
            This->format = format;
            This->streamRate = streamRate;
        }
    }

    static FLAC__StreamDecoderWriteStatus FlacWrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) {
        LibFlacDecoder *This = (LibFlacDecoder*)client_data;

        if (frame->header.channels > 2) { // 0 case is ignored
            AddLogf(Error, "Invalid flac stream channel count!");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }

        auto &pendingBytes = This->pendingBytes;
        char* outBuffer = This->decodeBuffer.get();
        unsigned bytesPerSample = frame->header.bits_per_sample / 8;

        unsigned bytesPerChannel = frame->header.channels * bytesPerSample;

        for (unsigned i = 0; i < frame->header.blocksize; i++) {
            for (unsigned c = 0; c < frame->header.channels; ++c) {
                auto v = buffer[c][i];
                memcpy(outBuffer + pendingBytes, &v, bytesPerSample);
                pendingBytes += bytesPerSample;
            }
            if(pendingBytes > Configuration::DesiredBufferSize - bytesPerChannel) {
                //AddLogf(Debug, "Decoded buffer size: %d", pendingBytes);
                alBufferData(This->currentBufferHandle, This->format, outBuffer, pendingBytes, This->streamRate);
                This->bufferCompleted = true;
                This->filledBytes = pendingBytes;
                pendingBytes = 0;
            }
        }

        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }

    static void FlacError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
        //LibFlacDecoder *This = (LibFlacDecoder*)client_data;
        AddLogf(Error, "FLAC decoder error: %s", FLAC__StreamDecoderErrorStatusString[status]);
    }

    bool SetData(StarVFS::ByteTable data, const std::string &fn)  override {
        fileData.swap(data);
        fileName = fn;
        handle.reset();
        return true;
    }

    bool Reset() override {
        format = 0;
        streamRate = 0;
        position = 0;
        pendingBytes = 0;
        handle.reset(FLAC__stream_decoder_new());
        if (!handle) {
            fprintf(stderr, "ERROR: allocating decoder\n");
            AddLogf(Error, "Cannot allocate flac stream decoder!");
            return false;
        }

        FLAC__stream_decoder_set_md5_checking(handle.get(), true);

        auto init_status = FLAC__stream_decoder_init_stream(handle.get(), &FlacRead, nullptr, nullptr, nullptr, nullptr, &FlacWrite, &FlacMetadata, &FlacError, this);
        if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
            AddLogf(Error, "Flac decoder init failed! msg: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
            handle.reset();
            return false;
        }

        if (!decodeBuffer)
            decodeBuffer.reset(new char[Configuration::DesiredBufferSize]);

        if (!FLAC__stream_decoder_process_until_end_of_metadata(handle.get()))
            return false;

        return true;
    }

    static constexpr uint32_t StepSize = 16 * 1024;
    static constexpr int SampleSize = 2;

    DecodeState DecodeBuffer(SoundBuffer buffer, uint32_t *decodedBytes) override {
        if (!handle)
            return DecodeState::Error;

        currentBufferHandle = buffer;
        bufferCompleted = false;
         
        while (!bufferCompleted) {
            auto state = FLAC__stream_decoder_get_state(handle.get());
            if (state == FLAC__STREAM_DECODER_END_OF_STREAM)
                break;

            if (!FLAC__stream_decoder_process_single(handle.get()))
                return DecodeState::Error;
        }

        if (!bufferCompleted && pendingBytes > 0) {
            alBufferData(buffer, format, decodeBuffer.get(), pendingBytes, streamRate);
            bufferCompleted = true;
            filledBytes = pendingBytes;
            //AddLogf(Debug, "Decoded buffer size: %d", filledBytes);
        }

        if (decodedBytes)
            *decodedBytes = filledBytes;

        if (filledBytes == 0)
            return DecodeState::Completed;

        if (filledBytes < Configuration::DesiredBufferSize)
            return DecodeState::LastBuffer;
        
        return DecodeState::Continue;
    }

    float GetDuration() const override {
        return duration;
    }
private:
    StarVFS::ByteTable fileData;
    std::string fileName;
    std::unique_ptr<FLAC__StreamDecoder, FLACStreamDecoderFileDeleter> handle;
    size_t position = 0;

    ALenum format = 0;
    ALuint streamRate = 0;
    float duration = -1;

    size_t pendingBytes = 0;
    size_t filledBytes = 0;
    bool bufferCompleted = false;
    SoundBuffer currentBufferHandle = SoundBuffer(0);

    std::unique_ptr<char[]> decodeBuffer;
};

//-------------------------------------------------------------

class LibFlacDecoderFactory : public iDecoderFactory {
public:
    LibFlacDecoderFactory() { }
    ~LibFlacDecoderFactory() { }
    std::unique_ptr<iDecoder> CreateDecoder() override {
        return std::make_unique<LibFlacDecoder>();
    }
};

}

#endif

//-------------------------------------------------------------

namespace MoonGlare::SoundSystem::Decoder {

std::vector<DecoderInfo> GetLibFlacDecoderInfo() {
#ifdef SOUNDSYSTEM_DISABLE_LIBFLAC
    return {};
#else
    DecoderInfo di;

    static auto factory = std::make_shared<LibFlacDecoderFactory>();
    di.decoderFactory = factory;

    di.supportedFormats = std::vector<FormatInfo>{
        FormatInfo{ "flac", "flac", "libflac" },
    };
    return { di };
#endif
}

}
