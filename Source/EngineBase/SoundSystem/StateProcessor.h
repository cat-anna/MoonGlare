#pragma once

#include <Foundation/iFileSystem.h>

#include "Decoder/iDecoder.h"
#include "Configuration.h"
#include "OpenAl.h"

namespace MoonGlare::SoundSystem {

class SoundSystem;


//todo: remove FinitPending?
enum class SourceStatus : uint8_t {
    Invalid, Standby, Inactive, InitPending, Playing, Paused, Stopped, FinitPending,
};

enum class SourceCommand : uint8_t {
    None, Finalize, ResumePlaying, StopPlaying, Pause,
};

using HandleGeneration = uint16_t;
static constexpr HandleGeneration InvalidHandleGeneration = 0;

union HandleComposite {
    struct {
        HandleGeneration generation;
        SourceIndex index;
    };
    Handle handle;
};

static_assert(sizeof(Handle) == sizeof(uint32_t));
static_assert(sizeof(SourceIndex) + sizeof(HandleGeneration) == sizeof(Handle));
static_assert(sizeof(Handle) == sizeof(HandleComposite));

struct SourceState {
    //TODO: split for public and private part
    using conf = MoonGlare::SoundSystem::Configuration;

    SourceStatus status = SourceStatus::Invalid;
    SourceCommand command = SourceCommand::None;
    bool streamFinished = false;
    bool releaseOnStop = false;
    bool loop = false;
    uint8_t bufferCount = 0;
    SoundSource sourceHandle = InvalidSoundSource;
    std::string uri;
    std::unique_ptr<Decoder::iDecoder> decoder;

    uint32_t processedBuffers = 0;
    uint32_t processedBytes = 0;
    float processedSeconds = 0;
    float duration = 0;

    bool Playable() const {
        return status >= SourceStatus::Playing && status <= SourceStatus::Stopped;
    }
};

class StateProcessor {
public:
    StateProcessor(iFileSystem * fs);
    ~StateProcessor();

    void Step();
    void Initialize();
    void Finalize();

    void PrintState() const;

    Handle AllocateSource();
    void ReleaseSource(Handle handle);

    void ActivateSource(Handle handle);
    void DeactivateSource(Handle handle);

    void SetCommand(Handle handle, SourceCommand command);  
    SourceStatus GetStatus(Handle handle);
    bool IsHandleValid(Handle handle) const { return CheckHandle(handle).first; }
    SoundSource GetSoundSource(Handle handle);
    void SetReleaseOnStop(Handle handle, bool value);
    void CloseHandle(Handle handle); //closes only handle, state does not change
    bool Open(Handle handle, const std::string &uri, SoundKind kind);
    float GetDuration(Handle handle) const;
    float GetTimePosition(Handle handle) const;
    void SetLoop(Handle handle, bool value);
private:
    iFileSystem * fileSystem = nullptr;

    using conf = MoonGlare::SoundSystem::Configuration;

    template<typename T, size_t S>
    using StaticVector = Space::Container::StaticVector<T, S>;

    std::unordered_map<std::string, std::shared_ptr<Decoder::iDecoderFactory>> decoderFactories;

    StaticVector<SoundBuffer, conf::MaxBuffers> standbyBuffers;
    StaticVector<SourceIndex, conf::MaxSources> activeSources;
    StaticVector<SourceState, conf::MaxSources> sourceState;
    StaticVector<HandleGeneration, conf::MaxSources> sourceStateGeneration;

    //access protected by standbySourcesMutex;
    StaticVector<SourceIndex, conf::MaxSources> standbySources;
    //access protected by sourceAcivationQueueMutex;
    StaticVector<SourceIndex, conf::SourceAcivationQueue> sourceAcivationQueue;

    uint32_t allocatedBuffersCount = 0;

    std::mutex standbySourcesMutex;
    std::mutex sourceAcivationQueueMutex;
    using lock_guard = std::lock_guard<std::mutex>;

    enum class SourceProcessStatus {
        Continue, ReleaseAndRemove, Remove,
    };

    void CheckOpenAlError() const;

    void ActivateSources();
    SourceProcessStatus StateProcessor::ProcessSource(SourceIndex si);

    void ProcessPlayState(SourceState &state);
    bool LoadBuffer(SourceState &state, SoundBuffer sb);

    void ReleaseSourceBufferQueue(SourceState &state);
    bool InitializeSource(SourceState &state);

    bool GenBuffers();
    SoundBuffer GetNextBuffer();
    void ReleaseBuffer(SoundBuffer b);

    bool GenSources();
    void ActivateSource(SourceIndex index);
    SourceIndex GetNextSource();
    void ReleaseSource(SourceIndex s);

    std::shared_ptr<Decoder::iDecoderFactory> FindFactory(const std::string &uri);

    Handle GetHandle(SourceIndex s);
    std::pair<bool, SourceIndex> CheckHandle(Handle h) const;
};

}
