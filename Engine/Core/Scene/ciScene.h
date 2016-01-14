/*
 * ciScene.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */
#ifndef CISCENE_H_
#define CISCENE_H_
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
	GABI_DECLARE_ABSTRACT_CLASS(ciScene, DataClasses::BasicResource)
	DECLARE_SCRIPT_HANDLERS_ROOT(SceneScriptEvents);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	DECLARE_EVENT_HOLDER();
public:
	ciScene(const ciScene&) = delete;
	ciScene();
	virtual ~ciScene();

	DefineRefSetGet(Settings, SceneSettings);
	void SetSceneState(SceneState state);
	DefineREADAcces(SceneState, SceneState);
	::Core::Camera::iCameraPtr& GetCamera() { return m_Camera; }

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

	bool SetMetaData(FileSystem::XML &file);
	using BaseClass::SetDataModule;

	virtual void DoMove(const MoveConfig &conf);
	virtual void PreRender(const PreRenderConfig& conf);

	virtual void DoRender(cRenderDevice &Dev) const;
	virtual void DoRenderMeshes(cRenderDevice &Dev) const;
	virtual Graphic::Light::LightConfiguration* GetLightConfig();

	int SetTimer(float secs, int TimerID, bool cyclic) { return SetProxyTimer(GetEventProxy(), secs, TimerID, cyclic); }
	void KillTimer(int TimerID) { return KillProxyTimer(GetEventProxy(), TimerID); }
	int SetProxyTimer(::Core::EventProxyPtr proxy, float secs, int TimerID, bool cyclic) { return m_TimeEvents.SetTimer(TimerID, secs, cyclic, proxy); }
	void KillProxyTimer(::Core::EventProxyPtr proxy, int TimerID) { m_TimeEvents.KillTimer(TimerID, proxy); }

	static void RegisterScriptApi(::ApiInitializer &api);

	const GUI::GUIEnginePtr& GetGUI() const { return m_GUI; }
//old
	DefineFlagGetter(m_Flags, sfset_IsReady, Ready)
	DefineFlagGetter(m_Flags, sf_Initialized, Initialized)
	enum eSceneFlags {
		sf_Initialized = 0x0001,
		sf_Ready = 0x0002,

		sfset_IsReady = sf_Ready | sf_Initialized,
	};
protected:
	TimeEvents m_TimeEvents;
	::Core::Camera::iCameraPtr m_Camera;
	GUI::GUIEnginePtr m_GUI;
	XMLFile m_MetaData;

	virtual bool DoInitialize();
	virtual bool DoFinalize();
	virtual bool LoadMeta(const xml_node Node);

	const xml_node GetRootNode() const;

	void FinishScene();
	void SetFinishedState();

	virtual int InternalEventNotification(Events::InternalEvents event, int Param);
//old
	unsigned m_Flags;
	DefineFlagSetter(m_Flags, sf_Initialized, Initialized);
	DefineFlagSetter(m_Flags, sf_Ready, Ready);
private:
	SceneState m_SceneState;
	SceneSettings m_Settings;
};

} //namespace Scene
} //namespace Core

#endif // CSCENE_H_
