#include <chrono>

#include "WorkThread.h"
#include "Sound.h"

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare::SoundSystem {

struct WorkThread::Storage {
    using conf = MoonGlare::SoundSystem::Configuration;

    template<typename T, size_t S>
    using StaticVector = Space::Container::StaticVector<T, S>;

    StaticVector<SoundBuffer, conf::MaxBuffers> standbyBuffers;
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
        Continue, ReleaseAndRemove, Remove,
    };

    void Step() {
        ActivateSources();
        for (size_t i = 0; i < activeSources.Allocated(); ) {
            SourceIndex si = activeSources[i];
            switch (ProcessSource(si)) {
            case SourceProcessStatus::Continue:
                ++i;
                continue;
            case SourceProcessStatus::ReleaseAndRemove:
                ReleaseSource(si);
                [[fallthrough]];
            case SourceProcessStatus::Remove: {
                size_t lastIdx = activeSources.Allocated() - 1;
                if (i != lastIdx) {
                    activeSources[i] = activeSources[lastIdx];
                }
                activeSources.pop(InvalidSourceIndex);
                break;
            }
            default:
                __debugbreak();
                //todo
            }
        }
    }

    SourceProcessStatus ProcessSource(SourceIndex si) {
        SourceState &state = sourceState[(size_t)si];
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
                    break;
                case SourceStatus::Stopped:
                    if (state.releaseOnStop)
                        return SourceProcessStatus::ReleaseAndRemove;
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
                            AddLogf(Debug, "Playing resumed: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
                            state.sourceHandle.Play();
                        }
                    }
                    break;
                case SourceCommand::Pause:
                    if (state.status == SourceStatus::Playing) {
                        AddLogf(Debug, "Playing paused: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
                        state.sourceHandle.Pause();
                        state.status = SourceStatus::Paused;
                    }
                    break;
                case SourceCommand::StopPlaying:
                    if (state.Playable()) {
                        state.status = SourceStatus::Stopped;
                        state.sourceHandle.Stop();
                        state.decoder->Reset();
                        ReleaseSourceQueue(state);
                        AddLogf(Debug, "Playing stopped: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
                    }
                    break;
                //case SourceCommand::Release:
                    //return SourceProcessStatus::Remove;

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
        auto loadbuffer = [this, &state](SoundBuffer sb) ->bool {
            Decoder::DecodeState s = Decoder::DecodeState::Completed;
            uint32_t bytes = 0;
            if (!state.streamFinished) {
                s = state.decoder->DecodeBuffer(sb, &bytes);
                state.processedBytes += bytes;
                state.streamFinished = s != Decoder::DecodeState::Continue;
            }
            if (s < Decoder::DecodeState::Completed) {
                state.sourceHandle.QueueBuffer(sb);
                ++state.bufferCount;
                return true;
            } else {
                return false;
            }
        };

        while (true) {
            ALint processedBuffers = state.sourceHandle.GetProcessedBuffers();
            if (processedBuffers <= 0)
                break;

            ++state.processedBuffers;
            SoundBuffer sb = state.sourceHandle.UnqueueBuffer();
            if (!sb)
                break;
            --state.bufferCount;
            if (!loadbuffer(sb)) {
                ReleaseBuffer(sb);
            }
        };

        ALint queuedBuffers = state.sourceHandle.GetQueuedBuffers();

        if (queuedBuffers < conf::MaxBuffersPerSource && !state.streamFinished) {
            SoundBuffer b = GetNextBuffer();
            if(!loadbuffer(b))
                ReleaseBuffer(b);
            ++queuedBuffers;
            if (queuedBuffers == 1 && state.sourceHandle.GetState() != AL_PLAYING) {
                state.sourceHandle.Play();
                AddLogf(Debug, "Playback started %s Source:%d", state.uri.c_str(), (int)state.sourceHandle);
            }
            return;
        }

        if (queuedBuffers == 0) {
            if (state.streamFinished && state.sourceHandle.GetState() == AL_STOPPED) {
                state.status = SourceStatus::Stopped;
                AddLogf(Debug, "Playing finished: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
            }
        }
    }

    void ReleaseSourceQueue(SourceState & state) {
        while (true) {
            SoundBuffer b = state.sourceHandle.UnqueueBuffer();
            if (!b)
                break;
            --state.bufferCount;
            assert(state.bufferCount <= conf::MaxBuffersPerSource);
            ReleaseBuffer(b);
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
                auto ss = sourceAcivationQueue.pop(InvalidSourceIndex);
                if(ss != InvalidSourceIndex)
                    activeSources.push(ss);
            }
        }
    }

    void Initialize() {
        standbyBuffers.fill(InvalidSoundBuffer);
        standbySources.fill(InvalidSourceIndex);
        activeSources.fill(InvalidSourceIndex);

        GenBuffers();
        GenSources();
    }
    void Finalize() {
        for (auto &index : activeSources) {
            ReleaseSource(index);
        }
        for (auto &index : sourceAcivationQueue) {
            ReleaseSource(index);
        }

        activeSources.ClearAllocation();
        sourceAcivationQueue.ClearAllocation();

        alDeleteBuffers(standbyBuffers.Allocated(), &standbyBuffers[0]);
        standbyBuffers.ClearAllocation();

        for (auto index : standbySources) {
            auto &state = sourceState[(size_t)index];

            alDeleteSources(1, &state.sourceHandle);
            state.sourceHandle = InvalidSoundSource;
            state.status = SourceStatus::Invalid;

            assert(state.bufferCount == 0);
        }
        standbySources.ClearAllocation();
        sourceState.ClearAllocation();

#ifdef DEBUG
        for (size_t index = 0; index < sourceState.Capacity(); ++index) {
            auto &state = sourceState[index];
            assert(state.status == SourceStatus::Invalid);
            assert(state.bufferCount == 0);
            assert(state.sourceHandle == InvalidSoundSource);     
        }
#endif
    }

    void ReleaseBuffer(SoundBuffer b) {
        if (b == InvalidSoundBuffer)
            return;
        b.ClearData();
        standbyBuffers.push(b);
    }
    //ReleaseSource is not threadsafe!!!
    void ReleaseSource(SourceIndex s) {
        if (s == InvalidSourceIndex)
            return;

        auto &state = sourceState[(size_t)s];
        assert(state.status != SourceStatus::Invalid);
        state.sourceHandle.Stop();
        ReleaseSourceQueue(state);
        state.command = SourceCommand::None;
        state.status = SourceStatus::Standby;
        state.decoder.reset();
        standbySources.push(s);
        AddLogf(Debug, "Released source index:%d handle:%d", (int)s, (int)state.sourceHandle);
    }

    //GetNextSource is not threadsafe!!!
    SourceIndex GetNextSource() {
        if (standbySources.empty()) {
            if (!GenSources())
                return InvalidSourceIndex;
        }
        auto si = standbySources.pop(InvalidSourceIndex);
        if (si == InvalidSourceIndex)
            return InvalidSourceIndex;
        auto &state = sourceState[(size_t)si];
        assert(state.status == SourceStatus::Standby);
        assert(state.bufferCount == 0);
        state.processedBuffers = state.processedBytes = 0;
        state.status = SourceStatus::InitPending;
        state.command = SourceCommand::None;
        state.releaseOnStop = false;
        state.streamFinished = false;
        AddLogf(Debug, "Allocated source index:%d handle:%d", (int)si, (int)state.sourceHandle);
        return si;
    }
    SoundBuffer GetNextBuffer() {
        if (standbyBuffers.empty())
            if(!GenBuffers())
                return InvalidSoundBuffer;
        return standbyBuffers.pop(InvalidSoundBuffer);
    }

    bool GenBuffers() {
        if (allocatedBuffersCount + conf::BufferGenCount > conf::MaxBuffers)
            return false;
        SoundBuffer arr[conf::BufferGenCount] = {};
        alGenBuffers(conf::BufferGenCount, &arr[0]);
        standbyBuffers.append_copy(arr, conf::BufferGenCount);
        allocatedBuffersCount += conf::BufferGenCount;
        AddLogf(Debug, "Generated buffers. New count: %d", (int)allocatedBuffersCount);
        return true;
    }
    bool GenSources() {
        size_t canAlloc = std::min((size_t)conf::SourceGenCount, conf::MaxSources - sourceState.Allocated());
        if (canAlloc == 0)
            return false;
        SoundSource::type arr[conf::SourceGenCount] = {};
        alGenSources(canAlloc, arr);
        for (size_t i = 0; i < canAlloc; ++i) {
            size_t index = 0;
            if (!sourceState.Allocate(index)) {
                __debugbreak();
                assert(false);
                throw false;
                //todo?
            }
            auto &ss = sourceState[index];
            standbySources.push(static_cast<SourceIndex>(index));
            ss = {};
            ss.sourceHandle = arr[i];
            ss.status = SourceStatus::Standby;
        }
        AddLogf(Debug, "Generated sources. New count: %d", (int)sourceState.Allocated());
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
    auto reportTime = std::chrono::steady_clock::now();
    while (threadState == ThreadState::Running) {
        storage->Step();

        std::unique_lock<std::mutex> lock(mtx);
        threadWait.wait_for(lock, std::chrono::milliseconds(Configuration::ThreadStep));

        auto currentTime = std::chrono::steady_clock::now();
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - reportTime).count();
        if (interval > Configuration::DebugReportInterval) {
            reportTime = currentTime;

            AddLogf(Debug, "SoundSystem state: ActiveSources:%u StandbySources:%u AllocatedBuffers:%u buffersInUse:%u", 
                storage->activeSources.Allocated(), storage->standbySources.Allocated(),
                storage->allocatedBuffersCount, storage->allocatedBuffersCount - storage->standbyBuffers.Allocated()
                );
        }
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

    auto &state = storage->sourceState[(size_t)si];
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
    threadWait.notify_one();
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
