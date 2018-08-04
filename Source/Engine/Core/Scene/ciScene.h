/*
 * ciScene.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */
#ifndef CISCENE_H_
#define CISCENE_H_

#include "../Component/SubsystemManager.h"
#include "Scene.Events.h"

namespace MoonGlare::Core::Scene {

struct SceneDescriptor;

class ciScene {
public:
	ciScene();
	~ciScene();

	Component::SubsystemManager& GetSubsystemManager() { return m_SubsystemManager; }

	bool Initialize(pugi::xml_node Node, std::string Name, Entity OwnerEntity);
	bool Finalize();

//old
	void BeginScene();
	void EndScene();

	void DoMove(const MoveConfig &conf);
protected:
	Component::SubsystemManager m_SubsystemManager;
	SceneDescriptor *m_Descriptor = nullptr;

    void SendState(SceneState state);

	bool SpawnChild(const std::string &URI, std::string Name, Entity &out);
};

} //namespace MoonGlare::Core::Scene

#endif // CSCENE_H_
