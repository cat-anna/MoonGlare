#include "StateProcessor.hpp"
#include "WorkThread.hpp"
#include <build_configuration.hpp>
#include <chrono>
#include <orbit_logger.h>

namespace MoonGlare::SoundSystem {

StateProcessor::StateProcessor(iReadOnlyFileSystem *fs) : fileSystem(fs) {
}
StateProcessor::~StateProcessor() {
}

void StateProcessor::PrintState() const {
    if constexpr (kDebugBuild) {
        AddLogf(Debug, "SoundSystem state: ActiveSources:%u StandbySources:%u AllocatedBuffers:%u buffersInUse:%u",
                activeSources.size(), standbySources.size(), allocatedBuffersCount,
                allocatedBuffersCount - standbyBuffers.size());
    }
}

//---------------------------

void StateProcessor::SetSettings(SoundSettings value) {
    actionQueue.Push([this, value] {
        bool enChanged = value.enabled != settings.enabled;
        settings = value;
        if (settings.enabled) {
            alListenerf(AL_GAIN, settings.masterVolume);
        } else {
            alListenerf(AL_GAIN, 0.0f);
        }
        for (size_t i = 0; i < activeSources.size(); ++i) {
            SourceIndex si = activeSources[i];
            SourceState &state = sourceState[(size_t)si];

            if (!settings.enabled)
                state.command = SourceCommand::StopPlaying;

            UpdateSourceVolume(state);
            if (enChanged && settings.enabled && watcherInterface) {
                watcherInterface->OnFinished(GetSoundHandle(si), state.loop, state.userData);
            }
        }
    });
}

//---------------------------

void StateProcessor::Initialize() {
    //decoderFactories
    for (auto &decoder : Decoder::iDecoderFactory::GetDecoders()) {
        for (auto format : decoder.supportedFormats) {
            decoderFactories[format.fileExtension] = decoder.decoderFactory;
            AddLogf(System, "Supported audio format: %s %s %s", format.fileExtension.c_str(), format.formatName.c_str(),
                    format.decoderName.c_str());
        }
    }

    // standbyBuffers.fill(InvalidSoundBuffer);
    // standbySources.fill(InvalidSourceIndex);
    // activeSources.fill(InvalidSourceIndex);
    sourceStateGeneration.fill(InvalidSoundHandleGeneration);

    GenBuffers();
    GenSources();
}

void StateProcessor::Finalize() {
    for (auto &index : activeSources) {
        ReleaseSource(index);
    }
    for (auto &index : sourceActivationQueue) {
        ReleaseSource(index);
    }

    activeSources.clear();
    sourceActivationQueue.clear();

    alDeleteBuffers(static_cast<ALsizei>(standbyBuffers.size()), &standbyBuffers[0]);
    standbyBuffers.clear();

    //todo InvalidSoundHandleGenration

    for (auto index : standbySources) {
        auto &state = sourceState[(size_t)index];

        alDeleteSources(1, &state.sourceSoundHandle);
        state.sourceSoundHandle = InvalidSoundSource;
        state.status = SourceStatus::Invalid;
    }
    standbySources.clear();
    sourceState.clear();
    decoderFactories.clear();
}

void StateProcessor::CheckOpenAlError() const {
    auto e = alcGetError(alcGetContextsDevice(alcGetCurrentContext()));
    if (e != 0) {
        __debugbreak();
    }
}

//---------------------------

void StateProcessor::Step() {
    while (!actionQueue.Empty()) {
        actionQueue.Pop()();
    }
    ActivateSources();
    for (size_t i = 0; i < activeSources.size();) {
        SourceIndex si = activeSources[i];
        switch (ProcessSource(si)) {
        case SourceProcessStatus::Continue:
            ++i;
            continue;
        case SourceProcessStatus::ReleaseAndRemove:
            ReleaseSource(si);
            [[fallthrough]];
        case SourceProcessStatus::Remove: {
            size_t lastIdx = activeSources.size() - 1;
            if (i != lastIdx) {
                activeSources[i] = activeSources[lastIdx];
            }
            activeSources.pop_back();
            break;
        }
        default:
            assert(false);
        }
    }
}

void StateProcessor::ActivateSources() {
    if (sourceActivationQueue.size() > 0) {
        lock_guard lock(sourceActivationQueueMutex);

        while (!sourceActivationQueue.empty()) {
            auto ss = sourceActivationQueue.back();
            sourceActivationQueue.pop_back();
            if (ss != InvalidSourceIndex)
                activeSources.push_back(ss);
        }
    }
}

//---------------------------

StateProcessor::SourceProcessStatus StateProcessor::ProcessSource(SourceIndex si) {
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
            ProcessPlayState(si, state);
            if (state.status == SourceStatus::Stopped) {
                return SourceProcessStatus::Remove;
            }
            break;
        case SourceStatus::Paused:
            break;
        case SourceStatus::Stopped:
            if (state.releaseOnStop) {
                return SourceProcessStatus::ReleaseAndRemove;
            }
            break;
        case SourceStatus::FinitPending:
            //assert(false);
            //__debugbreak();
            //todo
            break;

        case SourceStatus::Invalid:
        case SourceStatus::Standby:
        case SourceStatus::Inactive:
        default:
            assert(false);
            //invalid status
            //todo
            break;
        }

