/*
 * ciScene.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */
#ifndef CISCENE_H_
#define CISCENE_H_

#include "../Component/nComponent.h"

namespace MoonGlare {
namespace Core {
namespace Scene {

enum class SceneState {
	Unknown,
	Active,
	Waiting,
	Finished,
};

enum class SceneEscapeAction {
	None,
	ExitEngine,
	EnterPreviousScene,
	EnterSystemMenu,

	ExtendedAction,
};

struct SceneSettings {
	string PlayList;

	SceneSettings();

	bool LoadMeta(const xml_node node);
};

DECLARE_SCRIPT_EVENT_VECTOR(SceneScriptEvents, iScriptEvents,
		SCRIPT_EVENT_ADD(
			(OnTimer)
			(OnBeginScene)(OnEndScene)
			(OnInitialize)(OnFinalize)
			(OnEscape)
		),
		SCRIPT_EVENT_REMOVE());

class ciScene : public DataClasses::BasicResource {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(ciScene, DataClasses::BasicResource)
	DECLARE_SCRIPT_HANDLERS_ROOT(SceneScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DECLARE_EVENT_HOLDER();
public:
	ciScene(const ciScene&) = delete;
	ciScene();
	virtual ~ciScene();

	Component::ComponentManager& GetComponentManager() { return m_ComponentManager; }

//old
	DefineRefSetGet(Settings, SceneSettings);
	void SetSceneState(SceneState state);
	DefineREADAcces(SceneState, SceneState);
	Core::Camera::iCameraPtr& GetCamera() { return m_Camera; }

	/** Script code invokers  */
	virtual int InvokeOnTimer(int TimerID);
	virtual int InvokeOnEscape();
	virtual int InvokeOnBeginScene();
	virtual int InvokeOnEndScene();
	virtual int InvokeOnInitialize();
	virtual int InvokeOnFinalize();

	/** @brief Call this function to initialize scene before first call */
	virtual void BeginScene();
	/** @brief Call this function when scene won't be used for some time */
	virtual void EndScene();
	/** @brief Initialize scene */
	bool Initialize();
	/** @brief Finalize scene */
	bool Finalize();

	bool SetMetaData(FileSystem::XMLFile &file);

	virtual void DoMove(const MoveConfig &conf);

	int SetTimer(float secs, int TimerID, bool cyclic) { return SetProxyTimer(GetEventProxy(), secs, TimerID, cyclic); }
	void KillTimer(int TimerID) { return KillProxyTimer(GetEventProxy(), TimerID); }
	int SetProxyTimer(EventProxyPtr proxy, float secs, int TimerID, bool cyclic) { return m_TimeEvents.SetTimer(TimerID, secs, cyclic, proxy); }
	void KillProxyTimer(EventProxyPtr proxy, int TimerID) { m_TimeEvents.KillTimer(TimerID, proxy); }

	static void RegisterScriptApi(::ApiInitializer &api);

	const GUI::GUIEnginePtr& GetGUI() const { return m_GUI; }
//very old
	DefineFlagGetter(m_Flags, sfset_IsReady, Ready)
	DefineFlagGetter(m_Flags, sf_Initialized, Initialized)
	enum eSceneFlags {
		sf_Initialized = 0x0001,
		sf_Ready = 0x0002,

		sfset_IsReady = sf_Ready | sf_Initialized,
	};
protected:
	Component::ComponentManager m_ComponentManager;
//old

	TimeEvents m_TimeEvents;
	Core::Camera::iCameraPtr m_Camera;
	GUI::GUIEnginePtr m_GUI;
	XMLFile m_MetaData;

	virtual bool DoInitialize();
	virtual bool DoFinalize();
	virtual bool LoadMeta(const xml_node Node);

	const xml_node GetRootNode() const;

	void FinishScene();
	void SetFinishedState();

	virtual int InternalEventNotification(Events::InternalEvents event, int Param);
//very old
	unsigned m_Flags;
	DefineFlagSetter(m_Flags, sf_Initialized, Initialized);
	DefineFlagSetter(m_Flags, sf_Ready, Ready);
private:
//old
	SceneState m_SceneState;
	SceneSettings m_Settings;
};

} //namespace Scene
} //namespace Core
} //namespace MoonGlare

#endif // CSCENE_H_
