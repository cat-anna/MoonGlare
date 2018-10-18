#pragma once

#include <Foundation/StaticString.h>
#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Component/EventInfo.h>

namespace MoonGlare::Core::Scene {

struct SceneEventCommon {
    using SceneName = BasicStaticString<32, char>;
};

enum class SceneState : int {
    Invalid,
    BeforeDestruction,
    Started,
    Paused,
};

struct SceneStateChangeEvent : public SceneEventCommon {
    static constexpr char* EventName = "SceneStateChange";
    static constexpr char* HandlerName = "OnSceneStateChangeEvent";
    static constexpr bool Public = false;

    SceneState state;
    SceneName sceneName;

    friend std::ostream& operator<<(std::ostream& out, const SceneStateChangeEvent & dt) {
        out << "SceneStateChangeEvent"
            << "[Scene:" << dt.sceneName.c_str()
            << ",State:" << dt.GetSceneState()
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<SceneStateChangeEvent>("cSceneStateChangeEvent")
                .addProperty("Scene", &SceneStateChangeEvent::GetSceneName, &SceneStateChangeEvent::SetSceneName)
                .addProperty("State", &SceneStateChangeEvent::GetSceneState, &SceneStateChangeEvent::SetSceneState)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
                .addStaticInteger("EventId", Component::EventInfo<SceneStateChangeEvent>::GetClassId())
            .endClass();
    }
private:
    void SetSceneName(const char *str) { sceneName = str; }
    const char *GetSceneName() const { return sceneName.c_str(); }

    const char* GetSceneState() const {
        switch (state) {
        case SceneState::Invalid:
            return "Invalid";
        case SceneState::Started:
            return "Started";
        case SceneState::Paused:
            return "Paused";                  
        case SceneState::BeforeDestruction:
            return "BeforeDestruction";
        default:
            return nullptr;
        }
    }
    void SetSceneState(const char *str) {
        switch (Space::Utils::MakeHash32(str)) {
        case "Invalid"_Hash32:
            state = SceneState::Invalid;
            break;
        case "Started"_Hash32:
            state = SceneState::Started;
            break;
        case "Paused"_Hash32:
            state = SceneState::Paused;
            break;                        
        case "BeforeDestruction"_Hash32:
            state = SceneState::BeforeDestruction;
            break;
        }
    }
};

struct SetSceneEvent : public SceneEventCommon {
    static constexpr char* EventName = "SetSceneEvent";
    static constexpr char* HandlerName = "OnSetSceneEvent";
    static constexpr bool Public = true;

    SceneName sceneName;
    SceneName sceneTypeName;
    bool suspendCurrent;

    SetSceneEvent() = default;

    const SceneName& GetTypeName() const {
        if (sceneTypeName.empty())
            return sceneName;
        else
            return sceneTypeName;
    }

    SetSceneEvent(const char *name, bool pause = false, const char *typeName = nullptr) : 
           sceneName(name), suspendCurrent(pause), sceneTypeName(typeName) { }

    friend std::ostream& operator<<(std::ostream& out, const SetSceneEvent & dt) {
        out << "SetSceneEvent"
            << "[Scene:" << dt.sceneName.c_str()
            << ",TypeName:" << dt.sceneTypeName.c_str()
            << ",Suspend:" << (int)dt.suspendCurrent
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<SetSceneEvent>("SetSceneEvent")
                .addConstructor<void(*)(const char*, bool)>()

                .addProperty("Scene", &SetSceneEvent::GetSceneName, &SetSceneEvent::SetSceneName)
                .addProperty("TypeName", &SetSceneEvent::GetSceneTypeName, &SetSceneEvent::SetSceneTypeName)
                .addData("Suspend", &SetSceneEvent::suspendCurrent, true)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
                .addStaticInteger("EventId", Component::EventInfo<SetSceneEvent>::GetClassId())
            .endClass();
    }
private:
    void SetSceneName(const char *str) { sceneName = str; }
    const char *GetSceneName() const { return sceneName.c_str(); }
    void SetSceneTypeName(const char *str) { sceneTypeName = str; }
    const char *GetSceneTypeName() const { return sceneTypeName.c_str(); }
};

struct SetSceneChangeFenceEvent : public SceneEventCommon {
    static constexpr char* EventName = "SetSceneChangeFenceEvent";
    static constexpr char* HandlerName = "OnSetSceneChangeFenceEvent";
    static constexpr bool Public = true;

    BasicStaticString<32> fence;
    SceneName sceneName;
    bool active;

    SetSceneChangeFenceEvent() = default;
    SetSceneChangeFenceEvent(const char *str, bool act, const char *scene = nullptr) {
        fence = str;
        active = act;
        sceneName = scene;
    }

    friend std::ostream& operator<<(std::ostream& out, const SetSceneChangeFenceEvent & dt) {
        out << "SetSceneChangeFenceEvent"
            << "[Fence:" << dt.fence.c_str()
            << ",Active:" << dt.active
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
        .beginClass<SetSceneChangeFenceEvent>("SetSceneChangeFenceEvent")
            .addConstructor<void(*)(const char*, bool)>()

            .addProperty("Fence", &SetSceneChangeFenceEvent::GetFence, &SetSceneChangeFenceEvent::SetFence)
            .addData("Active", &SetSceneChangeFenceEvent::active, true)
            .addProperty("Scene", &SetSceneChangeFenceEvent::GetSceneName, &SetSceneChangeFenceEvent::SetSceneName)

            .addStaticString("EventName", EventName)
            .addStaticString("HandlerName", HandlerName)
            .addStaticInteger("EventId", Component::EventInfo<SetSceneChangeFenceEvent>::GetClassId())
        .endClass();
    }
private:
    void SetFence(const char *str) { fence = str;  }
    const char *GetFence() const {  return fence.c_str(); }
    void SetSceneName(const char *str) { sceneName = str; }
    const char *GetSceneName() const { return sceneName.c_str(); }
};

} //namespace MoonGlare::Core::Scene
