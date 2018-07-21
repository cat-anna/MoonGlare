#include "WorkThread.h"
#include "Sound.h"

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare::SoundSystem {

struct WorkThread::Storage {
    using conf = MoonGlare::SoundSystem::Configuration;

    template<typename T, size_t S>
    using StaticVector = Space::Container::StaticVector<T, S>;

    //BufferArray<SoundBuffer> allocatedBuffers;
    StaticVector<SoundBuffer, conf::MaxBuffers> standbyBuffers;
    //BufferArray<BufferState> bufferState;

    //SourceArray<SoundSource> allocatedSources;
    StaticVector<SourceIndex, conf::MaxSources> standbySources;
    StaticVector<SourceIndex, conf::MaxSources> activeSources;
    StaticVector<SourceState, conf::MaxSources> sourceState;

    StaticVector<SourceIndex, conf::SourceAcivationQueue> sourceAcivationQueue;

    uint32_t allocatedBuffersCount = 0;

    std::mutex sourceAcivationQueueMutex;
    std::mutex allocationMutex;
    using lock_guard = std::lock_guard<std::mutex>;

    iFileSystem * fileSystem = nullptr;

    enum class SourceProcessStatus {
        Continue, Remove,
    };

    void Step() {
        ActivateSources();
        for (size_t i = 0; i < activeSources.Allocated(); ) {
            SourceIndex si = activeSources[i];
            switch (ProcessSource(si)) {
            case SourceProcessStatus::Continue:
                ++i;
                continue;
            case SourceProcessStatus::Remove:
            default:
                __debugbreak();
                //todo
            }
        }
    }

    SourceProcessStatus ProcessSource(SourceIndex si) {
        SourceState &state = sourceState[si];
        bool done = false;
        while (!done) {
            done = true;
            switch (state.status) {
            case SourceStatus::InitPending:
                if (!InitializeSource(state)) {
                    state.status = SourceStatus::FinitPending;
                    return SourceProcessStatus::Continue;
                }
                break;
            case SourceStatus::Playing: 
                ProcessPlayState(state);
                break;
            case SourceStatus::Paused:
            case SourceStatus::Stopped:
                break;
            case SourceStatus::FinitPending:
                //todo
                break;

            case SourceStatus::Invalid:
            case SourceStatus::Standby:
            default:
                //invalid status
                //todo
                break;
            }

            switch (state.command) {
                case SourceCommand::ResumePlaying:
                    if (state.Playable()) {
                        state.status = SourceStatus::Playing;
                        if (state.status == SourceStatus::Paused) {
                            alSourcePlay(state.sourceHandle);
                        }
                    }
                    break;
                case SourceCommand::Pause:
                    if (state.status == SourceStatus::Playing) {
                        alSourcePause(state.sourceHandle);
                        state.status = SourceStatus::Paused;
                    }
                    break;
                case SourceCommand::StopPlaying:
                    if (state.Playable()) {
                        state.status = SourceStatus::Stopped;
                        state.decoder->Reset();
                        if (state.bufferCount > 0) {
                            //todo
                            __debugbreak();
                            assert(false);
                        }
                    }
                    break;
                case SourceCommand::Release:
                    return SourceProcessStatus::Remove;

                case SourceCommand::None:
                case SourceCommand::Finalize:

                default:
                    break;
            }

            state.command = SourceCommand::None;
        }
        return SourceProcessStatus::Continue;
    }

    void ProcessPlayState(SourceState & state) {
        auto loadbuffer = [this, &state](SoundBuffer sb) {
            Decoder::DecodeState s = Decoder::DecodeState::Completed;
            uint32_t bytes = 0;
            if (!state.streamFinished) {
                s = state.decoder->DecodeBuffer(sb, &bytes);
                state.processedBytes += bytes;
                state.streamFinished = s != Decoder::DecodeState::Continue;
            }
            if (s < Decoder::DecodeState::Completed) {
                alSourceQueueBuffers(state.sourceHandle, 1, &sb);
                state.queuedBuffers[state.firstBuffer] = sb;
                state.firstBuffer = (state.firstBuffer + 1) % conf::MaxBuffersPerSource;
                ++state.bufferCount;
            } else {
                ReleaseBuffer(sb);
            }
        };

        while (true) {
            ALint processedBuffers = 0;
            alGetSourcei(state.sourceHandle, AL_BUFFERS_PROCESSED, &processedBuffers);
            if (processedBuffers <= 0)
                break;

            ++state.processedBuffers;
            auto last = (state.firstBuffer + conf::MaxBuffersPerSource - state.bufferCount) % conf::MaxBuffersPerSource;
            SoundBuffer sb = state.queuedBuffers[last];
            alSourceUnqueueBuffers(state.sourceHandle, 1, &sb);
            loadbuffer(sb);          
        };

        ALint queuedBuffers = 0;
        alGetSourcei(state.sourceHandle, AL_BUFFERS_QUEUED, &queuedBuffers);

        bool start = false;
        if (queuedBuffers < conf::MaxBuffersPerSource && !state.streamFinished) {
            loadbuffer(GetNextBuffer());
            ++queuedBuffers;
            if (state.bufferCount == 1) {
                alSourcePlay(state.sourceHandle);
            }
        }

        if (queuedBuffers == 0) {
            if (state.streamFinished) {
                state.status = SourceStatus::Stopped;
                AddLogf(Debug, "Playing finished: %s processed buffers: %u bytes:%6.2fMib", state.uri.c_str(), state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
            } else {
                __debugbreak();
            }
        }
    }

    bool InitializeSource(SourceState & state) {
        if (!state.decoder) {
            return false;
        }

        StarVFS::ByteTable data;
        if (!fileSystem->OpenFile(data, state.uri)) {
            return false;
        }

        if (!state.decoder->SetData(std::move(data), state.uri)) {
            return false;
        }
        state.status = SourceStatus::Stopped;
        return state.decoder->Reset();
    }

    void ActivateSources() {
        if (sourceAcivationQueue.Allocated() > 0) {
            lock_guard lock(sourceAcivationQueueMutex);

            while (!sourceAcivationQueue.empty()) {
                auto ss = sourceAcivationQueue.pop(InvalidSoundSource);
                if(ss != InvalidSoundSource)
                    activeSources.push(ss);
            }
        }
    }

    void Initialize() {
        standbyBuffers.fill(InvalidSoundBuffer);
        standbySources.fill(InvalidSoundSource);
        activeSources.fill(InvalidSoundSource);
        //sourceState.fill(SourceState{});

        GenBuffers();
        GenSources();
    }
    void Finalize() {
        __debugbreak();
        //TODO
    }

    void ReleaseBuffer(SoundBuffer b) {
        if (b == InvalidSoundBuffer)
            return;
        alBufferData(b, AL_FORMAT_STEREO8, nullptr, 0, 8000);
        standbyBuffers.push(b);
    }
    void ReleaseSource(SourceIndex s) {
        if (s == InvalidSourceIndex)
            return;

        auto &state = sourceState[s];
        assert(state.status != SourceStatus::Invalid);
        alSourceStop(state.sourceHandle);
        for (uint16_t idx = 0; idx < state.bufferCount; ++idx) {
            uint16_t relidx = (state.firstBuffer + idx) % conf::MaxBuffersPerSource;
            ReleaseBuffer(state.queuedBuffers[relidx]);
        }
        state.bufferCount = state.firstBuffer = 0;
        state.command = SourceCommand::None;
        state.status = SourceStatus::Standby;
        state.decoder.reset();
        standbySources.push(s);
    }

    SourceIndex GetNextSource() {
        if (standbySources.empty()) {
            if (!GenSources())
                return InvalidSourceIndex;
        }
        auto si = standbySources.pop(InvalidSourceIndex);
        if (si == InvalidSourceIndex)
            return InvalidSourceIndex;
        auto &state = sourceState[si];
        assert(state.status == SourceStatus::Standby);
        assert(state.bufferCount == 0);
        assert(state.firstBuffer == 0);
        state.status = SourceStatus::InitPending;
        state.streamFinished = false;
        return si;
    }
    SoundSource GetNextBuffer() {
        if (standbyBuffers.empty())
            if(!GenSources())
                return InvalidSoundBuffer;
        return standbyBuffers.pop(InvalidSoundBuffer);
    }

    bool GenBuffers() {
        if (allocatedBuffersCount + conf::BufferGenCount > conf::MaxBuffers)
            return false;
        SoundBuffer arr[conf::BufferGenCount] = {};
        alGenBuffers(conf::BufferGenCount, arr);
        standbyBuffers.append_copy(arr, conf::BufferGenCount);
        allocatedBuffersCount += conf::BufferGenCount;
        return true;
    }
    bool GenSources() {
        size_t canAlloc = std::min((size_t)conf::SourceGenCount, conf::MaxSources - sourceState.Allocated());
        if (canAlloc == 0)
            return false;
        SoundSource arr[conf::SourceGenCount] = {};
        alGenSources(canAlloc, arr);
        for (size_t i = 0; i < canAlloc; ++i) {
            size_t index = 0;
            if (!sourceState.Allocate(index)) {
                __debugbreak;
                assert(false);
                throw false;
                //todo?
            }
            auto &ss = sourceState[index];
            standbySources.push(index);
            ss = {};
            ss.sourceHandle = arr[i];
            ss.status = SourceStatus::Standby;
        }
        return true;
    }
};

//-------------------------------------------

WorkThread::WorkThread(SoundSystem * owner, iFileSystem * fs) : soundSystem(owner), fileSystem(fs) {
}

WorkThread::~WorkThread() {
}

void WorkThread::Initialize() {
    if (thread.joinable())
        throw std::runtime_error("Already initialized");

    InitializeDevice();

    storage = std::make_unique<Storage>();
    storage->fileSystem = fileSystem;
    storage->Initialize();

    //decoderFactories
    for (auto &decoder : Decoder::iDecoderFactory::GetDecoders()) {
        for (auto format : decoder.supportedFormats) {
            decoderFactories[format.fileExtension] = decoder.decoderFactory;
            AddLogf(System, "Supported audio format: %s %s %s",
                format.fileExtension.c_str(),
                format.formatName.c_str(),
                format.decoderName.c_str());
        }
    }

    thread = std::thread([this]() { 
        threadState = ThreadState::Starting;
        ThreadMain(); 
        threadState = ThreadState::Finished;
    });
}

void WorkThread::Finalize() {
    if (threadState >= ThreadState::Running) {
        threadState = ThreadState::Stopping;
        thread.join();
        threadState = ThreadState::Stopped;
    }

    storage->Finalize();
    storage.reset();
    
    decoderFactories.clear();
    FinalizeDevice();
}

void WorkThread::ThreadMain() {
    OrbitLogger::ThreadInfo::SetName("SSWT");

    std::mutex mtx;
    threadState = ThreadState::Running;
    while (threadState == ThreadState::Running) {
        storage->Step();

        std::unique_lock<std::mutex> lock(mtx);
        threadWait.wait_for(lock, std::chrono::milliseconds(Configuration::ThreadStep));
    }
}

void WorkThread::InitializeDevice() {
    /* Open and initialize a device */
    ALCdevice *device = alcOpenDevice(nullptr);
    ALCcontext *ctx = alcCreateContext(device, nullptr);
    if (ctx == nullptr || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if (ctx != nullptr)
            alcDestroyContext(ctx);
        alcCloseDevice(device);

        throw std::runtime_error("Cannot open sound device");
    }

    const ALCchar *name = nullptr;
    if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if (!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);

    AddLogf(System, "OpenAl version: %d.%d", alGetInteger(ALC_MAJOR_VERSION), alGetInteger(ALC_MINOR_VERSION));
    AddLogf(System, "Sound device used: %s", name);
    AddLogf(System, "Sound device extensions: %s", alcGetString(device, ALC_EXTENSIONS));
}

void WorkThread::FinalizeDevice() {
    ALCcontext *ctx = alcGetCurrentContext();
    if (ctx == nullptr)
        return;

    ALCdevice *device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}

std::unique_ptr<iSound> WorkThread::OpenSound(const std::string &uri, bool start, SoundKind kind) {
    auto factory = FindFactory(uri);
    if (!factory)
        return nullptr;

    SourceIndex si;
    {
        Storage::lock_guard lock(storage->allocationMutex);
        si = storage->GetNextSource();
    }
    if (si == InvalidSourceIndex)
        return nullptr;

    auto &state = storage->sourceState[si];
    state.uri = uri;
    state.decoder = factory->CreateDecoder();
    state.status = SourceStatus::InitPending;
    if (start)
        state.command = SourceCommand::ResumePlaying;

    auto p = std::make_unique<StandaloneSoundPlayer>(si, &state);
    {
        Storage::lock_guard lock(storage->sourceAcivationQueueMutex);
        storage->sourceAcivationQueue.push(si);
    }
    return std::move(p);
}

std::shared_ptr<Decoder::iDecoderFactory> WorkThread::FindFactory(const std::string &uri) {
    const char *ext = strrchr(uri.c_str(), '.');
    if (!ext)
        return nullptr;
    
    auto it = decoderFactories.find(ext + 1);
    if (it == decoderFactories.end())
        return nullptr;

    return it->second;
}

}
