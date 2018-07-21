#pragma once

#include <Foundation/iFileSystem.h>

#include "Decoder/iDecoder.h"
#include "Configuration.h"
#include "OpenAl.h"

namespace MoonGlare::SoundSystem {

class SoundSystem;

enum class SourceStatus : uint8_t {
    Invalid, Standby, InitPending, Playing, Paused, Stopped, FinitPending,
};

enum class SourceCommand : uint8_t {
    None, Finalize, ResumePlaying, StopPlaying, Pause, Release,
};

struct SourceState {
    using conf = MoonGlare::SoundSystem::Configuration;
    template<typename T>
    using BufferArray = Space::Container::StaticVector<T, conf::MaxBuffersPerSource>;

    SourceStatus status = SourceStatus::Invalid;
    SourceCommand command = SourceCommand::None;
    bool streamFinished = false;
    bool automaticRelease = false;
    uint8_t firstBuffer = 0;
    uint8_t bufferCount = 0;
    BufferArray<SoundBuffer> queuedBuffers;
    SoundSource sourceHandle = InvalidSoundSource;
    uint32_t processedBuffers = 0;
    uint32_t processedBytes= 0;
    std::string uri;
    std::unique_ptr<Decoder::iDecoder> decoder;

    bool Playable() const {
        return status >= SourceStatus::Playing && status <= SourceStatus::Stopped;
    }
};

enum class ThreadState {
    Stopped,
    Starting,
    Running,
    Finished,
    Stopping,
};

class WorkThread {
public:
    WorkThread(SoundSystem *owner, iFileSystem *fs);
    ~WorkThread();

    void Initialize();
    void Finalize();

    std::unique_ptr<iSound> OpenSound(const std::string &uri, bool start, SoundKind kind);
private:
    std::thread thread;
    std::atomic<ThreadState> threadState = ThreadState::Stopped;
    std::condition_variable threadWait;
    SoundSystem * soundSystem = nullptr;
    iFileSystem * fileSystem = nullptr;

    std::unordered_map<std::string, std::shared_ptr<Decoder::iDecoderFactory>> decoderFactories;

    struct Storage;
    std::unique_ptr<Storage> storage;

    void ThreadMain();

    void InitializeDevice();
    void FinalizeDevice();

    std::shared_ptr<Decoder::iDecoderFactory> FindFactory(const std::string &uri);
};

}
