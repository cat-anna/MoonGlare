#ifndef ENGINE_H
#define ENGINE_H

namespace MoonGlare {
namespace Core {

class Engine : public cRootClass {
    SPACERTTI_DECLARE_CLASS_SINGLETON(Engine, cRootClass)
    DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
    Engine(World *World);
    ~Engine();

    void Initialize();
    void Finalize();
    void PostSystemInit();

    void EngineMain();

    /** Add action which has to be called between engine steps */
    template<class T> void PushSynchronizedAction(T &&t) { m_ActionQueue.Add(t); }

    /** Abort engine execution. Throws exception. No cleanup is done. */
    void Abort();
    /** Proper way to exit. Graceful engine exit. Engine smoothly finishes execution. */
    void Exit();

    static void ScriptApi(ApiInitializer &root);

    void SetFrameRate(float value);
    unsigned GetFrameRate() const { return m_LastFPS; }

    static string GetVersionString();

    World* GetWorld() { return m_World; }

protected:
    Space::ActionQueue m_ActionQueue;

    World *m_World = nullptr;
    Renderer::RendererFacade *m_Renderer = nullptr;

    std::unique_ptr<Graphic::Dereferred::DereferredPipeline> m_Dereferred;

    volatile bool m_Running;		//!< Indicates whether engine is running
    float m_FrameTimeSlice;			//!< Amount of ms per single frame. Equals to 16.(6) for 60PFSd
    unsigned m_LastFPS;				//!< FPS in previous second.
    unsigned m_SkippedFrames;		//!< Total amount of skipped frames.
};

inline Engine* GetEngine() { return Engine::Instance(); }

} //namespace Core
} //namespace MoonGlare

#endif // ENGINE_H
