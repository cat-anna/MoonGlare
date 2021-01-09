#pragma once

#include <cstdint>
#include <memory>
#include <resource_id.hpp>
#include <string>
#include <string_view>

namespace MoonGlare::SoundSystem {

class StateProcessor;
using UserData = uintptr_t;

enum class SoundHandle : uint32_t {
    Invalid = 0,
};

class iPlaybackWatcher {
public:
    //TODO: add some safety mechanism

    //virtual void OnStarted(SoundHandle handle, UserData userData) = 0;
    virtual void OnFinished(SoundHandle handle, bool loop, UserData userData) {}
};

enum class SoundState {
    Invalid,
    Playing,
    Paused,
    Stopped,
};

enum class SoundKind {
    Auto,   //autodetected by stream length
    Music,  //played by streaming
    Effect, //decoded to single buffer
    None,   //?
};

class iHandleApi {
public:
    virtual ~iHandleApi() = default;

    using Vector3f = float[3];

    virtual bool IsSoundHandleValid(SoundHandle handle) const = 0;
    virtual void Close(SoundHandle handle, bool ContinuePlaying = false) = 0;

    virtual SoundHandle Open(std::string_view uri, bool StartPlayback = true, SoundKind kind = SoundKind::Auto,
                             bool ReleaseOnStop = true) = 0;
    virtual SoundHandle Open(FileResourceId resource, bool StartPlayback = true, SoundKind kind = SoundKind::Auto,
                             bool ReleaseOnStop = true) = 0;

    virtual void ReopenStream(SoundHandle &handle, std::string_view uri, SoundKind kind = SoundKind::Auto) = 0;
    virtual void ReopenStream(SoundHandle &handle, FileResourceId resource, SoundKind kind = SoundKind::Auto) = 0;

    virtual std::string GetStreamResourceName(SoundHandle handle) = 0;
    virtual FileResourceId GetStreamResourceId(SoundHandle handle) = 0;

    virtual SoundState GetState(SoundHandle handle) const = 0;

    virtual void SetUserData(SoundHandle handle, UserData userData) const = 0;
    virtual void SetCallback(std::shared_ptr<iPlaybackWatcher> iface) const = 0;

    virtual void Play(SoundHandle handle) const = 0;
    virtual void Pause(SoundHandle handle) const = 0;
    virtual void Stop(SoundHandle handle) const = 0;
    virtual void SetLoop(SoundHandle handle, bool value) const = 0;
    virtual bool GetLoop(SoundHandle handle) const = 0;
    virtual void SetSoundKind(SoundHandle handle, SoundKind value) const = 0;
    virtual SoundKind GetSoundKind(SoundHandle handle) const = 0;

    virtual float GetTimePosition(SoundHandle handle) const = 0;
    virtual float GetDuration(SoundHandle handle) const = 0;

    virtual void SetSourceGain(SoundHandle handle, float gain) const = 0;
    virtual float GetSourceGain(SoundHandle handle) const = 0;
    virtual void SetSourcePosition(SoundHandle handle, const Vector3f coord) const = 0;
    virtual void GetSourcePosition(SoundHandle handle, Vector3f coord) const = 0;
    virtual void SetSourceVelocity(SoundHandle handle, const Vector3f coord) const = 0;
    virtual void GetSourceVelocity(SoundHandle handle, Vector3f coord) const = 0;
    virtual void SetRelativeToListenerPosition(SoundHandle handle, bool value) = 0;
    //AL_PITCH AL_DIRECTION

    virtual void SetListenerGain(float gain) const = 0;
    virtual float GetListenerGain() const = 0;
    virtual void SetListenerPosition(SoundHandle handle, const Vector3f coord) const = 0;
    virtual void GetListenerPosition(SoundHandle handle, Vector3f coord) const = 0;
    virtual void SetListenerVelocity(SoundHandle handle, const Vector3f coord) const = 0;
    virtual void GetListenerVelocity(SoundHandle handle, Vector3f coord) const = 0;

    virtual void SetReleaseOnStop(SoundHandle handle, bool value) = 0;

private:
    StateProcessor *stateProcessor = nullptr;
};

} // namespace MoonGlare::SoundSystem
