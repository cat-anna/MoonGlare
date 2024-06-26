﻿
#include "core/engine_core.hpp"
#include <chrono>
#include <fmt/format.h>
#include <orbit_logger.h>

// #include "iConsole.h"
// #include <Core/Scripts/ScriptEngine.h>
// #include <Foundation/TimeUtils.h>
// #include <Renderer/Deferred/DeferredFrontend.h>
// #include <Renderer/Frame.h>
// #include <Source/Renderer/RenderDevice.h>
// #include <Source/Renderer/Renderer.h>

using namespace std::chrono_literals;

namespace MoonGlare {

namespace {

template <typename TIMEPOINT>
auto TimeDiff(const TIMEPOINT &start, const TIMEPOINT &end) {
    return std::chrono::duration<float>(end - start).count();
}

} // namespace

//----------------------------------------------------------------------------------

EngineCore::EngineCore(Lua::iScriptContext *_lua_context) : lua_context(_lua_context) {
}

void EngineCore::SetRenderingDevice(Renderer::iRenderingDevice *_rendering_device) {
    rendering_device = _rendering_device;
}

void EngineCore::SetSceneManager(iStepableObject *_scene_manager) {
    scene_manager = _scene_manager;
}

//----------------------------------------------------------------------------------

void EngineCore::Stop() {
    can_work = false;
}

void EngineCore::EngineMain() {
    assert(rendering_device);
    assert(scene_manager);

    can_work = true;
    double last_report_time = 0;
    size_t last_frame_counter = 0;

    clock_t::time_point entry_time = clock_t::now();
    clock_t::time_point last_frame = entry_time;

    start_time_point = entry_time;
    AddLog(Debug, "Starting main engine loop");

    while (can_work.load()) {
        auto frame_start_time = clock_t::now();
        double frame_time = TimeDiff(last_frame, frame_start_time);
        current_frame_time = frame_time;
        global_time = TimeDiff(entry_time, frame_start_time);

        // m_ActionQueue.DispatchPendingActions();
        // conf.m_BufferFrame = Device->NextFrame();
        // if (!conf.m_BufferFrame)
        //     continue;

        ++frame_counter;

        rendering_device->NextFrame();

        scene_manager->DoStep(frame_time);
        lua_context->Step(frame_time);

        rendering_device->SubmitFrame();

        // auto StartTime = clock_t::now();
        // {
        //     conf.UpdateTime(CurrentTime);
        //     GetScriptEngine()->Step(conf);
        //     GetWorld()->Step(conf);
        //     console->ProcessConsole(conf);
        // }
        // auto MoveTime = clock_t::now();

        // auto EndTime = clock_t::now();
        // LastMoveTime = CurrentTime;

        // AddData(1, (float)FrameTimeDelta);

        last_frame = frame_start_time;

        if (global_time - last_report_time > 1.0) {
            last_report_time = global_time;
            double dt = global_time - last_report_time;
            auto avg_dt = (dt / static_cast<float>(frame_counter - last_frame_counter)) * 1000.0f;
            AddLog(Performance, fmt::format("time:{:05.3f} frames:{:06} avg_frame_dt:{:.3f}ms",
                                            global_time, frame_counter, avg_dt));
        }
    }
}

//----------------------------------------------------------------------------------

} //namespace MoonGlare