        auto cmd = state.command.exchange(SourceCommand::None);
        switch (cmd) {
        case SourceCommand::ResumePlaying:
            if (state.Playable()) { //  && state.status != SourceStatus::Playing
                if (state.status == SourceStatus::Paused) {
                    AddLogf(Debug, "Playing resumed: %s Source: %d processed buffers: %u bytes: %6.2f Mib",
                            fileSystem->GetNameOfResource(state.resource_id).c_str(), (int)state.sourceSoundHandle,
                            state.processedBuffers, (float)state.processedBytes / (1024.0f * 1024.0f));
                    state.sourceSoundHandle.Play();
                    CheckOpenAlError();
                } else {
                    state.streamFinished = false;
                    state.decoder->Reset();
                    state.sourceSoundHandle.Stop();
                    ReleaseSourceBufferQueue(state);
                    state.ResetStatistics();
                    state.duration = state.decoder->GetDuration();
                }
                state.status = SourceStatus::Playing;
            }
            break;
        case SourceCommand::Pause:
            if (state.status == SourceStatus::Playing) {
                AddLogf(Debug, "Playing paused: %s Source: %d processed buffers: %u bytes: %6.2f Mib",
                        fileSystem->GetNameOfResource(state.resource_id).c_str(), (int)state.sourceSoundHandle,
                        state.processedBuffers, (float)state.processedBytes / (1024.0f * 1024.0f));
                state.sourceSoundHandle.Pause();
                CheckOpenAlError();
                state.status = SourceStatus::Paused;
            }
            break;
        case SourceCommand::StopPlaying:
            if (state.Playable()) {
                AddLogf(Debug, "Playing stopped: %s Source: %d processed buffers: %u bytes: %6.2f Mib",
                        fileSystem->GetNameOfResource(state.resource_id).c_str(), (int)state.sourceSoundHandle,
                        state.processedBuffers, (float)state.processedBytes / (1024.0f * 1024.0f));
                state.sourceSoundHandle.Stop();
                state.decoder->Reset();
                state.ResetStatistics();
                ReleaseSourceBufferQueue(state);
            }
            state.status = SourceStatus::Stopped;
            if (state.releaseOnStop) {
                return SourceProcessStatus::ReleaseAndRemove;
            } else {
                return SourceProcessStatus::Remove;
            }
            break;

        case SourceCommand::Finalize:
            return SourceProcessStatus::ReleaseAndRemove;

        case SourceCommand::None:
            break;

        default:
            assert(false);
        }
    }
    return SourceProcessStatus::Continue;
}

void StateProcessor::ProcessPlayState(SourceIndex index, SourceState &state) {
    while (true) {
        ALint processedBuffers = state.sourceSoundHandle.GetProcessedBuffers();
        CheckOpenAlError();
        if (processedBuffers <= 0)
            break;

        SoundBuffer sb = state.sourceSoundHandle.UnqueueBuffer();
        CheckOpenAlError();
        if (!sb)
            break;
        state.processedSeconds += sb.GetDuration();
        ++state.processedBuffers;
        if (!LoadBuffer(state, sb)) {
            ReleaseBuffer(sb);
        }
    };

    ALint queuedBuffers = state.sourceSoundHandle.GetQueuedBuffers();
    CheckOpenAlError();

    if (queuedBuffers < kMaxBuffersPerSource && !state.streamFinished) {
        SoundBuffer b = GetNextBuffer();
        if (!LoadBuffer(state, b)) {
            ReleaseBuffer(b);
        } else {
            ++queuedBuffers;
            if (queuedBuffers == 1 && state.sourceSoundHandle.GetState() != AL_PLAYING) {
                CheckOpenAlError();
                state.sourceSoundHandle.Play();
                CheckOpenAlError();
                CheckSoundKind(state, b);
                CheckOpenAlError();
                AddLogf(Debug, "Playback started %s Source:%d",
                        fileSystem->GetNameOfResource(state.resource_id).c_str(), (int)state.sourceSoundHandle);
            }
        }
        return;
    }

    if (queuedBuffers == 0) {
        if (state.streamFinished && state.sourceSoundHandle.GetState() == AL_STOPPED) {
            CheckOpenAlError();
            state.status = SourceStatus::Stopped;
            if (state.loop) {
                state.command = SourceCommand::ResumePlaying;
            }
            if (watcherInterface) {
                watcherInterface->OnFinished(GetSoundHandle(index), state.loop, state.userData);
            }
            AddLogf(Debug, "Playing finished: %s Source: %d processed buffers: %u bytes: %6.2f Mib",
                    fileSystem->GetNameOfResource(state.resource_id).c_str(), (int)state.sourceSoundHandle,
                    state.processedBuffers, (float)state.processedBytes / (1024.0f * 1024.0f));
        }
    }
}

