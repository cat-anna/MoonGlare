#include <pch.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <OrbitLogger/src/sink/FileSink.h>
#include <OrbitLogger/src/sink/MSVCDebuggerSink.h>

#include <nfMoonGlare.h>

#include <Engine/Core/DataManager.h>
#include <Engine/Modules/ModuleManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/InputProcessor.h>                   
#include <Engine/World.h>
#include "Core/Scene/ScenesManager.h"

#include <Renderer/Renderer.h>
#include <Renderer/ScriptApi.h>
#include <Source/Renderer/RenderDevice.h>
#include <Foundation/OS/Path.h>
#include <Core/Scripts/ScriptEngine.h>

#include "Modules/BasicConsole.h"

#include "Application.h"

#include <RendererSettings.x2c.h>

#include <Foundation/SoundSystem/Component/SoundSystemRegister.h>
#include <Foundation/Component/EventDispatcher.h>

#include <Foundation/Resources/SkeletalAnimationManager.h>
#include <Foundation/Resources/StringTables.h>


namespace MoonGlare {

Application::Application() {
    m_Flags.m_UintValue = 0;
}

Application::~Application() {}

//---------------------------------------------------------------------------------------

bool Application::PreSystemInit() {
    return true;
}

bool Application::PostSystemInit() {
    GetModulesManager()->OnPostInit();
    m_World->PostSystemInit();
    m_Renderer->GetContext()->HookMouse();
    return true;
}

//---------------------------------------------------------------------------------------

void Application::LoadSettings() {
    settings = std::make_shared<Settings>(GetUpperLayerSettings());
    m_World->SetSharedInterface(settings);

    std::string settingsFileName = SettingsPath() + "Engine.cfg";
    settings->LoadFromFile(settingsFileName);
}

void Application::SaveSettings() {
    std::string settingsFileName = SettingsPath() + "Engine.cfg";

    auto stt = m_World->GetSharedInterface<Settings>();
    if (stt->Changed())
        stt->SaveToFile(settingsFileName);
}

Renderer::ContextCreationInfo Application::GetDisplaySettings() {
    auto stt = m_World->GetSharedInterface<Settings>();
    assert(stt);

    Renderer::ContextCreationInfo cci;

    stt->Get("Display.FullScreen", cci.fullScreen);
    stt->Get("Display.Monitor", cci.monitor);
    stt->Get("Display.Width", cci.width);
    stt->Get("Display.Height", cci.height);

    cci.visible = false;

    return cci;
}

//---------------------------------------------------------------------------------------

void Application::InitLogger() {
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    using OrbitLogger::LogCollector;
    using OrbitLogger::StdFileLoggerSink;
    using OrbitLogger::MSVCDebuggerSink;
    namespace LogChannels = OrbitLogger::LogChannels;

    std::string logPath = settings->GetString("Log.Path", "logs") + "/";
    boost::filesystem::create_directory(logPath);

    if (DEBUG_TRUE || settings->HasValue("Log.Output"))
        LogCollector::AddLogSink<StdFileLoggerSink>(logPath + settings->GetString("Log.Output", "Engine.log"));

    if (DEBUG_TRUE || settings->HasValue("Log.SessionOutput"))
        LogCollector::AddLogSink<StdFileLoggerSink>(logPath + settings->GetString("Log.SessionOutput", "Engine.session.log"), false);

#ifdef DEBUG_LOG
    bool dbgLog = settings->GetBool("Log.Debug", DEBUG_TRUE);
    if(dbgLog)
        LogCollector::AddLogSink<MSVCDebuggerSink>();
    LogCollector::SetChannelState(LogChannels::Debug, dbgLog);
#endif

    LogCollector::SetCaptureStdOut(OrbitLogger::LogChannels::StdOut);
    LogCollector::SetCaptureStdErr(OrbitLogger::LogChannels::StdErr);
    LogCollector::SetChannelName(OrbitLogger::LogChannels::StdOut, "SOUT");
    LogCollector::SetChannelName(OrbitLogger::LogChannels::StdErr, "SERR");
}   

//---------------------------------------------------------------------------------------

using Modules::ModulesManager;
using FileSystem::MoonGlareFileSystem;  
using MoonGlare::Core::Scripts::ScriptEngine;
using DataManager = MoonGlare::Core::Data::Manager;

void Application::Initialize() {
    m_World = std::make_unique<World>();
    m_World->SetInterface(this);

    m_World->CreateObject<Modules::BasicConsole, iConsole>();

    LoadSettings();
    InitLogger();

    //TODO:
    auto moveCfg = std::make_shared<Core::MoveConfig>();
    m_World->SetSharedInterface(moveCfg);
    m_World->SetInterface<Component::SubsystemUpdateData>(moveCfg.get());

    auto ModManager = new ModulesManager(m_World.get());

    if (!PreSystemInit()) {
        AddLogf(Error, "Pre system init action failed!");
        throw "Pre system init action failed";
    }

    m_World->CreateObject<MoonGlareFileSystem, iFileSystem>();
    m_World->CreateObject<Resources::StringTables>();

    if (!ModManager->Initialize()) {
        AddLogf(Error, "Unable to initialize modules manager!");
        throw "Unable to initialize modules manager";
    }        

    SoundSystem::Component::SoundSystemRegister::Install(*m_World);

    auto scrEngine = new ScriptEngine(m_World.get());
    
    m_Renderer = Renderer::iRendererFacade::CreateInstance(*m_World);
    auto *R = (Renderer::RendererFacade*)m_Renderer.get();
    R->GetScriptApi()->Install(scrEngine->GetLua());
    m_Renderer->Initialize(GetDisplaySettings(), GetFileSystem());
    m_World->SetRendererFacade(R);

    m_World->CreateObject<Resources::SkeletalAnimationManager>();

    auto *dataMgr = new DataManager(m_World.get());
    m_World->SetInterface(dataMgr);

    LoadDataModules();

#ifdef DEBUG_DUMP
    {
        std::ofstream fsvfs("logs/vfs.txt");
        GetFileSystem()->DumpStructure(fsvfs);
    }
#endif

    auto Engine = new MoonGlare::Core::Engine(m_World.get());

    if (!m_World->Initialize()) {
        AddLogf(Error, "Failed to initialize world!");
        throw "Failed to initialize world!";
    }               

    dataMgr->InitFonts();


    Engine->Initialize();

    //Temporary solution which probably will be used for eternity
    auto Input = Engine->GetWorld()->GetInputProcessor();
    auto rctx = m_Renderer->GetContext();
    rctx->SetInputHandler(Input);
    Input->SetInputSource(rctx);

    AddLog(Debug, "Application initialized");
#undef _init_chk

    R->SetStopObserver([Engine]() { Engine->Exit(); });

    if (!PostSystemInit()) {
        AddLogf(Error, "Post system init action failed!");
        throw "Post system init action failed!";
    }

#ifdef DEBUG_DUMP
    m_World->DumpObjects();
#endif
}

void Application::LoadDataModules() {
    static const std::string moduleListFileName = OS::GetSettingsDirectory() + "ModuleList.txt";
    std::ifstream file(moduleListFileName, std::ios::in);
                   
    while (!file.eof()) {
        std::string mod;
        std::getline(file, mod);
        
        boost::trim(mod);
                    
        auto comment = mod.find('#');
        if (comment != std::string::npos) {
            mod = mod.substr(0, comment);
            boost::trim(mod);
        }
        if (mod.empty())
            continue;

        AddLogf(Debug, "Trying to load container '%s'", mod.c_str());
        if (!GetFileSystem()->LoadContainer(mod)) {
            AddLogf(Error, "Unable to open container: '%s'", mod.c_str());
        }
    }
}                       

void Application::Execute() {
    try {
        Initialize();
        if (!m_World->PreSystemStart()) {
            AddLogf(Error, "Failure during PreSystemStart");
            return;
        }

        WaitForFirstScene();
        m_Renderer->GetContext()->SetVisible(true);

        auto engineThread = std::thread([this]() {
            ::OrbitLogger::ThreadInfo::SetName("CORE");
            MoonGlare::Core::GetEngine()->EngineMain();
        });

        ::OrbitLogger::ThreadInfo::SetName("REND");
        auto *R = (Renderer::RendererFacade*)m_Renderer.get();
        R->EnterLoop();

        engineThread.join();

        if (!m_World->PreSystemShutdown()) {
            AddLogf(Error, "Failure during PreSystemShutdown");
            return;
        }
    }
    catch (...) {
        Finalize();
        throw;
    }
    Finalize();
}

void Application::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::s_instance) break; if(!WHAT::s_instance->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

    SaveSettings();

    MoonGlare::Core::Engine::s_instance->Finalize();

    if(m_Renderer)
        m_Renderer->Finalize();
    m_Renderer.reset();

    _finit_chk(ModulesManager, "Finalization of modules manager failed!");

    ModulesManager::DeleteInstance();
    MoonGlare::Core::Engine::DeleteInstance();

    ScriptEngine::DeleteInstance();

    m_World.reset();

    AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
}

