#include "engine_runner/engine_runner.hpp"
#include "core/threaded_async_loader.hpp"
#include "engine_runner/script_init_runner_hook.hpp"
#include "lua_context/lua_script_context.hpp"
#include "lua_context/modules/lua_modules_all.hpp"
#include "lua_modules/core_lua_modules.hpp"
#include "scene_manager/default_scene_factory.hpp"
#include "scene_manager/prefab_manager.hpp"
#include "scene_manager/scenes_manager.hpp"
#include "systems/register_components.hpp"
#include "systems/register_systems.hpp"
#include <orbit_logger.h>
#include <stdexcept>

namespace MoonGlare::Runner {

EngineRunner::EngineRunner() {
    Systems::RegisterAllSystems(ecs_register);
    Systems::RegisterAllComponents(ecs_register);

    LuaModules::LoadAllCoreLuaModules(&runner_hooks);
    Lua::LoadAllLuaModules(&runner_hooks);
}

EngineRunner::~EngineRunner() {
    Teardown();
}

void EngineRunner::Execute() {
    do_soft_restart = false;
    Initialize();

    runner_hooks.AssertAllInterfaceHooksExecuted();

    engine_thread = std::thread([this]() {
        ::OrbitLogger::ThreadInfo::SetName("CORE");
        engine_core->EngineMain();
    });

    rendering_device->EnterLoop();

    Stop();

    if (engine_thread.joinable()) {
        engine_thread.join();
    }
}

void EngineRunner::Initialize() {
    ecs_register.Dump();
    auto config = LoadConfiguration();

    script_context = std::make_shared<Lua::LuaScriptContext>();
    runner_hooks.InterfaceReady<Lua::iScriptModuleManager>(script_context->GetModuleManager());
    runner_hooks.InterfaceReady<Lua::iScriptContext>(script_context.get());
    runner_hooks.InterfaceReady<Lua::iCodeChunkRunner>(script_context->GetCodeRunnerInterface());

    // auto moveCfg = std::make_shared<Core::MoveConfig>();
    // m_World->SetSharedInterface(moveCfg);
    // m_World->SetInterface<Component::SubsystemUpdateData>(moveCfg.get());
    // auto ModManager = new ModulesManager(m_World.get());
    // if (!PreSystemInit()) {
    //     AddLogf(Error, "Pre system init action failed!");
    //     throw "Pre system init action failed";
    // }

    filesystem = CreateFilesystem();
    async_loader = std::make_shared<ThreadedAsyncLoader>(filesystem);

    runner_hooks.InterfaceReady<iReadOnlyFileSystem>(filesystem.get());
    runner_hooks.InterfaceReady<iEngineRunner>(this);
    // runner_hooks.InterfaceReady<...>(async_loader.get());

    LoadDataModules();

    engine_core = std::make_unique<EngineCore>(script_context.get());

    using namespace Renderer;
    device_context = CreateDeviceContext();

    input_processor = std::make_unique<InputHandler::InputProcessor>(this);
    // runner_hooks.InterfaceReady<...>(input_processor.get());

    auto main_window = Renderer::WindowCreationInfo{config.window};
    device_window = device_context->CreateWindow(main_window, input_processor.get());

    resource_manager =
        iResourceManager::CreteDefaultResourceManager(async_loader.get(), filesystem.get());
    rendering_device = std::make_shared<RenderingDevice>(
        engine_core.get(), device_context.get(), device_window.get(), resource_manager.get());

    resource_manager->SetResourceLoader(rendering_device.get());
    engine_core->SetRenderingDevice(rendering_device.get());

    InitSceneManager();

    runner_hooks.AfterDataModulesLoad();

    // m_World->CreateObject<Resources::StringTables>();
    // if (!ModManager->Initialize()) {
    //     AddLogf(Error, "Unable to initialize modules manager!");
    //     throw "Unable to initialize modules manager";
    // }
    // SoundSystem::Component::SoundSystemRegister::Install(*m_World);

    // m_Renderer = Renderer::iRendererFacade::CreateInstance(*m_World);
    // auto *R = (Renderer::RendererFacade *)m_Renderer.get();
    // R->GetScriptApi()->Install(scrEngine->GetLua());
    // m_Renderer->Initialize(GetDisplaySettings(), GetFileSystem());
    // m_World->SetRendererFacade(R);
    // m_World->CreateObject<Resources::SkeletalAnimationManager>();
    // auto *dataMgr = new DataManager(m_World.get());
    // m_World->SetInterface(dataMgr);

    runner_hooks.InterfaceReady<iEngineTime>(engine_core.get());

    // LuaModules::LoadAllCoreLuaModules(script_module_manager.get(), engine_core.get());

    // if (!m_World->Initialize()) {
    //     AddLogf(Error, "Failed to initialize world!");
    //     throw "Failed to initialize world!";
    // }
    // dataMgr->InitFonts();

    AddLog(Info, "Engine initialized");

    // if (!PostSystemInit()) {
    //     AddLogf(Error, "Post system init action failed!");
    //     throw "Post system init action failed!";
    // }
}

void EngineRunner::Teardown() {
    if (engine_thread.joinable()) {
        engine_thread.join();
    }

    runner_hooks.Clear();
    engine_core.reset();

    scene_manager.reset();
    prefab_manager.reset();

    rendering_device.reset();
    resource_manager.reset();
    device_window.reset();
    device_context.reset();

    async_loader.reset();
    filesystem.reset();
    svfs_hooks.reset();
    script_context.reset();

    AddLog(Info, "Engine finalized");
}

void EngineRunner::Stop() {
    AddLog(Debug, "Stop called");
    if (engine_core) {
        engine_core->Stop();
    }
    if (rendering_device) {
        rendering_device->Stop();
    }
}

//---------------------------------------------------------------------------------------

StarVfs::iStarVfsHooks *EngineRunner::IntSvfsHooks() {
    if (!svfs_hooks) {
        auto h = std::make_unique<Runner::ScriptInitRunnerHook>(
            script_context->GetCodeRunnerInterface());
        runner_hooks.InstallHook(h.get());
        svfs_hooks = std::move(h);
    }
    return svfs_hooks.get();
}

void EngineRunner::InitSceneManager() {
    using namespace SceneManager;
    prefab_manager = std::make_unique<PrefabManager>(filesystem.get(), &ecs_register, &ecs_register,
                                                     resource_manager.get());

    runner_hooks.InterfaceReady<iPrefabManager>(prefab_manager.get());

    auto sm = std::make_unique<ScenesManager>(
        filesystem.get(), //
        std::make_unique<DefaultSceneFactory>(async_loader.get(), &ecs_register,
                                              prefab_manager.get(), rendering_device.get()));

    runner_hooks.InterfaceReady<iScenesManager>(sm.get());
    engine_core->SetSceneManager(sm.get());

    scene_manager = std::move(sm);
}

} // namespace MoonGlare::Runner

#if 0

namespace MoonGlare {

//---------------------------------------------------------------------------------------

bool Application::PostSystemInit() {
    GetModulesManager()->OnPostInit();
    m_World->PostSystemInit();
    m_Renderer->GetContext()->HookMouse();
    return true;
}

//---------------------------------------------------------------------------------------

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


void Application::Restart() {
    m_Flags.m_Restart = true;
    AddLogf(Debug, "Starting restart");
    MoonGlare::Core::GetEngine()->Exit();
}

} //namespace MoonGlare

#endif