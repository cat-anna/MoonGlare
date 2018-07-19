/*
 * ciScene.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */
#ifndef CISCENE_H_
#define CISCENE_H_

#include "../Component/nComponent.h"
#include "Scene.Events.h"

namespace MoonGlare::Core::Scene {

struct SceneDescriptor;

class ciScene {
public:
	ciScene(const ciScene&) = delete;
	ciScene();
	~ciScene();

	Component::ComponentManager& GetComponentManager() { return m_ComponentManager; }
	SceneDescriptor* GetSceneDescriptor() const { return m_Descriptor; }

	bool Initialize(pugi::xml_node Node, std::string Name, Entity OwnerEntity, SceneDescriptor *Descriptor);
	bool Finalize();

//old
	void BeginScene();
	void EndScene();

	void DoMove(const MoveConfig &conf);

	static void RegisterScriptApi(::ApiInitializer &api);

protected:
	Component::ComponentManager m_ComponentManager;
	SceneDescriptor *m_Descriptor = nullptr;

    void SendState(SceneState state);

	bool SpawnChild(const std::string &URI, std::string Name, Entity &out);
	bool SpawnChildRaw(const char *URI, const char *Name);

};

} //namespace MoonGlare::Core::Scene

#endif // CSCENE_H_
