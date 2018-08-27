#pragma once

#include <Foundation/ShortString.h>
#include <Foundation/Component/EventInfo.h>

namespace MoonGlare::Core::Scene {

enum class SceneState : unsigned {
    Invalid,
    Created,
    Started,
    Paused,
};

struct SceneStateChangeEvent {
    static constexpr char* EventName = "SceneStateChange";
    static constexpr char* HandlerName = "OnSceneStateChangeEvent";
    static constexpr bool Public = false;

    SceneState m_State;
    //ciScene *m_Scene;

    friend std::ostream& operator<<(std::ostream& out, const SceneStateChangeEvent & dt) {
        out << "SceneStateChangeEvent"
            << "[State:" << (int) dt.m_State
            //<< ";Scene:" << dt.m_Scene
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<SceneStateChangeEvent>("cSceneStateChangeEvent")
                .addData("State", (int SceneStateChangeEvent::*)&SceneStateChangeEvent::m_State, false)
                //.addData("Scene", &SceneStateChangeEvent::m_Scene, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

struct SetSceneEvent {
    static constexpr char* EventName = "SetSceneEvent";
    static constexpr char* HandlerName = "OnSetSceneEvent";
    static constexpr bool Public = true;

    BasicStaticString<63> sceneName;

    SetSceneEvent() = default;

    template<typename T>
    SetSceneEvent(T&& str) : sceneName(std::forward<T>(str)) {}

    friend std::ostream& operator<<(std::ostream& out, const SetSceneEvent & dt) {
        out << "SetSceneEvent"
            << "[Scene:" << dt.sceneName.c_str()
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<SetSceneEvent>("SetSceneEvent")
                .addConstructor<void(*)(const char*)>()

                .addProperty("Scene", &SetSceneEvent::GetSceneName, &SetSceneEvent::SetSceneName)

                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
                .addStaticInteger("EventId", Component::EventInfo<SetSceneEvent>::GetClassId())
            .endClass();
    }
private:
    void SetSceneName(const char *str) { sceneName = str; }
    const char *GetSceneName() const { return sceneName.c_str(); }
};

struct SetSceneChageFenceEvent {
    static constexpr char* EventName = "SetSceneChageFenceEvent";
    static constexpr char* HandlerName = "OnSetSceneChageFenceEvent";
    static constexpr bool Public = true;

    BasicStaticString<31> fenceId;
    bool active;

    SetSceneChageFenceEvent() = default;
    SetSceneChageFenceEvent(const char *str) {
        fenceId = str;
    }

    friend std::ostream& operator<<(std::ostream& out, const SetSceneChageFenceEvent & dt) {
        out << "SetSceneEvent"
            << "[Scene:" << dt.fenceId.c_str()
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<SetSceneChageFenceEvent>("SetSceneChageFenceEvent")
            //.addConstructor<void(*)(const char*)>()

            //.addProperty("Scene", &SetSceneEvent::GetSceneName, &SetSceneEvent::SetSceneName)

            .addStaticString("EventName", EventName)
            .addStaticString("HandlerName", HandlerName)
            .addStaticInteger("EventId", Component::EventInfo<SetSceneChageFenceEvent>::GetClassId())
            .endClass();
    }
private:
    //void SetSceneName(const char *str) { fenceId = str;  }
    //const char *GetSceneName() const {  return fenceId.c_str(); }
};

} //namespace MoonGlare::Core::Scene
