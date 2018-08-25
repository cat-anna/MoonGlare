#include <pch.h>
#include <nfMoonGlare.h>

#include "Engine.h"
#include "iConsole.h"
#include "Scene/ScenesManager.h"

#include <Renderer/Deferred/DeferredFrontend.h>

#include <Renderer/Frame.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/RenderDevice.h>
#include <Core/Scripts/ScriptEngine.h>

#include "JobQueue.h"

using namespace std::chrono_literals;

namespace MoonGlare {
namespace Core {

const char *VersionString = "0.2.0 build 512";
const char *ApplicationName = "MoonGlare engine";
const char *CompilationDate = __DATE__ " at " __TIME__;

//----------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Engine);

Engine::Engine(World *world) :
        cRootClass(),
        m_Dereferred(),
        m_World(world)
{
    assert(m_World);
    
    ::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Performance, "PERF");

    m_Renderer = m_World->GetRendererFacade();

    SetThisAsInstance();
    new JobQueue();
}

Engine::~Engine() {
    JobQueue::DeleteInstance();
}

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

void Engine::PostSystemInit() {
    if (!m_World->PostSystemInit()) {
        AddLogf(Error, "World PostSystemInit failed!");
        throw "World PostSystemInit failed!";
    }
}

//----------------------------------------------------------------------------------

void Engine::Exit() {
    m_Running = false;
}

void Engine::Abort() {
    m_Running = false;
    m_ActionQueue.Add([]{ throw __FUNCTION__ " called!"; });
    throw __FUNCTION__ " called!";
}

void Engine::EngineMain() {
    if (!m_World->PreSystemStart()) {
        AddLogf(Error, "Failure during PreSystemStart");
        return;
    }

    m_Running = true; 

    auto Device = m_Renderer->GetDevice();
    auto Ctx = m_Renderer->GetContext();

    MoveConfig &conf = stepData;
    conf.deffered = m_Dereferred.get();
    conf.m_ScreenSize = Ctx->GetSizef();

    using clock = std::chrono::steady_clock;
    auto tdiff = [](clock::time_point t1, clock::time_point t2) {
        return std::chrono::duration<double>(t2 - t1).count();
    };

    DebugLog(Debug, "Engine initialized. Waiting for scene to be ready.");
    while (!m_World->GetScenesManager()->CurrentScene()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_World->GetScenesManager()->ChangeScene(conf);
        Device->ProcessPendingCtrlQueues();
        Ctx->Process();
    }
    DebugLog(Debug, "Scene became ready. Starting main loop.");

    unsigned FrameCounter = 0;
    clock::time_point EntryTime = clock::now();
    clock::time_point LastFrame = EntryTime;
    clock::time_point BeginTime = EntryTime;
    clock::time_point CurrentTime = EntryTime;
    clock::time_point LastMoveTime = EntryTime;
    clock::time_point TitleRefresh = EntryTime;

    Ctx->SetVisible(true);

    while (m_Running) {
        CurrentTime = clock::now();

        double FrameTimeDelta = tdiff(LastFrame, CurrentTime);

    // if (FrameTimeDelta < m_FrameTimeSlice * 0.8f) 
        //continue;

        if (FrameTimeDelta + 0.001f < m_FrameTimeSlice) {
            auto remain = m_FrameTimeSlice - FrameTimeDelta;
            glFlush();
            std::this_thread::sleep_for(1ms * (remain * 0.9f));//arbitrary
            continue;
        }

        if (FrameTimeDelta < m_FrameTimeSlice) {
            continue;
        }

        if (FrameTimeDelta >= m_FrameTimeSlice * 1.5f)
            ++m_SkippedFrames;

        LastFrame = CurrentTime;
        conf.m_SecondPeriod = tdiff(TitleRefresh, CurrentTime) >= 1.0;

        m_ActionQueue.DispatchPendingActions();

        ++FrameCounter;
        conf.m_BufferFrame = Device->NextFrame();

        auto &cmdl = conf.m_BufferFrame->GetCommandLayers();
        using Layer = Renderer::Frame::CommandLayers::LayerEnum;

        auto StartTime = clock::now();
        {
            conf.deffered->Reset(conf.m_BufferFrame);
            conf.timeDelta = tdiff(LastMoveTime, CurrentTime);
            conf.globalTime = tdiff(EntryTime, CurrentTime);
            Ctx->Process();
            GetScriptEngine()->Step(conf);
            GetWorld()->Step(conf);
            auto console = m_World->GetConsole();
            if (console)
                console->ProcessConsole(conf);
        }
        auto MoveTime = clock::now();
        {
            cmdl.Get<Layer::GUI>().Sort();
        }
        auto SortTime = clock::now();
        Device->Submit(conf.m_BufferFrame);
        {
            //if (odd)

            auto frame = Device->PendingFrame();
            using Layer = Renderer::Frame::CommandLayers::LayerEnum;

            Device->ProcessPendingCtrlQueues();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glFlush();
            cmdl.Execute();
            glFlush();
            frame->GetFirstWindowLayer().Execute();
            Ctx->Flush();

            Device->ReleaseFrame(frame);
        }

        auto EndTime = clock::now();
        LastMoveTime = CurrentTime;

        if(conf.m_SecondPeriod) {
            TitleRefresh = CurrentTime;
            m_LastFPS = FrameCounter;
            FrameCounter = 0;
            double sum = tdiff(StartTime, EndTime);
            //if (Config::Current::EnableFlags::ShowTitleBarDebugInfo) {
                char Buffer[256];
                sprintf(Buffer, "time:%.2fs  fps:%u  frame:%llu  skipped:%u  mt:%.1f st:%.2f rti:%.1f sum:%.1f fill:%.1f",
                        tdiff(BeginTime, CurrentTime), m_LastFPS, Device->FrameCounter(), m_SkippedFrames,
                        tdiff(StartTime, MoveTime) * 1000.0f,
                        tdiff(MoveTime, SortTime) * 1000.0f,
                        tdiff(SortTime, EndTime) * 1000.0f,
                        (sum) * 1000.0f,
                        (sum / m_FrameTimeSlice) * 100.0f
                        );
                AddLogf(Performance, Buffer);
                Ctx->SetTitle(Buffer);
            //}
        }
    }

    if (!m_World->PreSystemShutdown()) {
        AddLogf(Error, "Failure during PreSystemShutdown");
        return;
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
