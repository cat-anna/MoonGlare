#pragma once

#include <cstdint>
#include <memory>
#include <sound_system/iHandleApi.hpp>
#include <string>
#include <string_view>
#include <svfs/hashes.hpp>

namespace MoonGlare::SoundSystem {

class StateProcessor;

class HandleApi : public iHandleApi {
public:
    explicit HandleApi(StateProcessor *stateProcessor);
    ~HandleApi() override = default;

    bool IsSoundHandleValid(SoundHandle handle) const override;
    void Close(SoundHandle handle, bool ContinuePlaying = false) override;
    SoundHandle Open(FileResourceId resource, bool StartPlayback = true, SoundKind kind = SoundKind::Auto,
                     bool ReleaseOnStop = true) override;

    void ReopenStream(SoundHandle &handle, FileResourceId resource, SoundKind kind = SoundKind::Auto) override;

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

    void SetReleaseOnStop(SoundHandle handle, bool value) override;

private:
    StateProcessor *const stateProcessor = nullptr;
};

} // namespace MoonGlare::SoundSystem