void Application::WaitForFirstScene() {
    auto *R = (Renderer::RendererFacade*)m_Renderer.get();
    auto *Device = R->GetDevice();
    auto *Ctx = m_Renderer->GetContext();
    auto *sm = m_World->GetScenesManager();
    auto *ed = m_World->GetInterface<Component::EventDispatcher>();

    DebugLog(Debug, "Engine initialized. Waiting for scene to be ready.");
    while (sm->IsScenePending()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ed->Step();
        Device->ProcessPendingCtrlQueues();
        Ctx->Process();
    }
    DebugLog(Debug, "Scene became ready. Starting main loop.");
}

//---------------------------------------------------------------------------------------

void Application::OnActivate() {
    m_Flags.m_Active = true;
    AddLogf(Debug, "Application activated");
}

void Application::OnDeactivate() {
    m_Flags.m_Active = false;
    AddLogf(Debug, "Application deactivated");
}

void Application::Exit() {
    AddLogf(Debug, "Exit called");
    MoonGlare::Core::GetEngine()->Exit();
}

void Application::Restart() {
    m_Flags.m_Restart = true;
    AddLogf(Debug, "Starting restart");
    MoonGlare::Core::GetEngine()->Exit();
}

//---------------------------------------------------------------------------------------

std::string Application::SettingsPath() const {
    auto str = settings->GetString("Settings.Path", OS::GetSettingsDirectory());
    if (str.back() != '/' || str.back() != '\\')
        str += "/";
    return std::move(str);
}

} //namespace MoonGlare
