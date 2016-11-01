#ifndef ENGINE_H
#define ENGINE_H

namespace MoonGlare {
namespace Core {

class Engine : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(Engine, cRootClass)
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
    Engine();
    ~Engine();

	bool Initialize();
	bool Finalize();
	bool PostSystemInit();

	void EngineMain();

    void DoMove(MoveConfig &conf);
    void DoRender(MoveConfig &conf);

	//void HandleSceneStateChange() { m_ActionQueue.Add([this]() { HandleSceneStateChangeImpl(); }); }
	/** pushes new scene onto stack, switches to it, and returns it */
	//ciScene* PushScene(const string& Name);

	/** Add action which has to be called between engine steps */
	template<class T> void PushSynchronizedAction(T &&t) { m_ActionQueue.Add(t); }

	/** Abort engine execution. Throws exception. No cleanup is done. */
	void Abort();			
	/** Proper way to exit. Graceful engine exit. Engine smoothly finishes execution. */
	void Exit();				

	static void ScriptApi(ApiInitializer &root);
	static void RegisterDebugScriptApi(ApiInitializer &root);

	void SetFrameRate(float value);
	unsigned GetFrameRate() const { return m_LastFPS; }

	/** Execute new thread watched by engine */
	std::thread StartThread(std::function<void()> ThreadMain);

	static string GetVersionString();

	World* GetWorld() { return m_World.get(); }

protected:
	unsigned m_Flags;
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

	void ChangeSceneImpl();
}; 

inline Engine* GetEngine() { return Engine::Instance(); }

} //namespace Core
} //namespace MoonGlare 

#endif // ENGINE_H
