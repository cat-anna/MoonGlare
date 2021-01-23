
#include "engine_core.hpp"
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

const char *VersionString = "0.3";
const char *ApplicationName = "MoonGlare engine";
const char *CompilationDate = __DATE__ " at " __TIME__;

template <typename TIMEPOINT>
auto TimeDiff(const TIMEPOINT &start, const TIMEPOINT &end) {
    return std::chrono::duration<float>(end - start).count();
}

} // namespace

//----------------------------------------------------------------------------------

// EngineCore::EngineCore(World *world) : PerfProducer(*world), m_Dereferred(), m_World(world) {
// assert(m_World);

// m_Renderer = m_World->GetRendererFacade();

// auto cid = AddChart("FrameStats");
// AddSeries("FrameTime", Unit::Miliseconds, cid);
// }

//----------------------------------------------------------------------------------

// void EngineCore::Initialize() {
// m_Dereferred = std::make_unique<Renderer::Deferred::DefferedFrontend>();
// m_Dereferred->Initialize(GetWorld()->GetRendererFacade());

// SetFrameRate(static_cast<float>(m_Renderer->GetContext()->GetRefreshRate()));
// }

// void EngineCore::Finalize() {
// m_Dereferred.reset();

// AddLog(Performance, "Frames skipped: " << m_SkippedFrames);

// if (!m_World->Finalize()) {
//     AddLogf(Error, "Failed to finalize world!");
//     throw "Failed to finalize world!";
// }
// }

//----------------------------------------------------------------------------------

void EngineCore::Stop() {
    can_work = false;
}

void EngineCore::EngineMain() {
    can_work = true;
    size_t last_frame_counter = 0;

    // auto Device = m_Renderer->GetDevice();
    // auto Ctx = m_Renderer->GetContext();

    // MoveConfig &conf = *m_World->GetInterface<MoveConfig>();
    // conf.deffered = m_Dereferred.get();
    // conf.m_ScreenSize = Ctx->GetSizef();

    using clock = std::chrono::steady_clock;

    clock::time_point entry_time = clock::now();
    clock::time_point last_one_second_tick = clock::now();
    // clock::time_point LastFrame = EntryTime;

    // clock::time_point BeginTime = EntryTime;
    // clock::time_point LastMoveTime = EntryTime;
    // clock::time_point TitleRefresh = EntryTime;

    // conf.m_SecondPeriod = false;
    // conf.ResetTime(EntryTime);

    // iConsole *console;
    // m_World->GetObject(console);

    AddLog(Debug, "Starting main engine loop");

    while (can_work.load()) {
        auto frame_start_time = clock::now();

        //     double FrameTimeDelta = TimeDiff(LastFrame, CurrentTime);

        //     if (FrameTimeDelta + 0.001f < m_FrameTimeSlice) {
        //         auto remain = m_FrameTimeSlice - FrameTimeDelta;
        //         std::this_thread::sleep_for(1ms * (remain * 0.9f)); //arbitrary
        //         continue;
        //     }

        //     if (FrameTimeDelta < m_FrameTimeSlice) {
        //         continue;
        //     }

        //     //if (FrameTimeDelta >= m_FrameTimeSlice * 1.5f)
        //     //++m_SkippedFrames;

        float each_second_interval = TimeDiff(last_one_second_tick, frame_start_time);
        bool each_one_second = each_second_interval >= 1.0;

        //     m_ActionQueue.DispatchPendingActions();

        //     conf.m_BufferFrame = Device->NextFrame();

        //     if (!conf.m_BufferFrame)
        //         continue;

        ++frame_counter;

        //     //auto &cmdl = conf.m_BufferFrame->GetCommandLayers();
        //     using Layer = Renderer::Frame::CommandLayers::LayerEnum;

        //     auto StartTime = clock::now();
        //     {
        //         conf.deffered->Reset(conf.m_BufferFrame);
        //         conf.UpdateTime(CurrentTime);

        //         GetScriptEngine()->Step(conf);
        //         GetWorld()->Step(conf);
        //         console->ProcessConsole(conf);
        //     }
        //     auto MoveTime = clock::now();

        //     Device->Submit(conf.m_BufferFrame);

        //     auto EndTime = clock::now();
        //     LastMoveTime = CurrentTime;

        //     AddData(1, (float)FrameTimeDelta);

        if (each_one_second) {
            auto current_time = clock::now();
            last_one_second_tick = frame_start_time;
            last_fps = frame_counter - last_frame_counter;
            double current_run_time = TimeDiff(entry_time, current_time);
            auto dt = (each_second_interval / static_cast<float>(last_fps)) * 1000.0f;
            auto info_line = fmt::format("time:{:05.1f} frame:{:06} fps:{:03} dt:{:.2f}ms", current_run_time,
                                         frame_counter, last_fps, dt);
            AddLog(Performance, info_line);
        }
    }
}

//----------------------------------------------------------------------------------

// void EngineCore::SetFrameRate(float value) {
//     if (value < 1.0f)
//         value = 1.0f;
//     m_FrameTimeSlice = 1.0f / value;
//     AddLogf(Debug, "Frame rate limit set to %d (%.5f ms per frame)", (unsigned)value, m_FrameTimeSlice * 1000.0f);
// }

//----------------------------------------------------------------------------------

} //namespace MoonGlare
