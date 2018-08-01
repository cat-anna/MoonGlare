#pragma once

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
    SceneState m_State;
    ciScene *m_Scene;

    friend std::ostream& operator<<(std::ostream& out, const SceneStateChangeEvent & dt) {
        out << "SceneStateChangeEvent"
            << "[State:" << (int) dt.m_State
            << ";Scene:" << dt.m_Scene
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<SceneStateChangeEvent>("cSceneStateChangeEvent")
                .addData("State", (int SceneStateChangeEvent::*)&SceneStateChangeEvent::m_State, false)
                .addData("Scene", &SceneStateChangeEvent::m_Scene, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

} //namespace MoonGlare::Core::Scene
