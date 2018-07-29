#pragma once

#include <Foundation/iFileSystem.h>

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

    bool IsSoundHandleValid(SoundHandle handle) const;
    void Close(SoundHandle handle, bool ContinuePlaying = false);
    SoundHandle Open(const std::string &uri, bool StartPlayback = true, SoundKind kind = SoundKind::Music, bool ReleaseOnStop = true);
    void ReopenStream(SoundHandle handle, const char *uri);
    const char *GetStreamURI(SoundHandle handle);

    SoundState GetState(SoundHandle handle) const;

    void SetCallback(SoundHandle handle, iPlaybackWatcher *iface, UserData userData) const;

    void Play(SoundHandle handle) const;
    void Pause(SoundHandle handle) const;
    void Stop(SoundHandle handle) const;
    void SetLoop(SoundHandle handle, bool value) const;
    bool GetLoop(SoundHandle handle) const;

    float GetTimePosition(SoundHandle handle) const;
    float GetDuration(SoundHandle handle) const;

    void SetSourceGain(SoundHandle handle, float gain) const;
    float GetSourceGain(SoundHandle handle) const;
    void SetSourcePosition(SoundHandle handle, const Vector coord) const;
    void GetSourcePosition(SoundHandle handle, Vector coord) const;
    void SetSourceVelocity(SoundHandle handle, const Vector coord) const;
    void GetSourceVelocity(SoundHandle handle, Vector coord) const;
    void SetRelativeToListenerPosition(SoundHandle handle, bool value);
    //AL_PITCH AL_DIRECTION    

    void SetListenerGain(float gain) const;
    float GetListenerGain() const;
    void SetListenerPosition(SoundHandle handle, const Vector coord) const;
    void GetListenerPosition(SoundHandle handle, Vector coord) const;
    void SetListenerVelovity(SoundHandle handle, const Vector coord) const;
    void GetListenerVelovity(SoundHandle handle, Vector coord) const;

    void SetReleaseOnStop(SoundHandle handle, bool value);
private:
    StateProcessor *stateProcessor = nullptr;
};

}
