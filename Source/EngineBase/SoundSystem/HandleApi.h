#pragma once

#include <Foundation/iFileSystem.h>

namespace MoonGlare::SoundSystem {

class StateProcessor;

enum class Handle : uint32_t {
    Invalid = 0,
};

enum class SoundState {
    Invalid, Playing, Paused, Stopped,
};

enum class SoundKind {
    Music,  //played by streaming
    Effect, //decoded to single buffer
};

class HandleApi {
public:
    explicit HandleApi(StateProcessor *stateProcessor = nullptr);
    ~HandleApi();

    using Vector = float[3];

    bool IsHandleValid(Handle handle) const;
    void Close(Handle handle, bool ContinuePlaying = false);
    Handle Open(const std::string &uri, bool StartPlayback = true, SoundKind kind = SoundKind::Music, bool ReleaseOnStop = true);

    SoundState GetState(Handle handle) const;

    void Play(Handle handle) const;
    void Pause(Handle handle) const;
    void Stop(Handle handle) const;
    void SetLoop(Handle handle, bool value) const;
    bool GetLoop(Handle handle) const;

    float GetTimePosition(Handle handle) const;
    float GetDuration(Handle handle) const;

    void SetSourceGain(Handle handle, float gain) const;
    float GetSourceGain(Handle handle) const;
    void SetSourcePosition(Handle handle, const Vector coord) const;
    void GetSourcePosition(Handle handle, Vector coord) const;
    void SetSourceVelocity(Handle handle, const Vector coord) const;
    void GetSourceVelocity(Handle handle, Vector coord) const;
    void SetRelativeToListenerPosition(Handle handle, bool value);

    void SetListenerGain(float gain) const;
    float GetListenerGain() const;
    void SetListenerPosition(Handle handle, const Vector coord) const;
    void GetListenerPosition(Handle handle, Vector coord) const;
    void SetListenerVelovity(Handle handle, const Vector coord) const;
    void GetListenerVelovity(Handle handle, Vector coord) const;

    void SetReleaseOnStop(Handle handle, bool value);
private:
    StateProcessor *stateProcessor = nullptr;
};

}