void StateProcessor::CheckSoundKind(SourceState &state, SoundBuffer b) {
    while (true) {
        switch (state.kind) {
        case SoundKind::Music:
        case SoundKind::Effect:
        case SoundKind::None:
            UpdateSourceVolume(state);
            return;
        case SoundKind::Auto:
        default:
            if (b.GetDuration() < kEffectThresholdDuration)
                state.kind = SoundKind::Effect;
            else
                state.kind = SoundKind::Music;
            continue;
        }
    }
}

bool StateProcessor::LoadBuffer(SourceState &state, SoundBuffer sb) {
    Decoder::DecodeState s = Decoder::DecodeState::Completed;
    uint64_t bytes = 0;
    if (!state.streamFinished) {
        s = state.decoder->DecodeBuffer(sb, &bytes);
        state.processedBytes += bytes;
        state.streamFinished = s != Decoder::DecodeState::Continue;
    }
    if (s < Decoder::DecodeState::Completed) {
        state.sourceSoundHandle.QueueBuffer(sb);
        CheckOpenAlError();
        return true;
    } else {
        return false;
    }
}

//---------------------------

void StateProcessor::ReleaseSourceBufferQueue(SourceState &state) {
    while (true) {
        SoundBuffer b = state.sourceSoundHandle.UnqueueBuffer();
        CheckOpenAlError();
        if (!b)
            break;
        ReleaseBuffer(b);
    }
    assert(state.sourceSoundHandle.GetProcessedBuffers() == 0);
    assert(state.sourceSoundHandle.GetQueuedBuffers() == 0);
}

bool StateProcessor::InitializeSource(SourceState &state) {
    auto factory = FindFactory(state.resource_id);
    if (!factory)
        return false;
    state.ResetStatistics();
    state.decoder = factory->CreateDecoder();
    if (!state.decoder)
        return false;

    std::string data;
    if (!fileSystem->ReadFileByResourceId(state.resource_id, data)) {
        return false;
    }

    if (!state.decoder->SetData(std::move(data), fileSystem->GetNameOfResource(state.resource_id))) {
        return false;
    }
    if (!state.decoder->Reset())
        return false;
    state.duration = state.decoder->GetDuration();
    state.status = SourceStatus::Stopped;
    return true;
}

//---------------------------

bool StateProcessor::GenBuffers() {
    if (allocatedBuffersCount + kBufferGenCount > kMaxBuffers)
        return false;
    SoundBuffer arr[kBufferGenCount];
    alGenBuffers(kBufferGenCount, &arr[0]);
    CheckOpenAlError();
    standbyBuffers.insert(standbyBuffers.end(), arr, arr + kBufferGenCount);
    allocatedBuffersCount += kBufferGenCount;
    AddLogf(Debug, "Generated buffers. New count: %d", (int)allocatedBuffersCount);
    return true;
}

SoundBuffer StateProcessor::GetNextBuffer() {
    if (standbyBuffers.empty())
        if (!GenBuffers())
            return InvalidSoundBuffer;

    auto last = standbyBuffers.back();
    standbyBuffers.pop_back();
    return last;
}

void StateProcessor::ReleaseBuffer(SoundBuffer b) {
    if (b == InvalidSoundBuffer)
        return;
    b.ClearData();
    standbyBuffers.push_back(b);
}

//---------------------------

