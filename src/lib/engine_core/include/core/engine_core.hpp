#pragma once

#include "core/engine_time.hpp"
#include "core/stepable_interface.hpp"
#include "core/stop_interface.hpp"
#include "lua_context/script_module.hpp"
#include "renderer/rendering_device_interface.hpp"
#include <atomic>

namespace MoonGlare {

class EngineCore : public iStopInterface, public iEngineTime {
public:
    EngineCore(Lua::iScriptContext *_lua_context);
    ~EngineCore() = default;

    void EngineMain();

    void SetRenderingDevice(Renderer::iRenderingDevice *_rendering_device);
    void SetSceneManager(iStepableObject *_scene_manager);

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
    Lua::iScriptContext *const lua_context;
    iStepableObject *scene_manager;
    Renderer::iRenderingDevice *rendering_device;
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
