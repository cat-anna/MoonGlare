#pragma once

#include <Foundation/Scripts/LuaPanic.h>
#include <Foundation/Scripts/ApiInit.h>

namespace MoonGlare::SoundSystem::Component {

struct SoundSourceComponentLuaWrap {
    //SoundSourceComponent *component;
    Entity owner;
    iSubsystemManager *subsystemManager;
    ComponentArray *componentArray;
    //mutable ComponentIndex index;
    mutable SoundSourceComponent *componentPtr;
    static HandleApi handleApi;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
        //index = component->GetComponentIndex(owner);
        componentPtr = componentArray->Get<SoundSourceComponent>(owner);
        //}
        if (componentPtr == nullptr) {
        //if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference deleted Transform component! ");
        }
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = SoundSourceComponentLuaWrap;
        return api
            .beginClass<LuaWrapper>("SoundSourceComponent")
                .addFunction("Play", &LuaWrapper::Play)
                .addFunction("Pause", &LuaWrapper::Pause)
                .addFunction("Stop", &LuaWrapper::Stop)

                .addProperty("State", &LuaWrapper::GetState)
                .addProperty("Position", &LuaWrapper::GetPosition)
                .addProperty("Duration", &LuaWrapper::GetDuration)

                .addProperty("File", &LuaWrapper::GetURI, &LuaWrapper::SetUri)
                .addProperty("Loop", &LuaWrapper::GetLoop, &LuaWrapper::SetLoop)
                .addProperty("Kind", &LuaWrapper::GetKind, &LuaWrapper::SetKind)
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
