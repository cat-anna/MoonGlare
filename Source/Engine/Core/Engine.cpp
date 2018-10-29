#include <pch.h>
#include <nfMoonGlare.h>

#include <Foundation/TimeUtils.h>

#include "Engine.h"
#include "iConsole.h"

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Renderer/Frame.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/RenderDevice.h>
#include <Core/Scripts/ScriptEngine.h>

using namespace std::chrono_literals;

namespace MoonGlare {
namespace Core {

const char *VersionString = "0.2.1 build 768";
const char *ApplicationName = "MoonGlare engine";
const char *CompilationDate = __DATE__ " at " __TIME__;

//----------------------------------------------------------------------------------

Engine* Engine::s_instance = nullptr;

Engine::Engine(World *world) :
        m_Dereferred(),
        m_World(world)
{
    assert(m_World);
    
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Performance, "PERF");

    m_Renderer = m_World->GetRendererFacade();

    s_instance = this;
}

Engine::~Engine() { }

//----------------------------------------------------------------------------------

void Engine::Initialize() {
    m_Dereferred = std::make_unique<Renderer::Deferred::DefferedFrontend>();
    m_Dereferred->Initialize(GetWorld()->GetRendererFacade());

    SetFrameRate(static_cast<float>(m_Renderer->GetContext()->GetRefreshRate()));
}

void Engine::Finalize() {
    m_Dereferred.reset();

    AddLog(Performance, "Frames skipped: " << m_SkippedFrames);

    if (!m_World->Finalize()) {
        AddLogf(Error, "Failed to finalize world!");
        throw "Failed to finalize world!";
    }
}

//----------------------------------------------------------------------------------

void Engine::Exit() {
    m_Running = false;
    m_Renderer->Stop();
}

void Engine::EngineMain() {
    m_Running = true; 

    auto Device = m_Renderer->GetDevice();
    auto Ctx = m_Renderer->GetContext();

    MoveConfig &conf = *m_World->GetInterface<MoveConfig>();
    conf.deffered = m_Dereferred.get();
    conf.m_ScreenSize = Ctx->GetSizef();

    using clock = std::chrono::steady_clock;

    unsigned FrameCounter = 0;
    clock::time_point EntryTime = clock::now();
    clock::time_point LastFrame = EntryTime;
    clock::time_point BeginTime = EntryTime;
    clock::time_point CurrentTime = EntryTime;
    clock::time_point LastMoveTime = EntryTime;
    clock::time_point TitleRefresh = EntryTime;

    conf.m_SecondPeriod = false;
    conf.ResetTime(EntryTime);

    while (m_Running) {
        CurrentTime = clock::now();

        double FrameTimeDelta = TimeDiff(LastFrame, CurrentTime);

        if (FrameTimeDelta + 0.001f < m_FrameTimeSlice) {
            auto remain = m_FrameTimeSlice - FrameTimeDelta;
            std::this_thread::sleep_for(1ms * (remain * 0.9f));//arbitrary
            continue;
        }

        if (FrameTimeDelta < m_FrameTimeSlice) {
            continue;
        }

        //if (FrameTimeDelta >= m_FrameTimeSlice * 1.5f)
            //++m_SkippedFrames;

        conf.m_SecondPeriod = TimeDiff(TitleRefresh, CurrentTime) >= 1.0;

        m_ActionQueue.DispatchPendingActions();

        conf.m_BufferFrame = Device->NextFrame();

        if (!conf.m_BufferFrame)
            continue;

        ++FrameCounter;
        LastFrame = CurrentTime;

        //auto &cmdl = conf.m_BufferFrame->GetCommandLayers();
        using Layer = Renderer::Frame::CommandLayers::LayerEnum;

        auto StartTime = clock::now();
        {
            conf.deffered->Reset(conf.m_BufferFrame);
            conf.UpdateTime(CurrentTime);

            GetScriptEngine()->Step(conf);
            GetWorld()->Step(conf);
            auto console = m_World->GetConsole();
            if (console)
                console->ProcessConsole(conf);
        }
        auto MoveTime = clock::now();

        Device->Submit(conf.m_BufferFrame);

        auto EndTime = clock::now();
        LastMoveTime = CurrentTime;

        if (conf.m_SecondPeriod) {
            TitleRefresh = CurrentTime;
            m_LastFPS = FrameCounter;
            FrameCounter = 0;
            double sum = TimeDiff(StartTime, EndTime);
            char Buffer[256];
            sprintf(Buffer, "time:%.2fs  fps:%u  frame:%llu  skipped:%u  mt:%.1f rti:%.1f sum:%.1f fill:%.1f",
                TimeDiff(BeginTime, CurrentTime), m_LastFPS, Device->FrameCounter(), m_SkippedFrames,
                TimeDiff(StartTime, MoveTime) * 1000.0f,
                TimeDiff(MoveTime, EndTime) * 1000.0f,
                (sum) * 1000.0f,
                (sum / m_FrameTimeSlice) * 100.0f
            );
            AddLogf(Performance, Buffer);
        }
    }
}

//----------------------------------------------------------------------------------

void Engine::SetFrameRate(float value) {
    if (value < 1.0f)
        value = 1.0f;
    m_FrameTimeSlice = 1.0f / value;
    AddLogf(Debug, "Frame rate limit set to %d (%.5f ms per frame)", (unsigned)value, m_FrameTimeSlice * 1000.0f);
}

//----------------------------------------------------------------------------------

} //namespace Core
} //namespace MoonGlare
