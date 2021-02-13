#pragma once

#include "engine_time.hpp"
#include <atomic>
#include <stepable_interface.hpp>
#include <stop_interface.hpp>

namespace MoonGlare {

class EngineCore : public iStopInterface, public iEngineTime {
public:
    EngineCore(iStepableObject *_scene_manager);
    ~EngineCore() = default;

    void EngineMain();

    // template <class T>
    // void PushSynchronizedAction(T&& t)
    // {
    //     m_ActionQueue.Add(t);
    // }

    // void SetFrameRate(float value);
    // unsigned GetFrameRate() const { return m_LastFPS; }

    void Stop() override;

    //iEngineTime
    double GetGlobalTime() const override { return global_time; }
    double GetCurrentTimeDelta() const override { return current_frame_time; }
    size_t GetFrameCounter() const override { return frame_counter; }
    clock_t::time_point GetStartTime() const override { return start_time_point; };

protected:
    iStepableObject *const scene_manager;
    // Memory::ActionQueue m_ActionQueue;

    // World *m_World = nullptr;
    // Renderer::RendererFacade *m_Renderer = nullptr;

    std::atomic<bool> can_work{false};

    clock_t::time_point start_time_point;

    double current_frame_time = 0;
    double global_time = 0;
    size_t frame_counter = 0;
};

} // namespace MoonGlare
