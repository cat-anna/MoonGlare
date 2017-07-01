#include <pch.h>
#include <MoonGlare.h>

#include "Engine.h"
#include "iConsole.h"

#include <Renderer/Dereferred/DereferredPipeline.h>

#include <Renderer/Frame.h>

#include <Source/Renderer/Renderer.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Context.h>

namespace MoonGlare {
namespace Core {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Engine);
RegisterApiDerivedClass(Engine, &Engine::ScriptApi);
RegisterApiInstance(Engine, &Engine::Instance, "Engine");

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
    if (!m_World->Initialize()) {
        AddLogf(Error, "Failed to initialize world!");
        throw "Failed to initialize world!";
    }

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
    GetDataMgr()->LoadGlobalData();

    if (!m_World->PostSystemInit()) {
        AddLogf(Error, "World PostSystemInit failed!");
        throw "World PostSystemInit failed!";
    }
}

//----------------------------------------------------------------------------------

void Engine::ScriptApi(ApiInitializer &root){
    root
    .deriveClass<ThisClass, BaseClass>("cEngine")
        .addFunction("GetFrameRate", &ThisClass::GetFrameRate)
        .addFunction("GetInfoString", Utils::Template::InstancedStaticCall<ThisClass, string>::get<&ThisClass::GetVersionString>())

//		.addFunction("CaptureScreenShot", &ThisClass::CaptureScreenShot)

#ifdef DEBUG_SCRIPTAPI
        .addFunction("SetFrameRate", &ThisClass::SetFrameRate)
#endif
    .endClass()
    ;
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

    MoveConfig conf;
    conf.deferredSink = m_Dereferred->GetDefferedSink();
    conf.m_ScreenSize = Ctx->GetSizef();

    using clock = std::chrono::steady_clock;
    auto tdiff = [](clock::time_point t1, clock::time_point t2) {
        return std::chrono::duration<float>(t2 - t1).count();
    };

    unsigned FrameCounter = 0;
    clock::time_point LastFrame = clock::now();
    clock::time_point BeginTime = LastFrame;
    clock::time_point CurrentTime = LastFrame;
    clock::time_point LastMoveTime = LastFrame;
    clock::time_point TitleRefresh = LastFrame;

    while (m_Running) {
        auto CurrentTime = clock::now();
        float FrameTimeDelta = tdiff(LastFrame, CurrentTime);
        if (FrameTimeDelta < m_FrameTimeSlice) 
            continue;
        if (FrameTimeDelta >= m_FrameTimeSlice * 1.5f) 
            ++m_SkippedFrames;

        LastFrame = CurrentTime;

        m_ActionQueue.DispatchPendingActions();

        ++FrameCounter;
        conf.m_BufferFrame = Device->NextFrame();

        auto &cmdl = conf.m_BufferFrame->GetCommandLayers();
        using Layer = Renderer::Frame::CommandLayers::LayerEnum;

        auto StartTime = clock::now();
        {
            conf.deferredSink->Reset(conf);
            conf.TimeDelta = tdiff(LastMoveTime, CurrentTime);
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
            auto frame = Device->PendingFrame();
            using Layer = Renderer::Frame::CommandLayers::LayerEnum;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Device->ProcessPendingCtrlQueues();

            cmdl.Execute();
            frame->GetFirstWindowLayer().Execute();

            Device->ReleaseFrame(frame);
        }

        auto RenderTime = clock::now();
        {
            Ctx->Flush();
        }
        auto EndTime = clock::now();
        LastMoveTime = CurrentTime;

        conf.m_SecondPeriod = tdiff(TitleRefresh, CurrentTime) >= 1.0;
        if(conf.m_SecondPeriod) {
            TitleRefresh = CurrentTime;
            m_LastFPS = FrameCounter;
            FrameCounter = 0;
            float sum = tdiff(StartTime, EndTime);
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

string Engine::GetVersionString() {    
#ifdef DEBUG
    return ::MoonGlare::Core::GetMoonGlareEngineVersion().VersionStringFull();
#else
    return ::MoonGlare::Core::GetMoonGlareEngineVersion().VersionString();
#endif
}

const Version::Info& GetMoonGlareEngineVersion() {
    static const Version::Info MoonGlareEngineVersion{ 0, 1, 512, __DATE__ " at " __TIME__ };
    return MoonGlareEngineVersion;
}

} //namespace Core
} //namespace MoonGlare
