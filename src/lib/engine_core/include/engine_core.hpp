#pragma once

// #include <Memory/ActionQueue.h>
// #include <Foundation/Tools/PerfViewClient.h>

#include <atomic>
#include <stop_interface.hpp>

namespace MoonGlare {

class EngineCore : public iStopInterface
// : private Tools::PerfView::PerfProducer
{
public:
    // Engine(World* World);
    ~EngineCore() = default;

    void EngineMain();

    // template <class T>
    // void PushSynchronizedAction(T&& t)
    // {
    //     m_ActionQueue.Add(t);
    // }

    // void SetFrameRate(float value);
    // unsigned GetFrameRate() const { return m_LastFPS; }

    // World* GetWorld() { return m_World; }
    void Stop() override;

protected:
    // Memory::ActionQueue m_ActionQueue;

    // World *m_World = nullptr;
    // Renderer::RendererFacade *m_Renderer = nullptr;

    // std::unique_ptr<Renderer::Deferred::DefferedFrontend> m_Dereferred;

    std::atomic<bool> can_work{false};
    size_t frame_counter = 0;
    size_t last_fps = 0;

    // float m_FrameTimeSlice = 1.0f;
};

} // namespace MoonGlare
