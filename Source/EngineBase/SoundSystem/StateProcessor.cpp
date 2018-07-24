#include <chrono>

#include "StateProcessor.h"
#include "WorkThread.h"

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare::SoundSystem {

StateProcessor::StateProcessor(iFileSystem * fs) : fileSystem(fs) { }
StateProcessor::~StateProcessor() {}

void StateProcessor::PrintState() const {
    AddLogf(Debug, "SoundSystem state: ActiveSources:%u StandbySources:%u AllocatedBuffers:%u buffersInUse:%u",
        activeSources.Allocated(), standbySources.Allocated(),
        allocatedBuffersCount, allocatedBuffersCount - standbyBuffers.Allocated()
    );
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
    sourceStateGeneration.fill(InvalidHandleGeneration);

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

    //todo InvalidHandleGenration

    for (auto index : standbySources) {
        auto &state = sourceState[(size_t)index];

        alDeleteSources(1, &state.sourceHandle);
        state.sourceHandle = InvalidSoundSource;
        state.status = SourceStatus::Invalid;

        assert(state.bufferCount == 0);
    }
    standbySources.ClearAllocation();
    sourceState.ClearAllocation();
    decoderFactories.clear();

#ifdef DEBUG
    for (size_t index = 0; index < sourceState.Capacity(); ++index) {
        auto &state = sourceState[index];
        assert(state.status == SourceStatus::Invalid);
        assert(state.bufferCount == 0);
        assert(state.sourceHandle == InvalidSoundSource);
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
            ProcessPlayState(state);
            break;
        case SourceStatus::Paused:
            break;
        case SourceStatus::Stopped:
            if (state.releaseOnStop)
                return SourceProcessStatus::ReleaseAndRemove;
            break;
        case SourceStatus::FinitPending:
            assert(false);
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
            if (state.Playable()) {
                if (state.status == SourceStatus::Paused) {
                    AddLogf(Debug, "Playing resumed: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
                    state.sourceHandle.Play();
                    CheckOpenAlError();
                }
                else
                    if (state.streamFinished) {
                        state.decoder->Reset();
                        state.streamFinished = false;
                        state.processedBuffers = 0;
                        state.processedBytes = 0;
                        state.processedSeconds = 0;
                    }
                state.status = SourceStatus::Playing;
            }
            break;
        case SourceCommand::Pause:
            if (state.status == SourceStatus::Playing) {
                AddLogf(Debug, "Playing paused: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
                state.sourceHandle.Pause();
                CheckOpenAlError();
                state.status = SourceStatus::Paused;
            }
            break;
        case SourceCommand::StopPlaying:
            if (state.Playable()) {
                state.status = SourceStatus::Stopped;
                state.sourceHandle.Stop();
                state.decoder->Reset();
                state.processedBuffers = 0;
                state.processedBytes = 0;
                state.processedSeconds = 0;
                ReleaseSourceBufferQueue(state);
                AddLogf(Debug, "Playing stopped: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
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

void StateProcessor::ProcessPlayState(SourceState &state) {
    while (true) {
        ALint processedBuffers = state.sourceHandle.GetProcessedBuffers();
        CheckOpenAlError();
        if (processedBuffers <= 0)
            break;

        SoundBuffer sb = state.sourceHandle.UnqueueBuffer();
        CheckOpenAlError();
        if (!sb)
            break;
        state.processedSeconds += sb.GetDuration();
        ++state.processedBuffers;
        --state.bufferCount;
        if (!LoadBuffer(state, sb)) {
            ReleaseBuffer(sb);
        }
    };

    ALint queuedBuffers = state.sourceHandle.GetQueuedBuffers();
    CheckOpenAlError();

    if (queuedBuffers < conf::MaxBuffersPerSource && !state.streamFinished) {
        SoundBuffer b = GetNextBuffer();
        if (!LoadBuffer(state, b))
            ReleaseBuffer(b);
        ++queuedBuffers;
        if (queuedBuffers == 1 && state.sourceHandle.GetState() != AL_PLAYING) {
            CheckOpenAlError();
            state.sourceHandle.Play();
            CheckOpenAlError();
            AddLogf(Debug, "Playback started %s Source:%d", state.uri.c_str(), (int)state.sourceHandle);
        }
        return;
    }

    if (queuedBuffers == 0) {
        if (state.streamFinished && state.sourceHandle.GetState() == AL_STOPPED) {
            CheckOpenAlError();
            state.status = SourceStatus::Stopped;
            AddLogf(Debug, "Playing finished: %s Source: %d processed buffers: %u bytes: %6.2f Mib", state.uri.c_str(), (int)state.sourceHandle, state.processedBuffers, (float)state.processedBytes / (1024.0f*1024.0f));
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
        state.sourceHandle.QueueBuffer(sb);
        CheckOpenAlError();
        ++state.bufferCount;
        return true;
    }
    else {
        return false;
    }
}

//---------------------------

void StateProcessor::ReleaseSourceBufferQueue(SourceState &state) {
    while (true) {
        SoundBuffer b = state.sourceHandle.UnqueueBuffer();
        CheckOpenAlError();
        if (!b)
            break;
        --state.bufferCount;
        assert(state.bufferCount <= conf::MaxBuffersPerSource);
        ReleaseBuffer(b);
    }
    assert(state.bufferCount == 0);
    assert(state.sourceHandle.GetProcessedBuffers() == 0);
    assert(state.sourceHandle.GetQueuedBuffers() == 0);
}

bool StateProcessor::InitializeSource(SourceState &state) {
    auto factory = FindFactory(state.uri);
    if (!factory)
        return false;
    state.decoder = factory->CreateDecoder();
    if (!state.decoder)
        return false;

    StarVFS::ByteTable data;
    if (!fileSystem->OpenFile(data, state.uri)) {
        return false;
    }

    if (!state.decoder->SetData(std::move(data), state.uri)) {
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
    if (allocatedBuffersCount + conf::BufferGenCount > conf::MaxBuffers)
        return false;
    SoundBuffer arr[conf::BufferGenCount] = {};
    alGenBuffers(conf::BufferGenCount, &arr[0]);
    CheckOpenAlError();
    standbyBuffers.append_copy(arr, conf::BufferGenCount);
    allocatedBuffersCount += conf::BufferGenCount;
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
    size_t canAlloc = std::min((size_t)conf::SourceGenCount, conf::MaxSources - sourceState.Allocated());
    if (canAlloc == 0)
        return false;
    SoundSource::type arr[conf::SourceGenCount] = {};
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
        ss.sourceHandle = arr[i];
        ss.status = SourceStatus::Standby;
    }
    AddLogf(Debug, "Generated sources. New count: %d", (int)sourceState.Allocated());
    return true;
}

Handle StateProcessor::AllocateSource() {
    auto index = GetNextSource();
    if (index == InvalidSourceIndex)
        return Handle::Invalid;
    return GetHandle(index);
}

void StateProcessor::ReleaseSource(Handle handle) {
    const auto[valid, index] = CheckHandle(handle);
    if (!valid)
        return;

    CloseHandle(handle);
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
    assert(state.bufferCount == 0);
    state.processedBuffers = state.processedBytes = 0;
    state.status = SourceStatus::Inactive;
    state.command = SourceCommand::None;
    state.releaseOnStop = false;
    state.streamFinished = false;
    state.processedBuffers = 0;
    state.processedBytes = 0;
    state.processedSeconds = 0;
    state.duration = 0;
    if (sourceStateGeneration[(size_t)si] == InvalidHandleGeneration)
        ++sourceStateGeneration[(size_t)si];
    AddLogf(Debug, "Allocated source index:%d handle:%d", (int)si, (int)state.sourceHandle);
    return si;
}

void StateProcessor::ReleaseSource(SourceIndex s) {
    if (s == InvalidSourceIndex)
        return;

    auto &state = sourceState[(size_t)s];
    assert(state.status != SourceStatus::Invalid);
    state.sourceHandle.Stop();
    ReleaseSourceBufferQueue(state);
    state.command = SourceCommand::None;
    state.status = SourceStatus::Standby;
    state.decoder.reset();
    ++sourceStateGeneration[(size_t)s];
    {
        lock_guard lock(standbySourcesMutex);
        standbySources.push(s);
    }

    AddLogf(Debug, "Released source index:%d handle:%d", (int)s, (int)state.sourceHandle);
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

//---------------------------

Handle StateProcessor::GetHandle(SourceIndex s) {
    if (s == InvalidSourceIndex)
        return Handle::Invalid;
    HandleComposite hc;
    hc.generation = sourceStateGeneration[(size_t)s];
    if (hc.generation == InvalidHandleGeneration)
        return Handle::Invalid;
    hc.index = s;
    return hc.handle;
}

std::pair<bool, SourceIndex> StateProcessor::CheckHandle(Handle handle) const {
    HandleComposite hc;
    hc.handle = handle;
    if (hc.generation == InvalidHandleGeneration)
        return { false, InvalidSourceIndex };
    if (hc.index >= static_cast<SourceIndex>(conf::MaxSources))
        return { false, InvalidSourceIndex };
    if (sourceStateGeneration[(size_t)hc.index] != hc.generation)
        return { false, InvalidSourceIndex };
    return { true, hc.index };
}

void StateProcessor::SetCommand(Handle handle, SourceCommand command) {
    const auto[valid, index] = CheckHandle(handle);
    if (valid)
        sourceState[(size_t)index].command = command;
}

SoundSource StateProcessor::GetSoundSource(Handle handle) {
    const auto[valid, index] = CheckHandle(handle);
    if (valid)
        return sourceState[(size_t)index].sourceHandle;
    return InvalidSoundSource;
}

SourceStatus StateProcessor::GetStatus(Handle handle) {
    const auto[valid, index] = CheckHandle(handle);
    if (valid)
        return sourceState[(size_t)index].status;
    return SourceStatus::Invalid;
}

void StateProcessor::SetReleaseOnStop(Handle handle, bool value) {
    const auto[valid, index] = CheckHandle(handle);
    if (valid)
        sourceState[(size_t)index].releaseOnStop = value;
}

void StateProcessor::ActivateSource(Handle handle) {
    const auto[valid, index] = CheckHandle(handle);
    if (valid) 
        ActivateSource(index);
}

void StateProcessor::DeactivateSource(Handle handle) {
    assert(false);
}

void StateProcessor::CloseHandle(Handle handle) {
    const auto[valid, index] = CheckHandle(handle);
    if (valid)
        ++sourceStateGeneration[(size_t)index];
}

bool StateProcessor::Open(Handle handle, const std::string &uri, SoundKind kind) {
    const auto[valid, index] = CheckHandle(handle);
    if (!valid)
        return false;

    auto &state = sourceState[(size_t)index];
    assert(state.status == SourceStatus::Inactive);
    if (state.status != SourceStatus::Inactive)
        return false;

    std::string cpy = uri;
    state.uri.swap(cpy);
    assert(!state.decoder);
    state.decoder.reset();

    //todo: kind

    return true;
}

float StateProcessor::GetDuration(Handle handle) const {
    const auto[valid, index] = CheckHandle(handle);
    if (!valid)
        return -1.0f;
    auto &state = sourceState[(size_t)index];
    if (state.duration <= 0) {

    }
    return state.duration;
        //return sourceState[(size_t)index].decoder->getd;
}

float StateProcessor::GetTimePosition(Handle handle) const {
    const auto[valid, index] = CheckHandle(handle);
    if (!valid)
        return -1.0f;
    auto &state = sourceState[(size_t)index];
    return state.processedSeconds + state.sourceHandle.GetTimePosition();
}

//---------------------------

std::shared_ptr<Decoder::iDecoderFactory> StateProcessor::FindFactory(const std::string &uri) {
    const char *ext = strrchr(uri.c_str(), '.');
    if (!ext)
        return nullptr;

    auto it = decoderFactories.find(ext + 1);
    if (it == decoderFactories.end())
        return nullptr;

    return it->second;
}

}