bool StateProcessor::GenSources() {
    size_t canAlloc = std::min((size_t)kSourceGenCount, kMaxSources - sourceState.size());
    if (canAlloc == 0)
        return false;
    SoundSource::type arr[kSourceGenCount] = {};
    alGenSources(static_cast<ALsizei>(canAlloc), arr);
    CheckOpenAlError();
    lock_guard lock(standbySourcesMutex);
    for (size_t i = 0; i < canAlloc; ++i) {
        size_t index = sourceState.size();
        sourceState.resize(index + 1); //no push_back - requires copy - not allowed by atomic
        standbySources.push_back(static_cast<SourceIndex>(index));
        auto &ss = sourceState[index];
        ss.Reset();
        ss.sourceSoundHandle = arr[i];
        ss.status = SourceStatus::Standby;
    }
    AddLogf(Debug, "Generated sources. New count: %d", (int)sourceState.size());
    return true;
}

SoundHandle StateProcessor::AllocateSource() {
    auto index = GetNextSource();
    if (index == InvalidSourceIndex)
        return SoundHandle::Invalid;
    return GetSoundHandle(index);
}

void StateProcessor::ReleaseSource(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;

    CloseSoundHandle(handle);
    auto &state = sourceState[(size_t)index];
    switch (state.status) {
    case SourceStatus::Playing:
    case SourceStatus::Paused:
    case SourceStatus::Stopped:
        state.command = SourceCommand::Finalize;
        break;

    case SourceStatus::FinitPending:
        //nothing todo
        break;

    case SourceStatus::Inactive:
        ActivateSource(index); //todo: is this the best method?
        state.command = SourceCommand::Finalize;
        break;

    case SourceStatus::Standby:
    case SourceStatus::Invalid:
        //these states should not have valid handles
    default:
        assert(false);
    }
}

SourceIndex StateProcessor::GetNextSource() {
    if (standbySources.empty()) {
        if (!GenSources())
            return InvalidSourceIndex;
    }
    SourceIndex si;
    {
        lock_guard lock(standbySourcesMutex);
        si = standbySources.back();
        standbySources.pop_back();
    }
    if (si == InvalidSourceIndex)
        return InvalidSourceIndex;
    auto &state = sourceState[(size_t)si];
    assert(state.status == SourceStatus::Standby);
    state.Reset();
    if (sourceStateGeneration[(size_t)si] == InvalidSoundHandleGeneration)
        ++sourceStateGeneration[(size_t)si];
    AddLogf(Debug, "Allocated source index:%d handle:%d", (int)si, (int)state.sourceSoundHandle);
    return si;
}

void StateProcessor::ReleaseSource(SourceIndex s) {
    if (s == InvalidSourceIndex)
        return;

    auto &state = sourceState[(size_t)s];
    assert(state.status != SourceStatus::Invalid);
    state.sourceSoundHandle.Stop();
    ReleaseSourceBufferQueue(state);
    state.command = SourceCommand::None;
    state.status = SourceStatus::Standby;
    state.decoder.reset();
    ++sourceStateGeneration[(size_t)s];
    {
        lock_guard lock(standbySourcesMutex);
        standbySources.push_back(s);
    }

    AddLogf(Debug, "Released source index:%d handle:%d", (int)s, (int)state.sourceSoundHandle);
}

void StateProcessor::ActivateSource(SourceIndex index) {
    auto &state = sourceState[(size_t)index];
    assert(state.status == SourceStatus::Inactive);
    if (state.status == SourceStatus::Inactive) {
        state.status = SourceStatus::InitPending;
        lock_guard lock(sourceActivationQueueMutex);
        sourceActivationQueue.push_back(index);
    } else {
        AddLogf(Warning, "Source not activated - not in InitPending state index:%d state:%d", (int)index,
                (int)state.status);
    }
}

void StateProcessor::UpdateSourceVolume(SourceState &state) {
    switch (state.kind) {
    case SoundKind::Music:
        state.sourceSoundHandle.SetGain(settings.musicVolume);
        break;
    case SoundKind::Effect:
        state.sourceSoundHandle.SetGain(settings.effectVolume);
        break;
    case SoundKind::None:
        state.sourceSoundHandle.SetGain(1.0f);
    case SoundKind::Auto:
    default:
        break;
        //ignored
    }
}

//---------------------------

SoundHandle StateProcessor::GetSoundHandle(SourceIndex s) {
    if (s == InvalidSourceIndex)
        return SoundHandle::Invalid;
    SoundHandleComposite hc;
    hc.generation = sourceStateGeneration[(size_t)s];
    if (hc.generation == InvalidSoundHandleGeneration)
        return SoundHandle::Invalid;
    hc.index = s;
    return hc.handle;
}

