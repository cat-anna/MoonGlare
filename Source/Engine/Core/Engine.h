#pragma once

namespace MoonGlare {
namespace Core {

class Engine : public cRootClass {
    SPACERTTI_DECLARE_CLASS_SINGLETON(Engine, cRootClass)
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

    void SetFrameRate(float value);
    unsigned GetFrameRate() const { return m_LastFPS; }

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
