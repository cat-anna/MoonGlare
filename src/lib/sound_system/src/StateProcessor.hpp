#pragma once

#include "HandleApi.hpp"
#include "OpenAl.hpp"
#include "decoder/decoder.hpp"
#include "readonly_file_system.h"
#include "sound_system/configuration.hpp"
#include "synchronized_task_queue.hpp"
#include <array>
#include <atomic>
#include <boost/container/static_vector.hpp>
#include <functional>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace MoonGlare::SoundSystem {

class SoundSystem;

//todo: remove FinitPending?
enum class SourceStatus : uint8_t {
    Invalid,
    Standby,
    Inactive,
    InitPending,
    Playing,
    Paused,
    Stopped,
    FinitPending,
};

enum class SourceCommand : uint8_t {
    None,
    Finalize,
    ResumePlaying,
    StopPlaying,
    Pause,
};

using SoundHandleGeneration = uint32_t;
static constexpr SoundHandleGeneration InvalidSoundHandleGeneration = 0;

union SoundHandleComposite {
    struct {
        SoundHandleGeneration generation;
        SourceIndex index;
    };
    SoundHandle handle;
};

static_assert(sizeof(SoundHandle) == sizeof(uint64_t));
static_assert(sizeof(SourceIndex) + sizeof(SoundHandleGeneration) == sizeof(SoundHandle));
static_assert(sizeof(SoundHandle) == sizeof(SoundHandleComposite));

struct SourceState {
    //TODO: split for public and private part
    //TODO: fadein/out support
    //TODO: make watcherInterface not per stream

    SourceStatus status = SourceStatus::Invalid;
    std::atomic<SourceCommand> command = SourceCommand::None; //atomic?
    SoundKind kind = SoundKind::Auto;
    bool streamFinished = false;
    bool releaseOnStop = false;
    bool loop = false; //AL_LOOPING ?
    SoundSource sourceSoundHandle = InvalidSoundSource;
    FileResourceId resource_id = kInvalidResourceId;
    std::unique_ptr<Decoder::iDecoder> decoder;

    uint32_t processedBuffers = 0;
    uint64_t processedBytes = 0;
    float processedSeconds = 0;
    float duration = 0;

    void Reset() {
        ResetStatistics();
        kind = SoundKind::Auto;
        status = SourceStatus::Inactive;
        command = SourceCommand::None;
        releaseOnStop = false;
        streamFinished = false;
        ResetStatistics();
        loop = false;
        userData = 0;
    }

    void ResetStatistics() {
        processedBuffers = 0;
        processedBytes = 0;
        processedSeconds = 0;
        duration = 0;
    }

    UserData userData = 0;

    bool Playable() const { return status >= SourceStatus::Playing && status <= SourceStatus::Stopped; }
};

class StateProcessor {
public:
    StateProcessor(iReadOnlyFileSystem *fs);
    ~StateProcessor();

    void Step();
    void Initialize();
    void Finalize();

    SoundSettings GetSettings() const { return settings; }
    void SetSettings(SoundSettings value);

    void PrintState() const;

    SoundHandle AllocateSource();
    void ReleaseSource(SoundHandle handle);

    void ActivateSource(SoundHandle handle);
    void DeactivateSource(SoundHandle handle);

    void SetCommand(SoundHandle handle, SourceCommand command);
    SourceStatus GetStatus(SoundHandle handle);
    bool IsSoundHandleValid(SoundHandle handle) const { return CheckSoundHandle(handle).first; }
    SoundSource GetSoundSource(SoundHandle handle);
    void SetReleaseOnStop(SoundHandle handle, bool value);
    void CloseSoundHandle(SoundHandle handle); //closes only handle, state does not change
    bool Open(SoundHandle handle, FileResourceId resource, SoundKind kind);
    float GetDuration(SoundHandle handle) const;
    float GetTimePosition(SoundHandle handle) const;
    void SetLoop(SoundHandle handle, bool value);
    bool GetLoop(SoundHandle handle);
    void SetUserData(SoundHandle handle, UserData userData);
    void SetCallback(std::shared_ptr<iPlaybackWatcher> iface);
    void ReopenStream(SoundHandle handle, FileResourceId resource, SoundKind kind);
    FileResourceId GetStreamResourceId(SoundHandle handle);
    void SetSoundKind(SoundHandle handle, SoundKind value);
    SoundKind GetSoundKind(SoundHandle handle);

private:
    iReadOnlyFileSystem *const fileSystem = nullptr;

    template <typename T, size_t S>
    using StaticVector = boost::container::static_vector<T, S>;

    std::unordered_map<std::string, std::shared_ptr<Decoder::iDecoderFactory>> decoderFactories;

    StaticVector<SoundBuffer, kMaxBuffers> standbyBuffers;
    StaticVector<SourceIndex, kMaxSources> activeSources;
    StaticVector<SourceState, kMaxSources> sourceState;
    std::array<SoundHandleGeneration, kMaxSources> sourceStateGeneration;

    //access protected by standbySourcesMutex;
    StaticVector<SourceIndex, kMaxSources> standbySources;
    //access protected by sourceActivationQueueMutex;
    StaticVector<SourceIndex, kSourceActivationQueue> sourceActivationQueue;

    uint32_t allocatedBuffersCount = 0;
    SoundSettings settings;
    SynchronizedTaskQueue<std::function<void()>> actionQueue;

    std::mutex standbySourcesMutex;
    std::mutex sourceActivationQueueMutex;
    std::shared_ptr<iPlaybackWatcher> watcherInterface = nullptr;
    using lock_guard = std::lock_guard<std::mutex>;

    enum class SourceProcessStatus {
        Continue,
        ReleaseAndRemove,
        Remove,
    };

    void CheckOpenAlError() const;

    void ActivateSources();
    SourceProcessStatus ProcessSource(SourceIndex si);

    void ProcessPlayState(SourceIndex index, SourceState &state);
    void CheckSoundKind(SourceState &state, SoundBuffer b);
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
    void UpdateSourceVolume(SourceState &state);

    std::shared_ptr<Decoder::iDecoderFactory> FindFactory(FileResourceId res_id);

    SoundHandle GetSoundHandle(SourceIndex s);
    std::pair<bool, SourceIndex> CheckSoundHandle(SoundHandle h) const;
};

} // namespace MoonGlare::SoundSystem
