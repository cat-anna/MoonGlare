#pragma once

#include "../HandleApi.h"
#include <Foundation/Scripts/ApiInit.h>

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

    Entity e;          // ugly but acceptable, for now
    bool finishEvent;  // ugly but acceptable, for now
    bool autostart;    // ugly but acceptable, for now

    static HandleApi handleApi;  // ugly but acceptable, for now

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<SoundSourceComponent>("SoundSourceComponent")
                .addStaticString("ComponentName", ComponentName)
                .addFunction("Play", &SoundSourceComponent::Play)
                .addFunction("Pause", &SoundSourceComponent::Pause)
                .addFunction("Stop", &SoundSourceComponent::Stop)

                .addProperty("State", &SoundSourceComponent::GetState)
                .addProperty("Position", &SoundSourceComponent::GetPosition)
                .addProperty("Duration", &SoundSourceComponent::GetDuration)

                .addProperty("File", &SoundSourceComponent::GetURI, &SoundSourceComponent::SetUri)
                .addProperty("Loop", &SoundSourceComponent::GetLoop, &SoundSourceComponent::SetLoop)
                .addProperty("Kind", &SoundSourceComponent::GetKind, &SoundSourceComponent::SetKind)
            .endClass();
    }

private:
    const char* GetURI() const { return handleApi.GetStreamURI(handle);  }
    void SetUri(const char *c) { handleApi.ReopenStream(handle, c); autostart = false; }

    bool GetLoop() const { return handleApi.GetLoop(handle); }
    void SetLoop(bool v) { handleApi.SetLoop(handle, v); }

    const char* GetKind() const { 
        switch (handleApi.GetSoundKind(handle)) {
        case SoundKind::Auto: return "Auto";
        case SoundKind::Music: return "Music";
        case SoundKind::Effect: return "Effect";
        case SoundKind::None:
        default:
            return "None";
        }
    }
    void SetKind(const char* v) { 
        auto hash = Space::Utils::MakeHash32(v);
        switch (hash) {
        case "Music"_Hash32:
            return handleApi.SetSoundKind(handle, SoundKind::Music);
        case "Effect"_Hash32:
            return handleApi.SetSoundKind(handle, SoundKind::Effect);
        case "None"_Hash32:
            return handleApi.SetSoundKind(handle, SoundKind::None);
        case "Auto"_Hash32:
        default:
            return handleApi.SetSoundKind(handle, SoundKind::Auto);
        }
    }

    void Play() { handleApi.Play(handle); }
    void Pause() { handleApi.Pause(handle); autostart = false; }
    void Stop() { handleApi.Stop(handle); autostart = false; }

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
