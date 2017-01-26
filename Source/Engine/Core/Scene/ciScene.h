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

class ciScene : public cRootClass {
	SPACERTTI_DECLARE_STATIC_CLASS(ciScene, cRootClass)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	ciScene(const ciScene&) = delete;
	ciScene();
	virtual ~ciScene();

	Component::ComponentManager& GetComponentManager() { return m_ComponentManager; }

	bool Initialize(pugi::xml_node Node, std::string Name, Entity OwnerEntity, SceneDescriptor *Descriptor);
	bool Finalize();

	Entity GetSceneEntity() const { return m_Entity; }
	SceneDescriptor* GetSceneDescriptor() const { return m_Descriptor; }

//old
	/** @brief Call this function to initialize scene before first call */
	void BeginScene();
	/** @brief Call this function when scene won't be used for some time */
	void EndScene();

	virtual void DoMove(const MoveConfig &conf);

	static void RegisterScriptApi(::ApiInitializer &api);

//very old
	DefineFlagGetter(m_Flags, sfset_IsReady, Ready)
	DefineFlagGetter(m_Flags, sf_Initialized, Initialized)
	enum eSceneFlags {
		sf_Initialized = 0x0001,
		sf_Ready = 0x0002,
		sfset_IsReady = sf_Ready | sf_Initialized,
	};
protected:
	Entity m_Entity;
	Component::ComponentManager m_ComponentManager;
	SceneDescriptor *m_Descriptor;

	void SendState(SceneState state) {
		m_ComponentManager.GetEventDispatcher().SendMessage<SceneStateChangeEvent>({ state, this });
	}

	bool SpawnChild(const std::string &URI, std::string Name, Entity &out);
	bool SpawnChildRaw(const char *URI, const char *Name);

//old
	Graphic::Environment m_Environment;

	void FinishScene();
	void SetFinishedState();

//very old
	unsigned m_Flags;
	DefineFlagSetter(m_Flags, sf_Initialized, Initialized);
	DefineFlagSetter(m_Flags, sf_Ready, Ready);
};

} //namespace MoonGlare::Core::Scene

#endif // CSCENE_H_
