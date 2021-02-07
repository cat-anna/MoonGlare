#pragma once

// #include <Foundation/StaticString.h>
// #include <Foundation/Scripts/ApiInit.h>
// #include <Foundation/Component/EventInfo.h>

namespace MoonGlare::Core::Scene {
#if 0
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
                .addProperty("scene", &SceneStateChangeEvent::GetSceneName, &SceneStateChangeEvent::SetSceneName)
                .addProperty("state", &SceneStateChangeEvent::GetSceneState, &SceneStateChangeEvent::SetSceneState)

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

/*@ [PublicLuaEvents/_] `SetSceneEvent` @*/
/*@ [EventsReference/SetSceneEvent] `SetSceneEvent` Event
    Send this event to change current scene. If scene cannot be executed immediately
    (i.e. some resources are still loading) the loading scene will show.
    Unique scene name must be unique. It may be equal to scene type name
    in cases when multiple instances are not needed. @*/
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
            << "[scene:" << dt.sceneName.c_str()
            << ",typeName:" << dt.sceneTypeName.c_str()
            << ",suspend:" << (int)dt.suspendCurrent
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
            .beginClass<SetSceneEvent>("SetSceneEvent")
/* @ [SetSceneEvent/_] SetSceneEvent constructor
    `SetSceneEvent([uniqueSceneName[, suspend]])` @*/
                .addConstructor<void(*)(const char*, bool)>()

/* @ [SetSceneEvent/_] `SetSceneEvent.scene`
    Unique name of scene @*/
                .addProperty("scene", &SetSceneEvent::GetSceneName, &SetSceneEvent::SetSceneName)
/* @ [SetSceneEvent/_] `SetSceneEvent.typeName`
    Scene type name @*/
                .addProperty("typeName", &SetSceneEvent::GetSceneTypeName, &SetSceneEvent::SetSceneTypeName)
/* @ [SetSceneEvent/_] `SetSceneEvent.suspend`
    Boolean. Does current scene should be suspended or deleted@*/
                .addData("suspend", &SetSceneEvent::suspendCurrent, true)

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

/* @ [PublicLuaEvents/_] `SetSceneChangeFenceEvent` @*/
/* @ [EventsReference/SetSceneChangeFenceEvent] `SetSceneChangeFenceEvent` Event
    This event allows to set fence to prohibit scene execution. All fences must be cleared to execute scene. @*/
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
            << "[fence:" << dt.fence.c_str()
            << ",sceneName:" << dt.sceneName.c_str()
            << ",active:" << dt.active
            << "]";
        return out;
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
        .beginClass<SetSceneChangeFenceEvent>("SetSceneChangeFenceEvent")
/* @ [PublicLuaEvents/SetSceneEvent] SetSceneEvent constructor
    `SetSceneEvent([uniqueSceneName[, suspend]])` @*/
            .addConstructor<void(*)(const char*, bool)>()

/* @ [SetSceneChangeFenceEvent/_] `SetSceneChangeFenceEvent.fence`
    Name of fence @*/
            .addProperty("fence", &SetSceneChangeFenceEvent::GetFence, &SetSceneChangeFenceEvent::SetFence)
/* @ [SetSceneChangeFenceEvent/_] `SetSceneChangeFenceEvent.active`
    Whether set or remove fence @*/
            .addData("active", &SetSceneChangeFenceEvent::active, true)
/* @ [SetSceneChangeFenceEvent/_] `SetSceneChangeFenceEvent.scene`
    Unique name of affected scene. If empty defaults to pending scene @*/
            .addProperty("scene", &SetSceneChangeFenceEvent::GetSceneName, &SetSceneChangeFenceEvent::SetSceneName)

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
#endif
} //namespace MoonGlare::Core::Scene
