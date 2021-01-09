#include "HandleApi.hpp"
#include "StateProcessor.hpp"
#include <orbit_logger.h>

namespace MoonGlare::SoundSystem {

HandleApi::HandleApi(StateProcessor *stateProcessor, iReadOnlyFileSystem *fs) : stateProcessor(stateProcessor), fs(fs) {
}

HandleApi::~HandleApi() {
}

void HandleApi::SetUserData(SoundHandle handle, UserData userData) const {
    stateProcessor->SetUserData(handle, userData);
}

void HandleApi::SetCallback(std::shared_ptr<iPlaybackWatcher> iface) const {
    stateProcessor->SetCallback(std::move(iface));
}

bool HandleApi::IsSoundHandleValid(SoundHandle handle) const {
    return stateProcessor->IsSoundHandleValid(handle);
}

void HandleApi::Close(SoundHandle handle, bool ContinuePlaying) {
    SetReleaseOnStop(handle, !ContinuePlaying);
    if (!ContinuePlaying)
        Stop(handle);
    stateProcessor->CloseSoundHandle(handle);
}

SoundHandle HandleApi::Open(FileResourceId resource, bool StartPlayback, SoundKind kind, bool ReleaseOnStop) {
    SoundHandle handle = stateProcessor->AllocateSource();
    if (handle == SoundHandle::Invalid) {
        //todo: log error
        return SoundHandle::Invalid;
    }

    if (!stateProcessor->Open(handle, resource, kind)) {
        stateProcessor->ReleaseSource(handle);
        //todo: log error
        return SoundHandle::Invalid;
    }

    stateProcessor->ActivateSource(handle);
    if (StartPlayback)
        Play(handle);

    SetReleaseOnStop(handle, ReleaseOnStop);
}

SoundHandle HandleApi::Open(std::string_view uri, bool StartPlayback, SoundKind kind, bool ReleaseOnStop) {
    return Open(fs->GetResourceByPath(uri), StartPlayback, kind, ReleaseOnStop);
}

void HandleApi::ReopenStream(SoundHandle &handle, FileResourceId resource, SoundKind kind) {
    if (!stateProcessor->IsSoundHandleValid(handle)) {
        handle = Open(resource, false, kind, false);
        return;
    }
    stateProcessor->ReopenStream(handle, resource, kind);
}

void HandleApi::ReopenStream(SoundHandle &handle, std::string_view uri, SoundKind kind) {
    if (!stateProcessor->IsSoundHandleValid(handle)) {
        handle = Open(uri, false, kind, false);
        return;
    }

    stateProcessor->ReopenStream(handle, fs->GetResourceByPath(uri), kind);
}

std::string HandleApi::GetStreamResourceName(SoundHandle handle) {
    return fs->GetNameOfResource(GetStreamResourceId(handle));
}

FileResourceId HandleApi::GetStreamResourceId(SoundHandle handle) {
    return stateProcessor->GetStreamResourceId(handle);
}

SoundState HandleApi::GetState(SoundHandle handle) const {
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
    stateProcessor->SetCommand(handle, SourceCommand::ResumePlaying);
}

void HandleApi::Pause(SoundHandle handle) const {
    stateProcessor->SetCommand(handle, SourceCommand::Pause);
}

void HandleApi::Stop(SoundHandle handle) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetCommand(handle, SourceCommand::StopPlaying);
}

void HandleApi::SetLoop(SoundHandle handle, bool value) const {
    stateProcessor->SetLoop(handle, value);
}

bool HandleApi::GetLoop(SoundHandle handle) const {
    return stateProcessor->GetLoop(handle);
}

void HandleApi::SetSoundKind(SoundHandle handle, SoundKind value) const {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetSoundKind(handle, value);
}

SoundKind HandleApi::GetSoundKind(SoundHandle handle) const {
    return stateProcessor->GetSoundKind(handle);
}

float HandleApi::GetTimePosition(SoundHandle handle) const {
    return stateProcessor->GetTimePosition(handle);
}

float HandleApi::GetDuration(SoundHandle handle) const {
    return stateProcessor->GetDuration(handle);
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

void HandleApi::SetSourcePosition(SoundHandle handle, const Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetSourcePosition(SoundHandle handle, Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetSourceVelocity(SoundHandle handle, const Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetSourceVelocity(SoundHandle handle, Vector3f coord) const {
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

void HandleApi::SetListenerPosition(SoundHandle handle, const Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetListenerPosition(SoundHandle handle, Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetListenerVelocity(SoundHandle handle, const Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::GetListenerVelocity(SoundHandle handle, Vector3f coord) const {
    assert(stateProcessor);
    //todo
}

void HandleApi::SetReleaseOnStop(SoundHandle handle, bool value) {
    assert(stateProcessor);
    if (stateProcessor)
        stateProcessor->SetReleaseOnStop(handle, value);
}

} // namespace MoonGlare::SoundSystem
