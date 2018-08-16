#include <chrono>

#include "StateProcessor.h"
#include "WorkThread.h"

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare::SoundSystem {

char *copystr(const char *str) {
    if (!str) return nullptr;
    char *r = new char[strlen(str) + 1];
    std::strcpy(r, str);
    return r;
}

//---------------------------

StateProcessor::StateProcessor(iFileSystem * fs) : fileSystem(fs) { }
StateProcessor::~StateProcessor() {}

void StateProcessor::PrintState() const {
    AddLogf(Debug, "SoundSystem state: ActiveSources:%u StandbySources:%u AllocatedBuffers:%u buffersInUse:%u",
        activeSources.Allocated(), standbySources.Allocated(),
        allocatedBuffersCount, allocatedBuffersCount - standbyBuffers.Allocated()
    );
}

//---------------------------

void StateProcessor::SetSettings(SoundSettings value) {
    actionQueue.Add([this, value]{
        bool enChanged = value.enabled != settings.enabled;
        settings = value;
        if (settings.enabled) {
            alListenerf(AL_GAIN, settings.masterVolume);
        }
        else {
            alListenerf(AL_GAIN, 0.0f);
        }
        for (size_t i = 0; i < activeSources.Allocated(); ++i) {
            SourceIndex si = activeSources[i];
            SourceState &state = sourceState[(size_t)si];

            if (!settings.enabled)
                state.command = SourceCommand::StopPlaying;

            UpdateSourceVolume(state);
            if(enChanged && settings.enabled && state.watcherInterface) {
                state.watcherInterface->OnFinished(GetSoundHandle(si), state.loop, state.userData);
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
            AddLogf(System, "Supported audio format: %s %s %s",
                format.fileExtension.c_str(),
                format.formatName.c_str(),
                format.decoderName.c_str());
        }
    }

    standbyBuffers.fill(InvalidSoundBuffer);
    standbySources.fill(InvalidSourceIndex);
    activeSources.fill(InvalidSourceIndex);
    sourceStateGeneration.fill(InvalidSoundHandleGeneration);

    GenBuffers();
    GenSources();
}

void StateProcessor::Finalize() {
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

    //todo InvalidSoundHandleGenration

    for (auto index : standbySources) {
        auto &state = sourceState[(size_t)index];

        alDeleteSources(1, &state.sourceSoundHandle);
        state.sourceSoundHandle = InvalidSoundSource;
        state.status = SourceStatus::Invalid;
    }
    standbySources.ClearAllocation();
    sourceState.ClearAllocation();
    decoderFactories.clear();

#ifdef DEBUG
    for (size_t index = 0; index < sourceState.Capacity(); ++index) {
        auto &state = sourceState[index];
        assert(state.status == SourceStatus::Invalid);
        assert(state.sourceSoundHandle == InvalidSoundSource);
    }
#endif
}

void StateProcessor::CheckOpenAlError() const {
    auto e = alcGetError(alcGetContextsDevice(alcGetCurrentContext()));
    if (e != 0) {
        __debugbreak();
    }
}

//---------------------------

void StateProcessor::Step() {
    actionQueue.AsyncDispatchAll();
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
            assert(false);
        }
    }
}

void StateProcessor::ActivateSources() {
    if (sourceAcivationQueue.Allocated() > 0) {
        lock_guard lock(sourceAcivationQueueMutex);

        while (!sourceAcivationQueue.empty()) {
            auto ss = sourceAcivationQueue.pop(InvalidSourceIndex);
            if (ss != InvalidSourceIndex)
                activeSources.push(ss);
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
            break;
        case SourceStatus::Paused:
            break;
        case SourceStatus::Stopped:
            if (state.releaseOnStop)
                return SourceProcessStatus::ReleaseAndRemove;
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

        switch (state.command) {
        case SourceCommand::ResumePlaying:
            if (state.Playable()) {  //  && state.status != SourceStatus::Playing
                if (state.status == SourceStatus::Paused) {
                    AddLogf(Debug, "Playing resumed: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.get(), (int)state.sourceSoundHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
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
                AddLogf(Debug, "Playing paused: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.get(), (int)state.sourceSoundHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
                state.sourceSoundHandle.Pause();
                CheckOpenAlError();
                state.status = SourceStatus::Paused;
            }
            break;
        case SourceCommand::StopPlaying:
            if (state.Playable()) {
                state.status = SourceStatus::Stopped;
                state.sourceSoundHandle.Stop();
                state.decoder->Reset();
                state.ResetStatistics(); 
                ReleaseSourceBufferQueue(state);
                AddLogf(Debug, "Playing stopped: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.get(), (int)state.sourceSoundHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
            }
            break;

        case SourceCommand::Finalize:
            return SourceProcessStatus::ReleaseAndRemove;

        case SourceCommand::None:
            break;

        default:
            assert(false);
        }

        state.command = SourceCommand::None;
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

    if (queuedBuffers < Configuration::MaxBuffersPerSource && !state.streamFinished) {
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
                AddLogf(Debug, "Playback started %s Source:%d", state.uri.get(), (int)state.sourceSoundHandle);
            }
        }
        return;
    }

    if (queuedBuffers == 0) {
        if (state.streamFinished && state.sourceSoundHandle.GetState() == AL_STOPPED) {
            CheckOpenAlError();
            state.status = SourceStatus::Stopped;
            if (state.loop) 
                state.command = SourceCommand::ResumePlaying;
            if (state.watcherInterface)
                state.watcherInterface->OnFinished(GetSoundHandle(index), state.loop, state.userData);
            AddLogf(Debug, "Playing finished: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.get(), (int)state.sourceSoundHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
        }
    }
}

void StateProcessor::CheckSoundKind(SourceState & state, SoundBuffer b) {
    while (true) {
        switch (state.kind) {
        case SoundKind::Music:
        case SoundKind::Effect:
        case SoundKind::None:
            UpdateSourceVolume(state);
            return;
        case SoundKind::Auto:
        default:
            if (b.GetDuration() < Configuration::EffectThresholdDuration)
                state.kind = SoundKind::Effect;
            else
                state.kind = SoundKind::Music;
            continue;
        }
    }
}

bool StateProcessor::LoadBuffer(SourceState &state, SoundBuffer sb) {
    Decoder::DecodeState s = Decoder::DecodeState::Completed;
    uint32_t bytes = 0;
    if (!state.streamFinished) {
        s = state.decoder->DecodeBuffer(sb, &bytes);
        state.processedBytes += bytes;
        state.streamFinished = s != Decoder::DecodeState::Continue;
    }
    if (s < Decoder::DecodeState::Completed) {
        state.sourceSoundHandle.QueueBuffer(sb);
        CheckOpenAlError();
        return true;
    }
    else {
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
    auto factory = FindFactory(state.uri.get());
    if (!factory)
        return false;
    state.ResetStatistics();
    state.decoder = factory->CreateDecoder();
    if (!state.decoder)
        return false;

    StarVFS::ByteTable data;
    std::string uri = state.uri.get(); //todo this makes a copy!
    if (!fileSystem->OpenFile(data, uri)) {
        return false;
    }

    if (!state.decoder->SetData(std::move(data), uri)) {
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
    if (allocatedBuffersCount + Configuration::BufferGenCount > Configuration::MaxBuffers)
        return false;
    SoundBuffer arr[Configuration::BufferGenCount] = {};
    alGenBuffers(Configuration::BufferGenCount, &arr[0]);
    CheckOpenAlError();
    standbyBuffers.append_copy(arr, Configuration::BufferGenCount);
    allocatedBuffersCount += Configuration::BufferGenCount;
    AddLogf(Debug, "Generated buffers. New count: %d", (int)allocatedBuffersCount);
    return true;
}

SoundBuffer StateProcessor::GetNextBuffer() {
    if (standbyBuffers.empty())
        if (!GenBuffers())
            return InvalidSoundBuffer;
    return standbyBuffers.pop(InvalidSoundBuffer);
}

void StateProcessor::ReleaseBuffer(SoundBuffer b) {
    if (b == InvalidSoundBuffer)
        return;
    b.ClearData();
    standbyBuffers.push(b);
}

//---------------------------

bool StateProcessor::GenSources() {
    size_t canAlloc = std::min((size_t)Configuration::SourceGenCount, Configuration::MaxSources - sourceState.Allocated());
    if (canAlloc == 0)
        return false;
    SoundSource::type arr[Configuration::SourceGenCount] = {};
    alGenSources(canAlloc, arr);
    CheckOpenAlError();
    lock_guard lock(standbySourcesMutex);
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
        ss.sourceSoundHandle = arr[i];
        ss.status = SourceStatus::Standby;
    }
    AddLogf(Debug, "Generated sources. New count: %d", (int)sourceState.Allocated());
    return true;
}

SoundHandle StateProcessor::AllocateSource() {
    auto index = GetNextSource();
    if (index == InvalidSourceIndex)
        return SoundHandle::Invalid;
    return GetSoundHandle(index);
}

void StateProcessor::ReleaseSource(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
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
        si = standbySources.pop(InvalidSourceIndex);
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
        standbySources.push(s);
    }

    AddLogf(Debug, "Released source index:%d handle:%d", (int)s, (int)state.sourceSoundHandle);
}

void StateProcessor::ActivateSource(SourceIndex index) {
    auto &state = sourceState[(size_t)index];
    assert(state.status == SourceStatus::Inactive);
    if (state.status == SourceStatus::Inactive) {
        state.status = SourceStatus::InitPending;
        lock_guard lock(sourceAcivationQueueMutex);
        sourceAcivationQueue.push(index);
    }
    else {
        AddLogf(Warning, "Source not activated - not in InitPending state index:%d state:%d", (int)index, (int)state.status);
    }
}

void StateProcessor::UpdateSourceVolume(SourceState & state)
{
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
        return { false, InvalidSourceIndex };
    if (hc.index >= static_cast<SourceIndex>(Configuration::MaxSources))
        return { false, InvalidSourceIndex };
    if (sourceStateGeneration[(size_t)hc.index] != hc.generation)
        return { false, InvalidSourceIndex };
    return { true, hc.index };
}

void StateProcessor::SetCommand(SoundHandle handle, SourceCommand command) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;
    
    if (!settings.enabled && command == SourceCommand::ResumePlaying)
        return;

    sourceState[(size_t)index].command = command;
}

SoundSource StateProcessor::GetSoundSource(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (valid)
        return sourceState[(size_t)index].sourceSoundHandle;
    return InvalidSoundSource;
}

SourceStatus StateProcessor::GetStatus(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (valid)
        return sourceState[(size_t)index].status;
    return SourceStatus::Invalid;
}

void StateProcessor::SetReleaseOnStop(SoundHandle handle, bool value) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (valid)
        sourceState[(size_t)index].releaseOnStop = value;
}

void StateProcessor::ActivateSource(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (valid) 
        ActivateSource(index);
}

void StateProcessor::DeactivateSource(SoundHandle handle) {
    assert(false);
}

void StateProcessor::CloseSoundHandle(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (valid)
        ++sourceStateGeneration[(size_t)index];
}

bool StateProcessor::Open(SoundHandle handle, const std::string &uri, SoundKind kind) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return false;

    auto &state = sourceState[(size_t)index];
    assert(state.status == SourceStatus::Inactive);
    if (state.status != SourceStatus::Inactive)
        return false;

    state.kind = kind;
    state.uri.reset(copystr(uri.c_str()));
    assert(!state.decoder);

    return true;
}

float StateProcessor::GetDuration(SoundHandle handle) const {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return -1.0f;
    auto &state = sourceState[(size_t)index];
    if (state.duration <= 0) {
        //TODO:?
    }
    return state.duration;
}

float StateProcessor::GetTimePosition(SoundHandle handle) const {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return -1.0f;
    auto &state = sourceState[(size_t)index];
    return state.processedSeconds + state.sourceSoundHandle.GetTimePosition();
}

void StateProcessor::SetLoop(SoundHandle handle, bool value) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;
    sourceState[(size_t)index].loop = value;
}

bool StateProcessor::GetLoop(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return false;
    return sourceState[(size_t)index].loop;
}

void StateProcessor::SetCallback(SoundHandle handle, iPlaybackWatcher *iface, UserData userData) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;

    auto &state = sourceState[(size_t)index];

    //in case of any event being generated
    state.watcherInterface = nullptr;
    state.userData = 0;
    state.userData = userData;
    state.watcherInterface = iface;
}

void StateProcessor::ReopenStream(SoundHandle handle, const char *uri, SoundKind kind) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;

    auto &state = sourceState[(size_t)index];
    state.uri.reset(copystr(uri));
    state.kind = kind;
    state.command = SourceCommand::None;
    actionQueue.Add([this, index] { 
        sourceState[(size_t)index].status = SourceStatus::InitPending; 
    });
}

const char *StateProcessor::GetStreamURI(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
         return nullptr;
    return sourceState[(size_t)index].uri.get();
}

void StateProcessor::SetSoundKind(SoundHandle handle, SoundKind value) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return;
    actionQueue.Add([this, index, value] {
        sourceState[(size_t)index].kind = value;
        UpdateSourceVolume(sourceState[(size_t)index]);
    });
}

SoundKind StateProcessor::GetSoundKind(SoundHandle handle) {
    const auto[valid, index] = CheckSoundHandle(handle);
    if (!valid)
        return SoundKind::None;
    return sourceState[(size_t)index].kind;
}

//---------------------------

std::shared_ptr<Decoder::iDecoderFactory> StateProcessor::FindFactory(const char *uri) {
    const char *ext = strrchr(uri, '.');
    if (!ext)
        return nullptr;

    auto it = decoderFactories.find(ext + 1);
    if (it == decoderFactories.end())
        return nullptr;

    return it->second;
}

}
