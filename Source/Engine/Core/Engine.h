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

    template<class T> void PushSynchronizedAction(T &&t) { m_ActionQueue.Add(t); }

    void Abort();
    void Exit();

    static void ScriptApi(ApiInitializer &root);

    void SetFrameRate(float value);
    unsigned GetFrameRate() const { return m_LastFPS; }

    static string GetVersionString();

    World* GetWorld() { return m_World; }

    const MoveConfig& StepData() const { return stepData; }
protected:
    Space::ActionQueue m_ActionQueue;

    World *m_World = nullptr;
    Renderer::RendererFacade *m_Renderer = nullptr;

    std::unique_ptr<Graphic::Dereferred::DereferredPipeline> m_Dereferred;

    volatile bool m_Running;	
    float m_FrameTimeSlice;		
    unsigned m_LastFPS;			
    unsigned m_SkippedFrames;	

    MoveConfig stepData;
};

inline Engine* GetEngine() { return Engine::Instance(); }

} //namespace Core
} //namespace MoonGlare

#endif // ENGINE_H
