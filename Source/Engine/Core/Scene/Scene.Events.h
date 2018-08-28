#pragma once

#include <Foundation/ShortString.h>
#include <Foundation/Scripts/ApiInit.h>
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

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
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

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
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

struct SetSceneChangeFenceEvent {
    static constexpr char* EventName = "SetSceneChangeFenceEvent";
    static constexpr char* HandlerName = "OnSetSceneChangeFenceEvent";
    static constexpr bool Public = true;

    BasicStaticString<31> fence;
    bool active;

    SetSceneChangeFenceEvent() = default;
    SetSceneChangeFenceEvent(const char *str, bool act) {
        fence = str;
        active = act;
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

            .addStaticString("EventName", EventName)
            .addStaticString("HandlerName", HandlerName)
            .addStaticInteger("EventId", Component::EventInfo<SetSceneChangeFenceEvent>::GetClassId())
        .endClass();
    }
private:
    void SetFence(const char *str) { fence = str;  }
    const char *GetFence() const {  return fence.c_str(); }
};

} //namespace MoonGlare::Core::Scene
