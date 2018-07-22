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
    None, Finalize, ResumePlaying, StopPlaying, Pause,// Release,
};

struct SourceState {
    //TODO: split for public and private part
    using conf = MoonGlare::SoundSystem::Configuration;

    SourceStatus status = SourceStatus::Invalid;
    SourceCommand command = SourceCommand::None;
    bool streamFinished = false;
    bool releaseOnStop = false;
    uint8_t bufferCount = 0;
    SoundSource sourceHandle = InvalidSoundSource;
    uint32_t processedBuffers = 0;
    uint32_t processedBytes = 0;
    float processedSeconds = 0;
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

    struct Storage;
    std::unique_ptr<Storage> storage;

    void ThreadMain();

    void InitializeDevice();
    void FinalizeDevice();
};

}
