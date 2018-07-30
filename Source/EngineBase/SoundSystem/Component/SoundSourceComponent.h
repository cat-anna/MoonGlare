#pragma once

#include "../HandleApi.h"
#include <EngineBase/Script/ApiInit.h>

namespace MoonGlare::SoundSystem::Component {

enum class PositionMode : uint8_t {
    None,
    RelativeToListener,
    Absolute,

    Default = RelativeToListener,
};

struct SoundSourceComponent  {
    static constexpr char* ComponentName = "SoundSource";

    SoundHandle handle;

    Entity e;        // ugly but acceptable, for now
    union {
        uint32_t flags;
        struct {
            bool finishEvent;  // ugly but acceptable, for now
            bool autostart;  // ugly but acceptable, for now
        };
    };

    static HandleApi handleApi;  // ugly but acceptable, for now

    static Script::ApiInitializer RegisterScriptApi(Script::ApiInitializer api) {
        return api
            .beginClass<SoundSourceComponent>("SoundSourceComponent")
                .addStaticString("ComponentName", ComponentName)
                .addFunction("Play", &SoundSourceComponent::Play)
                .addFunction("Pause", &SoundSourceComponent::Pause)
                .addFunction("Stopped", &SoundSourceComponent::Stop)

                .addProperty("State", &SoundSourceComponent::GetState)
                .addProperty("Position", &SoundSourceComponent::GetPosition)
                .addProperty("Duration", &SoundSourceComponent::GetDuration)

                .addProperty("File", &SoundSourceComponent::GetURI, &SoundSourceComponent::SetUri)
                .addProperty("Loop", &SoundSourceComponent::GetLoop, &SoundSourceComponent::SetLoop)
            .endClass();
    }

private:
    const char* GetURI() const { return handleApi.GetStreamURI(handle);  }
    void SetUri(const char *c) { handleApi.ReopenStream(handle, c); autostart = false; }

    bool GetLoop() const { return handleApi.GetLoop(handle); }
    void SetLoop(bool v) { handleApi.SetLoop(handle, v); }

    void Play() { handleApi.Play(handle); }
    void Pause() { handleApi.Pause(handle); }
    void Stop() { handleApi.Stop(handle); }

    float GetPosition() const { return handleApi.GetTimePosition(handle); }
    float GetDuration() const { return handleApi.GetDuration(handle); }

    const char* GetState() const {
        switch (handleApi.GetState(handle)) {
        case SoundState::Playing: return "Playing";
        case SoundState::Paused: return "Paused";
        case SoundState::Stopped: return "Stop";
        case SoundState::Invalid: 
        default:
            return "Unknown";
        }
    }
};

}