std::pair<bool, SourceIndex> StateProcessor::CheckSoundHandle(SoundHandle handle) const {
    SoundHandleComposite hc;
    hc.handle = handle;
    if (hc.generation == InvalidSoundHandleGeneration)
        return {false, InvalidSourceIndex};
    if (hc.index >= static_cast<SourceIndex>(kMaxSources))
        return {false, InvalidSourceIndex};
    if (sourceStateGeneration[(size_t)hc.index] != hc.generation)
        return {false, InvalidSourceIndex};
    return {true, hc.index};
}

void StateProcessor::SetCommand(SoundHandle handle, SourceCommand command) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;

    if (!settings.enabled && command == SourceCommand::ResumePlaying)
        return;

    sourceState[(size_t)index].command = command;
}

SoundSource StateProcessor::GetSoundSource(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (valid)
        return sourceState[(size_t)index].sourceSoundHandle;
    return InvalidSoundSource;
}

SourceStatus StateProcessor::GetStatus(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (valid)
        return sourceState[(size_t)index].status;
    return SourceStatus::Invalid;
}

void StateProcessor::SetReleaseOnStop(SoundHandle handle, bool value) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (valid)
        sourceState[(size_t)index].releaseOnStop = value;
}

void StateProcessor::ActivateSource(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (valid)
        ActivateSource(index);
}

void StateProcessor::DeactivateSource(SoundHandle handle) {
    assert(false);
}

void StateProcessor::CloseSoundHandle(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (valid)
        ++sourceStateGeneration[(size_t)index];
}

bool StateProcessor::Open(SoundHandle handle, FileResourceId resource, SoundKind kind) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return false;

    auto &state = sourceState[(size_t)index];
    assert(state.status == SourceStatus::Inactive);
    if (state.status != SourceStatus::Inactive)
        return false;

    state.kind = kind;
    state.resource_id = resource;
    assert(!state.decoder);

    return true;
}

float StateProcessor::GetDuration(SoundHandle handle) const {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return -1.0f;
    auto &state = sourceState[(size_t)index];
    if (state.duration <= 0) {
        //TODO:?
    }
    return state.duration;
}

float StateProcessor::GetTimePosition(SoundHandle handle) const {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return -1.0f;
    auto &state = sourceState[(size_t)index];
    return state.processedSeconds + state.sourceSoundHandle.GetTimePosition();
}

void StateProcessor::SetLoop(SoundHandle handle, bool value) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;
    sourceState[(size_t)index].loop = value;
}

bool StateProcessor::GetLoop(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return false;
    return sourceState[(size_t)index].loop;
}

void StateProcessor::SetUserData(SoundHandle handle, UserData userData) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;
    auto &state = sourceState[(size_t)index];
    state.userData = userData;
}

void StateProcessor::SetCallback(std::shared_ptr<iPlaybackWatcher> iface) {
    watcherInterface = std::move(iface);
}

void StateProcessor::ReopenStream(SoundHandle handle, FileResourceId resoure, SoundKind kind) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;

    auto &state = sourceState[(size_t)index];
    state.resource_id = resoure;
    state.kind = kind;
    state.command = SourceCommand::None;
    actionQueue.Push([this, index = index] { sourceState[(size_t)index].status = SourceStatus::InitPending; });
}

FileResourceId StateProcessor::GetStreamResourceId(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return kInvalidResourceId;
    return sourceState[(size_t)index].resource_id;
}

void StateProcessor::SetSoundKind(SoundHandle handle, SoundKind value) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;
    actionQueue.Push([this, index = index, value] {
        sourceState[(size_t)index].kind = value;
        UpdateSourceVolume(sourceState[(size_t)index]);
    });
}

SoundKind StateProcessor::GetSoundKind(SoundHandle handle) {
    const auto [valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return SoundKind::None;
    return sourceState[(size_t)index].kind;
}

//---------------------------

std::shared_ptr<Decoder::iDecoderFactory> StateProcessor::FindFactory(FileResourceId res_id) {
    auto name = fileSystem->GetNameOfResource(res_id, false);
    auto last_dot = name.rfind('.');
    if (last_dot == std::string::npos) {
        return nullptr;
    }

    auto it = decoderFactories.find(&name[last_dot + 1]);
    if (it == decoderFactories.end())
        return nullptr;

    return it->second;
}

} // namespace MoonGlare::SoundSystem
