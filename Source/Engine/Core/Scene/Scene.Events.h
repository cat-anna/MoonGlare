#pragma once
#ifndef SCENEEVENTS_H_
#define SCENEEVENTS_H_

namespace MoonGlare::Core::Scene {

enum class SceneState : unsigned {
	Invalid,
	Created,
	Started,
	Paused,
};

struct SceneStateChangeEvent {
	static constexpr char* EventName = "OnSceneStateChangeEvent";
	static constexpr char* HandlerName = "SceneStateChange";
	SceneState m_State;
	ciScene *m_Scene;

	friend std::ostream& operator<<(std::ostream& out, const SceneStateChangeEvent & dt) {
		out << "SceneStateChangeEvent"
			<< "[State:" << (int) dt.m_State
			<< ";Scene:" << dt.m_Scene
			<< "]";
		return out;
	}

	static ApiInitializer RegisterLuaApi(ApiInitializer api) {
		return api
			.beginClass<SceneStateChangeEvent>("cSceneStateChangeEvent")
				.addData("State", (int SceneStateChangeEvent::*)&SceneStateChangeEvent::m_State, false)
				.addData("Scene", &SceneStateChangeEvent::m_Scene, false)
				.addStaticData("Name", EventName, false)
			.endClass();
	}
};

} //namespace MoonGlare::Core::Scene

#endif // CSCENE_H_
