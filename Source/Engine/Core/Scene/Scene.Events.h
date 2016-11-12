#pragma once
#ifndef SCENEEVENTS_H_
#define SCENEEVENTS_H_

namespace MoonGlare::Core::Scene {

enum class SceneState {
	Invalid,
	Created,
	Started,
	Paused,
};

struct SceneStateChangeEvent {
	SceneState m_State;

	friend std::ostream& operator<<(std::ostream& out, const SceneStateChangeEvent const & dt) {
		out << "SceneStateChangeEvent["
			<< "State:" << (int)dt.m_State
			<< "]";
		return out;
	}
};

} //namespace MoonGlare::Core::Scene

#endif // CSCENE_H_
