#pragma once

#include <Foundation/Scripts/LuaPanic.h>
#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>

namespace MoonGlare::SoundSystem::Component {

/*@ [ComponentReference/SoundSourceComponentLuaWrap] SoundSource component
    Component is responsible for emitting sound @*/    
struct SoundSourceComponentLuaWrap : public ComponentScriptWrapTemplate<SoundSourceComponent> {
    static HandleApi handleApi;

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = SoundSourceComponentLuaWrap;
        return api
            .beginClass<LuaWrapper>("SoundSourceComponent")
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent:Play()`
    TODO @*/             
                .addFunction("Play", &LuaWrapper::Play)
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent:Pause()`
    TODO @*/                      
                .addFunction("Pause", &LuaWrapper::Pause)
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent:Stop()`
    TODO @*/                      
                .addFunction("Stop", &LuaWrapper::Stop)

/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent.state`
    TODO @*/      
                .addProperty("state", &LuaWrapper::GetState)
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent.position`
    TODO @*/                      
                .addProperty("position", &LuaWrapper::GetPosition)
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent.duration`
    TODO @*/                      
                .addProperty("duration", &LuaWrapper::GetDuration)
    
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent.file`
    TODO @*/      
                .addProperty("file", &LuaWrapper::GetURI, &LuaWrapper::SetUri)
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent.loop`
    TODO @*/                      
                .addProperty("loop", &LuaWrapper::GetLoop, &LuaWrapper::SetLoop)
/*@ [SoundSourceComponentLuaWrap/_] `SoundSourceComponent.kind`
    TODO @*/                      
                .addProperty("kind", &LuaWrapper::GetKind, &LuaWrapper::SetKind)
            .endClass();
    }

    const SoundHandle& Handle() const { return componentPtr->soundHandle; }
    SoundHandle& Handle() { return componentPtr->soundHandle; }

    const char* GetURI() const {
        Check();
        return handleApi.GetStreamURI(Handle());
    }
    void SetUri(const char *c) {
        Check();
        handleApi.ReopenStream(Handle(), c);
        handleApi.SetUserData(Handle(), owner.GetIntValue());
    }

    bool GetLoop() const {
        Check();
        return handleApi.GetLoop(Handle());
    }
    void SetLoop(bool v) {
        Check();
        handleApi.SetLoop(Handle(), v);
    }

    const char* GetKind() const {
        Check();
        switch (handleApi.GetSoundKind(Handle())) {
        case SoundKind::Auto: return "Auto";
        case SoundKind::Music: return "Music";
        case SoundKind::Effect: return "Effect";
        case SoundKind::None:
        default:
            return "None";
        }
    }
    void SetKind(const char* v) {
        Check();
        auto hash = Space::Utils::MakeHash32(v);
        switch (hash) {
        case "Music"_Hash32:
            return handleApi.SetSoundKind(Handle(), SoundKind::Music);
        case "Effect"_Hash32:
            return handleApi.SetSoundKind(Handle(), SoundKind::Effect);
        case "None"_Hash32:
            return handleApi.SetSoundKind(Handle(), SoundKind::None);
        case "Auto"_Hash32:
        default:
            return handleApi.SetSoundKind(Handle(), SoundKind::Auto);
        }
    }

    void Play() {
        Check();
        handleApi.Play(Handle());
    }
    void Pause() {
        Check();
        handleApi.Pause(Handle());
    }
    void Stop() {
        Check();
        handleApi.Stop(Handle());
    }

    float GetPosition() const {
        Check();
        return handleApi.GetTimePosition(Handle());
    }
    float GetDuration() const {
        Check();
        return handleApi.GetDuration(Handle());
    }

    const char* GetState() const {
        switch (handleApi.GetState(Handle())) {
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
