#include "HandleApi.h"
#include "StateProcessor.h"

namespace MoonGlare::SoundSystem {

HandleApi::HandleApi(StateProcessor *stateProcessor) : stateProcessor(stateProcessor) {
}

HandleApi::~HandleApi() {
    stateProcessor = nullptr;
}

void HandleApi::SetCallback(SoundHandle handle, iPlaybackWatcher *iface, UserData userData) const {
    assert(stateProcessor);
    if (!stateProcessor)
        return;
    stateProcessor->SetCallback(handle, iface, userData);
}

bool HandleApi::IsSoundHandleValid(SoundHandle handle) const {
    assert(stateProcessor);
    if (!stateProcessor)
        return false;
    return stateProcessor->IsSoundHandleValid(handle);
}

void HandleApi::Close(SoundHandle handle, bool ContinuePlaying){
    assert(stateProcessor);
    if (stateProcessor) {
        SetReleaseOnStop(handle, !ContinuePlaying);
        if (!ContinuePlaying)
            Stop(handle);
        stateProcessor->CloseSoundHandle(handle);
    }
}

SoundHandle HandleApi::Open(const std::string &uri, bool StartPlayback, SoundKind kind, bool ReleaseOnStop) {
    assert(stateProcessor);
    if (!stateProcessor)
        return SoundHandle::Invalid;

    SoundHandle handle = stateProcessor->AllocateSource();
    if (handle == SoundHandle::Invalid) {
        //todo: log error
        return SoundHandle::Invalid;
    }

    if (!stateProcessor->Open(handle, uri, kind)) {
        stateProcessor->ReleaseSource(handle);
        //todo: log error
        return SoundHandle::Invalid;
    }

    //std::unique_ptr<iSound> WorkThread::OpenSound(const std::string &uri, bool start, SoundKind kind) {
    //    SourceIndex si = stateProcessor->AllocateSource();
    //    if (si == InvalidSourceIndex)
    //        return nullptr;
    //    auto &state = storage->sourceState[(size_t)si];
    //    state.uri = uri;
    //    state.status = SourceStatus::InitPending;
    //    if (start)
    //        state.command = SourceCommand::ResumePlaying;
    //    stateProcessor->ActivateSource(si);
    //    return std::move(p);
    //}

    stateProcessor->ActivateSource(handle);
    if (StartPlayback)
        Play(handle);
    return handle;
}

void HandleApi::ReopenStream(SoundHandle &handle, const char *uri, SoundKind kind) {
    assert(stateProcessor);
    if (!stateProcessor)
        return;

    if (!stateProcessor->IsSoundHandleValid(handle)) {
        handle = Open(uri, false, kind, false);
        return;
    }

    stateProcessor->ReopenStream(handle, uri, kind);
}   

const char *HandleApi::GetStreamURI(SoundHandle handle) {
    assert(stateProcessor);
    if (!stateProcessor)
        return nullptr;
    return stateProcessor->GetStreamURI(handle);
}

SoundState HandleApi::GetState(SoundHandle handle) const {
    assert(stateProcessor);
    if (!stateProcessor)
        return SoundState::Invalid;
    switch (stateProcessor->GetStatus(handle)) {
    case SourceStatus::Playing:
        return SoundState::Playing;
    case SourceStatus::Paused:
        return SoundState::Paused;
    case SourceStatus::InitPending:
    case SourceStatus::Stopped:
        return SoundState::Stopped;
    case SourceStatus::FinitPending:
    case SourceStatus::Standby:
    case SourceStatus::Invalid:
    case SourceStatus::Inactive:
    default:
        return SoundState::Invalid;
    }
}

void HandleApi::Play(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::ResumePlaying);
}

void HandleApi::Pause(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::Pause);
}

void HandleApi::Stop(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::StopPlaying);
}

void HandleApi::SetLoop(SoundHandle handle, bool value) const{
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetLoop(handle, value);
}

bool HandleApi::GetLoop(SoundHandle handle) const{
    assert(stateProcessor);
    if (stateProcessor)
        return stateProcessor->GetLoop(handle);
    return false;
}

void HandleApi::SetSoundKind(SoundHandle handle, SoundKind value) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetSoundKind(handle, value);
}

SoundKind HandleApi::GetSoundKind(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        return stateProcessor->GetSoundKind(handle);
    return SoundKind::None;
}

float HandleApi::GetTimePosition(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        return stateProcessor->GetTimePosition(handle);
    return -1;
}

float HandleApi::GetDuration(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        return stateProcessor->GetDuration(handle);
    return -1;
}

void HandleApi::SetSourceGain(SoundHandle handle, float gain) const {
    assert(stateProcessor);
    //todo
}

float HandleApi::GetSourceGain(SoundHandle handle) const {
    assert(stateProcessor);
    //todo
    return -1;
}

void HandleApi::SetSourcePosition(SoundHandle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetSourcePosition(SoundHandle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetSourceVelocity(SoundHandle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetSourceVelocity(SoundHandle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetRelativeToListenerPosition(SoundHandle handle, bool value) {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetListenerGain(float gain) const {
    assert(stateProcessor);
    //todo
}

float HandleApi::GetListenerGain() const {
    assert(stateProcessor);
    //todo
    return -1;
}

void HandleApi::SetListenerPosition(SoundHandle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetListenerPosition(SoundHandle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetListenerVelovity(SoundHandle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetListenerVelovity(SoundHandle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetReleaseOnStop(SoundHandle handle, bool value) {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetReleaseOnStop(handle, value);
}

}
