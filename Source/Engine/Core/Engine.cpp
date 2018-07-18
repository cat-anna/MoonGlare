#include <pch.h>
#include <MoonGlare.h>

#include "Engine.h"
#include "iConsole.h"
#include "iSoundEngine.h"
#include "Scene/ScenesManager.h"

#include <Renderer/Dereferred/DereferredPipeline.h>

#include <Renderer/Frame.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Context.h>
#include <Core/Scripts/ScriptEngine.h>


using namespace std::chrono_literals;

namespace MoonGlare {
namespace Core {

const char *VersionString = "0.1.0 build 512";
const char *ApplicationName = "MoonGlare engine";
const char *CompilationDate = __DATE__ " at " __TIME__;

//----------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Engine);

Engine::Engine(World *world) :
        cRootClass(),
        m_Running(false),

        m_LastFPS(0),
        m_SkippedFrames(0),
        m_FrameTimeSlice(1.0f),

        m_Dereferred(),

        m_World(world)
{
    MoonGlareAssert(m_World);
    
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
    m_Dereferred = std::make_unique<Graphic::Dereferred::DereferredPipeline>();
    m_Dereferred->Initialize(GetWorld());

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
    GetSoundEngine()->ScanForSounds();

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
    auto Ctx = m_Renderer->GetContextImpl();

    MoveConfig &conf = stepData;
    conf.deferredSink = m_Dereferred->GetDefferedSink();
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
    bool odd = true;

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
        odd = odd || conf.m_SecondPeriod;

        m_ActionQueue.DispatchPendingActions();

        ++FrameCounter;
        conf.m_BufferFrame = Device->NextFrame();

        auto &cmdl = conf.m_BufferFrame->GetCommandLayers();
        using Layer = Renderer::Frame::CommandLayers::LayerEnum;

        auto StartTime = clock::now();
        {
            conf.deferredSink->Reset(conf);
            conf.TimeDelta = tdiff(LastMoveTime, CurrentTime);
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
            if (odd) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glFlush();
                cmdl.Execute();
                glFlush();
                frame->GetFirstWindowLayer().Execute();
                Ctx->Flush();

            }
            else {
                frame->GetCommandLayers().Execute<Layer::Controll, Layer::PreRender>();
            }

            Device->ReleaseFrame(frame);

            //glFlush();
            //glFinish();
            //glFlush();
        }

        auto RenderTime = clock::now();
        {
            //glFinish();
            //Ctx->Flush();
        }
        auto EndTime = clock::now();
        LastMoveTime = CurrentTime;

        odd = !odd;

        if(conf.m_SecondPeriod) {
            TitleRefresh = CurrentTime;
            m_LastFPS = FrameCounter;
            FrameCounter = 0;
            double sum = tdiff(StartTime, EndTime);
            //if (Config::Current::EnableFlags::ShowTitleBarDebugInfo) {
                char Buffer[256];
                sprintf(Buffer, "time:%.2fs  fps:%u  frame:%llu  skipped:%u  mt:%.1f st:%.2f rti:%.1f swp:%.1f sum:%.1f fill:%.1f",
                        tdiff(BeginTime, CurrentTime), m_LastFPS, Device->FrameCounter(), m_SkippedFrames,
                        tdiff(StartTime, MoveTime) * 1000.0f,
                        tdiff(MoveTime, SortTime) * 1000.0f,
                        tdiff(SortTime, RenderTime) * 1000.0f,
                        tdiff(RenderTime, EndTime) * 1000.0f,
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
