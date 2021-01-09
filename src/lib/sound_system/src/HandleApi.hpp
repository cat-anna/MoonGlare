#pragma once

#include <cstdint>
#include <memory>
#include <readonly_file_system.h>
#include <sound_system/iHandleApi.hpp>
#include <string>
#include <string_view>
#include <svfs/hashes.hpp>

namespace MoonGlare::SoundSystem {

class StateProcessor;

class HandleApi : public iHandleApi {
public:
    explicit HandleApi(StateProcessor *stateProcessor, iReadOnlyFileSystem *fs);
    ~HandleApi() override;

    bool IsSoundHandleValid(SoundHandle handle) const override;
    void Close(SoundHandle handle, bool ContinuePlaying = false) override;
    SoundHandle Open(std::string_view uri, bool StartPlayback = true, SoundKind kind = SoundKind::Auto,
                     bool ReleaseOnStop = true) override;
    SoundHandle Open(FileResourceId resource, bool StartPlayback = true, SoundKind kind = SoundKind::Auto,
                     bool ReleaseOnStop = true) override;

    void ReopenStream(SoundHandle &handle, std::string_view uri, SoundKind kind = SoundKind::Auto) override;
    void ReopenStream(SoundHandle &handle, FileResourceId resource, SoundKind kind = SoundKind::Auto) override;

    std::string GetStreamResourceName(SoundHandle handle) override;
    FileResourceId GetStreamResourceId(SoundHandle handle) override;

    SoundState GetState(SoundHandle handle) const override;

    void SetUserData(SoundHandle handle, UserData userData) const override;
    void SetCallback(std::shared_ptr<iPlaybackWatcher> iface) const override;

    void Play(SoundHandle handle) const override;
    void Pause(SoundHandle handle) const override;
    void Stop(SoundHandle handle) const override;
    void SetLoop(SoundHandle handle, bool value) const override;
    bool GetLoop(SoundHandle handle) const override;
    void SetSoundKind(SoundHandle handle, SoundKind value) const override;
    SoundKind GetSoundKind(SoundHandle handle) const override;

    float GetTimePosition(SoundHandle handle) const override;
    float GetDuration(SoundHandle handle) const override;

    void SetSourceGain(SoundHandle handle, float gain) const override;
    float GetSourceGain(SoundHandle handle) const override;
    void SetSourcePosition(SoundHandle handle, const Vector3f coord) const override;
    void GetSourcePosition(SoundHandle handle, Vector3f coord) const override;
    void SetSourceVelocity(SoundHandle handle, const Vector3f coord) const override;
    void GetSourceVelocity(SoundHandle handle, Vector3f coord) const override;
    void SetRelativeToListenerPosition(SoundHandle handle, bool value) override;
    //AL_PITCH AL_DIRECTION

    void SetListenerGain(float gain) const override;
    float GetListenerGain() const override;
    void SetListenerPosition(SoundHandle handle, const Vector3f coord) const override;
    void GetListenerPosition(SoundHandle handle, Vector3f coord) const override;
    void SetListenerVelocity(SoundHandle handle, const Vector3f coord) const override;
    void GetListenerVelocity(SoundHandle handle, Vector3f coord) const override;

    void SetReleaseOnStop(SoundHandle handle, bool value) override;

private:
    StateProcessor *const stateProcessor = nullptr;
    iReadOnlyFileSystem *const fs = nullptr;
};

} // namespace MoonGlare::SoundSystem
