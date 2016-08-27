#ifndef ENGINE_H
#define ENGINE_H

namespace MoonGlare {
namespace Core {

const Version::Info& GetMoonGlareEngineVersion();

class Engine : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(Engine, cRootClass)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
    Engine();
    ~Engine();

	struct Flags {
		enum {
			Ready		= 0x0001,
		};
	};

	bool Initialize();
	void EngineMain();
	bool Finalize();

    void DoMove(MoveConfig &conf);
    void DoRender(MoveConfig &conf);

	/** Proxy call to SceneManager instance. Should be used only by scripts. */
	void SetNextScene(const string& Name) const;
	/** Proxy call to SceneManager instance. Should be used only by scripts. */
	void ClearSceneStack();
	/** Proxy call to SceneManager instance. Should be used only by scripts. */
	void PopScenes(int count) const;
	/** Proxy call to SceneManager instance. Should be used only by scripts. */
	void ClearScenesUntil(const string& Name) const;

	/** Queue change scene action. */
	void ChangeScene() { m_ActionQueue.Add([this]() { ChangeSceneImpl(); }); }

	//void HandleSceneStateChange() { m_ActionQueue.Add([this]() { HandleSceneStateChangeImpl(); }); }
	/** pushes new scene onto stack, switches to it, and returns it */
	//ciScene* PushScene(const string& Name);

	/** Add action which has to be called between engine steps */
	template<class T> void PushSynchronizedAction(T &&t) { m_ActionQueue.Add(t); }

	DefineREADAccesPTR(CurrentScene, ciScene);

	/** Abort engine execution. Throws exception. No cleanup is done. */
	void Abort();			
	/** Proper way to exit. Graceful engine exit. Engine smoothly finishes execution. */
	void Exit();				

	void HandleEscapeKey() { m_ActionQueue.Add([this]() { HandleEscapeKeyImpl(); }); }
	void CaptureScreenShot();

	int SetProxyTimer(EventProxyPtr proxy, float secs, int TimerID, bool cyclic) { return m_TimeEvents.SetTimer(TimerID, secs, cyclic, proxy); }
	void KillProxyTimer(EventProxyPtr proxy, int TimerID) { m_TimeEvents.KillTimer(TimerID, proxy); }

	static void ScriptApi(ApiInitializer &root);
	static void RegisterDebugScriptApi(ApiInitializer &root);

	void SetFrameRate(float value);
	unsigned GetFrameRate() const { return m_LastFPS; }

	/** Execute new thread watched by engine */
	std::thread StartThread(std::function<void()> ThreadMain);

	static string GetVersionString();

	World* GetWorld() { return m_World.get(); }

	DefineFlagGetter(m_Flags, Flags::Ready, Ready);
protected:
	unsigned m_Flags;
	ciScene* m_CurrentScene;
	Space::ActionQueue m_ActionQueue;
	TimeEvents m_TimeEvents;

	std::unique_ptr<World> m_World;

	std::unique_ptr<Graphic::Dereferred::DereferredPipeline> m_Dereferred;
	std::unique_ptr<Graphic::Forward::ForwardPipeline> m_Forward;

	volatile bool m_Running;		//!< Indicates whether engine is running
	float m_FrameTimeSlice;			//!< Amount of ms per single frame. Equals to 16.(6) for 60PFSd
	unsigned m_LastFPS;				//!< FPS in previous second.
	unsigned m_FrameCounter;		//!< Counter of fps in current second. Updated per cycle.
	unsigned m_SkippedFrames;		//!< Total amount of skipped frames.

	bool BeginGame();
	bool EndGame();

	DefineFlagSetter(m_Flags, Flags::Ready, Ready);

	virtual void HandleEscapeKeyImpl();
	void CaptureScreenShotImpl();

	//void GoToSceneImpl(const string& Name);
	//void HandleSceneStateChangeImpl();
	void ChangeSceneImpl();
}; 

inline Engine* GetEngine() { return Engine::Instance(); }

} //namespace Core
} //namespace MoonGlare 

#endif // ENGINE_H
