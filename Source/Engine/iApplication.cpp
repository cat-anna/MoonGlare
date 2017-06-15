/*
  * Generated by cppsrc.sh
  * On 2015-06-29 10:15:07.37
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <boost/filesystem.hpp>

#include <MoonGlare.h>

#include <Engine/Core/DataManager.h>
#include <Engine/Core/Console.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/InputProcessor.h>
#include <Engine/World.h>

#include <Assets/AssetManager.h>
#include <Renderer/Renderer.h>
#include <Renderer/Context.h>
#include <Renderer/ScriptApi.h>

#include <iApplication.h>

#include <AssetSettings.x2c.h>
#include <RendererSettings.x2c.h>
#include <EngineSettings.x2c.h>

namespace MoonGlare {
namespace Application {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(iApplication);

iApplication::iApplication() : BaseClass() {
    m_Flags.m_UintValue = 0;

    m_Configuration = std::make_unique<x2c::Settings::EngineSettings_t>();
    m_ConfigurationFileName = "MoonGlare.Settings.xml";

    SetThisAsInstance();
}

iApplication::~iApplication() {}

//---------------------------------------------------------------------------------------

bool iApplication::PreSystemInit() {
    return true;
}

bool iApplication::PostSystemInit() {
    Core::GetEngine()->PostSystemInit();
    m_Renderer->GetContext()->HookMouse();
    return true;
}

//---------------------------------------------------------------------------------------

void iApplication::LoadSettings() {
    if (m_Flags.m_Initialized) {
        throw "Cannot load settings after initialization!";
    }
    m_Configuration->ResetToDefault();

    if (!boost::filesystem::is_regular_file(m_ConfigurationFileName.data())) {
        m_Flags.m_SettingsLoaded = true;
        m_Flags.m_SettingsChanged = true;
        AddLogf(Warning, "Settings not loaded. File is not valid");
        return;
    }

    m_Configuration->ReadFile(m_ConfigurationFileName.data());

    m_Flags.m_SettingsLoaded = true;
    m_Flags.m_SettingsChanged = false;
}

void iApplication::SaveSettings() {
    if (!m_Flags.m_SettingsLoaded) {
        AddLogf(Error, "Settings not loaded!");
        return;
    }
    if (!m_Flags.m_SettingsChanged) {
        AddLogf(Warning, "Settings not saved. There is no changes");
        return;
    }

    m_Configuration->WriteFile(m_ConfigurationFileName.data());

    m_Flags.m_SettingsChanged = false;
}

//---------------------------------------------------------------------------------------

using Modules::ModulesManager;
using MoonGlare::Core::Console;
using FileSystem::MoonGlareFileSystem;
using MoonGlare::Core::Scripts::ScriptEngine;
using DataManager = MoonGlare::Core::Data::Manager;

void iApplication::Initialize() {
    LoadSettings();

    m_World = std::make_unique<World>();

    if (!PreSystemInit()) {
        AddLogf(Error, "Pre system init action failed!");
        throw "Pre system init action failed";
    }
#define _init_chk(WHAT, ERRSTR, ...) \
do { if(!(WHAT)->Initialize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); throw ERRSTR; } } while(false)

    _init_chk(new MoonGlareFileSystem(), "Unable to initialize internal filesystem!");

    static x2c::Settings::AssetSettings_t as;
    m_AssetManager = std::make_unique<Asset::AssetManager>();
    if (!m_AssetManager->Initialize(&as)) {
        AddLogf(Error, "Unable to initialize asset manager!");
        throw "Unable to initialize asset manager!";
    }

    auto ModManager = new ModulesManager();
    ::Settings->Load();
    if (!ModManager->Initialize()) {
        AddLogf(Error, "Unable to initialize modules manager!");
        throw "Unable to initialize modules manager";
    }

    auto scrEngine = new ScriptEngine(m_World.get());
    _init_chk(scrEngine, "Unable to initialize script engine!");
    
    m_Renderer = std::make_unique<Renderer::RendererFacade>();
    m_World->SetRendererFacade(m_Renderer.get());

    m_Renderer->GetScriptApi()->Install(scrEngine->GetLua());
    m_Renderer->SetConfiguration(&m_Configuration->m_Renderer);
    if (!m_Renderer->Initialize(m_Configuration->m_Display, m_AssetManager.get())) {
        AddLogf(Error, "Unable to initialize renderer");
        throw "Unable to initialize renderer";
    }

    auto window = std::make_unique<Graphic::Window>(m_Renderer->GetContext()->GetHandle());
    auto Device = new Graphic::cRenderDevice(std::move(window));

    if (!(new DataManager())->Initialize(ScriptEngine::Instance())) {
        AddLogf(Error, "Unable to initialize data manager!");
        throw "Unable to initialize data manager";
    }

    auto Engine = new MoonGlare::Core::Engine(m_World.get());

    if (m_Configuration->m_Core.m_EnableConsole) {
        auto c = new Console();
        _init_chk(c, "Unable to initialize console!");
        m_World->SetConsole(c);
    }

    Engine->Initialize();

    //Temporary solution which probably will be used for eternity
    auto Input = Engine->GetWorld()->GetInputProcessor();
    auto rctx = m_Renderer->GetContext();
    rctx->SetInputHandler(Input);
    Input->SetInputSource(rctx);

    AddLog(Debug, "Application initialized");
#undef _init_chk

    m_Renderer->SetStopObserver([Engine]() { Engine->Exit(); });

    if (!PostSystemInit()) {
        AddLogf(Error, "Post system init action failed!");
        throw "Post system init action failed!";
    }

    m_Flags.m_Initialized = true;
}

void iApplication::Execute() {
    try {
        Initialize();
        MoonGlare::Core::GetEngine()->EngineMain();
    }
    catch (...) {
        Finalize();
        throw;
    }
    Finalize();
}

void iApplication::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::InstanceExists()) break; if(!WHAT::Instance()->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

    Settings->Save();

    _del_chk(Console, "Console finalization failed");

    MoonGlare::Core::Engine::Instance()->Finalize();
    _finit_chk(DataManager, "Data Manager finalization failed");
    _finit_chk(Graphic::cRenderDevice, "Render device finalization failed");

    Graphic::cRenderDevice::DeleteInstance();

    if (m_Renderer && !m_Renderer->Finalize()) 
        AddLogf(Error, "Unable to finalize renderer");
    m_Renderer.reset();

    _finit_chk(ModulesManager, "Finalization of modules manager failed!");

    ModulesManager::DeleteInstance();
    MoonGlare::Core::Engine::DeleteInstance();
    DataManager::DeleteInstance();

    _del_chk(ScriptEngine, "Finalization of script engine failed!");
    if(!m_AssetManager->Finalize()) {
         AddLogf(Error, "AssetManager finalization failed");
    }
    m_AssetManager.reset();
    _del_chk(FileSystem::MoonGlareFileSystem, "Finalization of filesystem failed!");

    m_World.reset();

    AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
    SaveSettings();
    m_Flags.m_Initialized = false;
}

//---------------------------------------------------------------------------------------

void iApplication::OnActivate() {
    m_Flags.m_Active = true;
    AddLogf(Debug, "Application activated");
}

void iApplication::OnDeactivate() {
    m_Flags.m_Active = false;
    AddLogf(Debug, "Application deactivated");
}

void iApplication::Exit() {
    AddLogf(Debug, "Exit called");
    MoonGlare::Core::GetEngine()->Exit();
}

const char* iApplication::ExeName() const {
    return "";
}

//----------------------------------------------------------------------------------

void iApplication::RegisterScriptApi(ApiInitializer &root) {
    root
    .deriveClass<ThisClass, BaseClass>("ciApplication")
        .addFunction("Exit", &iApplication::Exit)
        .addProperty("DoRestart", &iApplication::DoRestart, &iApplication::SetRestart)
    .endClass()
    ;
}

RegisterApiDerivedClass(iApplication, &iApplication::RegisterScriptApi);
RegisterApiInstance(iApplication, &iApplication::Instance, "Application");

} //namespace Application
} //namespace MoonGlare
