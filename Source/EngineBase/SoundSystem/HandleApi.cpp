
#include "HandleApi.h"
#include "StateProcessor.h"

namespace MoonGlare::SoundSystem {

HandleApi::HandleApi(StateProcessor *stateProcessor) : stateProcessor(stateProcessor) {
}

HandleApi::~HandleApi() {
    stateProcessor = nullptr;
}

bool HandleApi::IsHandleValid(Handle handle) const {
    assert(stateProcessor);
    if (!stateProcessor)
        return false;
    return stateProcessor->IsHandleValid(handle);
}

void HandleApi::Close(Handle handle, bool ContinuePlaying){
    assert(stateProcessor);
    if (stateProcessor) {
        SetReleaseOnStop(handle, !ContinuePlaying);
        if (!ContinuePlaying)
            Stop(handle);
        stateProcessor->CloseHandle(handle);
    }
}

Handle HandleApi::Open(const std::string &uri, bool StartPlayback, SoundKind kind, bool ReleaseOnStop) {
    assert(stateProcessor);
    if (!stateProcessor)
        return Handle::Invalid;

    Handle handle = stateProcessor->AllocateSource();
    if (handle == Handle::Invalid) {
        //todo: log error
        return Handle::Invalid;
    }

    if (!stateProcessor->Open(handle, uri, kind)) {
        stateProcessor->ReleaseSource(handle);
        //todo: log error
        return Handle::Invalid;
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

SoundState HandleApi::GetState(Handle handle) const {
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

void HandleApi::Play(Handle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::ResumePlaying);
}

void HandleApi::Pause(Handle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::Pause);
}

void HandleApi::Stop(Handle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::StopPlaying);
}

void HandleApi::SetLoop(Handle handle, bool value) const{
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetLoop(handle, value);
}

float HandleApi::GetTimePosition(Handle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        return stateProcessor->GetTimePosition(handle);
    return -1;
}

float HandleApi::GetDuration(Handle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        return stateProcessor->GetDuration(handle);
    return -1;
}

void HandleApi::SetSourceGain(Handle handle, float gain) const {
    assert(stateProcessor);
    //todo
}

float HandleApi::GetSourceGain(Handle handle) const {
    assert(stateProcessor);
    //todo
    return -1;
}

void HandleApi::SetSourcePosition(Handle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetSourcePosition(Handle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetSourceVelocity(Handle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetSourceVelocity(Handle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetRelativeToListenerPosition(Handle handle, bool value) {
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

void HandleApi::SetListenerPosition(Handle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetListenerPosition(Handle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetListenerVelovity(Handle handle, const Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetListenerVelovity(Handle handle, Vector coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetReleaseOnStop(Handle handle, bool value) {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetReleaseOnStop(handle, value);
}

}
