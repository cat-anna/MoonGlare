#include "HandleApi.hpp"
#include "StateProcessor.hpp"
#include <orbit_logger.h>

namespace MoonGlare::SoundSystem {

HandleApi::HandleApi(StateProcessor *stateProcessor) : stateProcessor(stateProcessor) {
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
    if (!ContinuePlaying) {
        Stop(handle);
    }
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
    if (StartPlayback) {
        Play(handle);
    }

    SetReleaseOnStop(handle, ReleaseOnStop);

    return handle;
}

void HandleApi::ReopenStream(SoundHandle &handle, FileResourceId resource, SoundKind kind) {
    if (!stateProcessor->IsSoundHandleValid(handle)) {
        handle = Open(resource, false, kind, false);
        return;
    }
    stateProcessor->ReopenStream(handle, resource, kind);
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
    stateProcessor->SetCommand(handle, SourceCommand::StopPlaying);
}

void HandleApi::SetLoop(SoundHandle handle, bool value) const {
    stateProcessor->SetLoop(handle, value);
}

bool HandleApi::GetLoop(SoundHandle handle) const {
    return stateProcessor->GetLoop(handle);
}

void HandleApi::SetSoundKind(SoundHandle handle, SoundKind value) const {
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

void HandleApi::SetReleaseOnStop(SoundHandle handle, bool value) {
    stateProcessor->SetReleaseOnStop(handle, value);
}

} // namespace MoonGlare::SoundSystem
