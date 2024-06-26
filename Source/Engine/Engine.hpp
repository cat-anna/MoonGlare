#pragma once

#include <Memory/ActionQueue.h>
#include <Foundation/Tools/PerfViewClient.h>

namespace MoonGlare::Core {

class Engine : private Tools::PerfView::PerfProducer {
public:
    static Engine* s_instance;
    static void DeleteInstance()
    {
        delete s_instance;
        s_instance = nullptr;
    }

    Engine(World* World);
    ~Engine();

    void Initialize();
    void Finalize();

    void EngineMain();

    template <class T>
    void PushSynchronizedAction(T&& t)
    {
        m_ActionQueue.Add(t);
    }

    void Exit();

    void SetFrameRate(float value);
    unsigned GetFrameRate() const { return m_LastFPS; }

    World* GetWorld() { return m_World; }

protected:
    Memory::ActionQueue m_ActionQueue;

    World* m_World = nullptr;
    Renderer::RendererFacade* m_Renderer = nullptr;

    std::unique_ptr<Renderer::Deferred::DefferedFrontend> m_Dereferred;

    bool m_Running = false;
    float m_FrameTimeSlice = 1.0f;
    unsigned m_LastFPS = 0;
    unsigned m_SkippedFrames = 0;
};

inline Engine* GetEngine() { return Engine::s_instance; }

} // namespace MoonGlare::Core
