#include "engine_runner/engine_runner.hpp"
#include "core/threaded_async_loader.hpp"
#include "engine_runner/script_init_runner_hook.hpp"
#include "lua_context/lua_script_context.hpp"
#include "lua_context/modules/lua_modules_all.hpp"
#include "lua_modules/core_lua_modules.hpp"
#include "scene_manager/prefab_manager.hpp"
#include "scene_manager/scenes_manager.hpp"
#include "systems/register_components.hpp"
#include "systems/register_systems.hpp"
#include <orbit_logger.h>
#include <renderer/resource_manager.hpp>
#include <stdexcept>

namespace MoonGlare {

EngineRunner::EngineRunner() {
    Systems::RegisterAllSystems(ecs_register);
    Systems::RegisterAllComponents(ecs_register);

    LuaModules::LoadAllCoreLuaModules(&runner_hooks);
    Lua::LoadAllLuaModules(&runner_hooks);
}

void EngineRunner::Execute() {
    do_soft_restart = false;
    try {
        Initialize();

        runner_hooks.AssertAllInterfaceHooksExecuted();

        engine_thread = std::thread([this]() {
            ::OrbitLogger::ThreadInfo::SetName("CORE");
            engine_core->EngineMain();
        });

        rendering_device->EnterLoop();

    } catch (...) {
        Finalize();
        throw;
    }
    Finalize();
}

void EngineRunner::Initialize() {
    ecs_register.Dump();
    auto config = LoadConfiguration();

    script_module_manager = std::make_shared<Lua::LuaScriptContext>();
    runner_hooks.InterfaceReady<Lua::iScriptModuleManager>(script_module_manager.get());

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

    using namespace SceneManager;
    prefab_manager = std::make_unique<PrefabManager>(filesystem.get(), &ecs_register, &ecs_register);
    runner_hooks.InterfaceReady<iPrefabManager>(prefab_manager.get());

    scene_manager =
        std::make_unique<ScenesManager>(filesystem.get(), async_loader.get(), &ecs_register, prefab_manager.get());
    runner_hooks.InterfaceReady<iScenesManager>(scene_manager.get());

    runner_hooks.AfterDataModulesLoad();

    // m_World->CreateObject<Resources::StringTables>();
    // if (!ModManager->Initialize()) {
    //     AddLogf(Error, "Unable to initialize modules manager!");
    //     throw "Unable to initialize modules manager";
    // }
    // SoundSystem::Component::SoundSystemRegister::Install(*m_World);
    // auto scrEngine = new ScriptEngine(m_World.get());

    device_context = CreateDeviceContext();
    rendering_device = std::make_shared<Renderer::RenderingDevice>(device_context);
    resource_manager = std::make_shared<Renderer::ResourceManager>(async_loader, rendering_device);

    input_processor = std::make_unique<InputHandler::InputProcessor>(this);

    auto main_window = Renderer::WindowCreationInfo{config.window};
    device_window = device_context->CreateWindow(main_window, input_processor.get());

    // m_Renderer = Renderer::iRendererFacade::CreateInstance(*m_World);
    // auto *R = (Renderer::RendererFacade *)m_Renderer.get();
    // R->GetScriptApi()->Install(scrEngine->GetLua());
    // m_Renderer->Initialize(GetDisplaySettings(), GetFileSystem());
    // m_World->SetRendererFacade(R);
    // m_World->CreateObject<Resources::SkeletalAnimationManager>();
    // auto *dataMgr = new DataManager(m_World.get());
    // m_World->SetInterface(dataMgr);
    // LoadDataModules();

    engine_core = std::make_unique<EngineCore>(dynamic_cast<iStepableObject *>(scene_manager.get())); //TODO: ugly
    runner_hooks.InterfaceReady<iEngineTime>(engine_core.get());

    // LuaModules::LoadAllCoreLuaModules(script_module_manager.get(), engine_core.get());

    // if (!m_World->Initialize()) {
    //     AddLogf(Error, "Failed to initialize world!");
    //     throw "Failed to initialize world!";
    // }
    // dataMgr->InitFonts();

    AddLog(Debug, "Application initialized");

    // R->SetStopObserver([Engine]() { Engine->Exit(); });
    // if (!PostSystemInit()) {
    //     AddLogf(Error, "Post system init action failed!");
    //     throw "Post system init action failed!";
    // }
}

void EngineRunner::Finalize() {
    if (engine_thread.joinable()) {
        engine_thread.join();
    }

    // #define _finit_chk(WHAT, ERRSTR, ...)                                                                                  \
//     do {                                                                                                               \
//         if (!WHAT::s_instance)                                                                                         \
//             break;                                                                                                     \
//         if (!WHAT::s_instance->Finalize()) {                                                                           \
//             AddLogf(Error, ERRSTR, __VA_ARGS__);                                                                       \
//         }                                                                                                              \
//     } while (false)
    // #define _del_chk(WHAT, ERRSTR, ...)                                                                                    \
//     do {                                                                                                               \
//         _finit_chk(WHAT, ERRSTR, __VA_ARGS__);                                                                         \
//         WHAT::DeleteInstance();                                                                                        \
//     } while (false)

    //     SaveSettings();
    //     if (m_Renderer)
    //         m_Renderer->Finalize();
    //     m_Renderer.reset();
    //     _finit_chk(ModulesManager, "Finalization of modules manager failed!");
    //     ModulesManager::DeleteInstance();

    runner_hooks.Clear();
    engine_core.reset();

    // ScriptEngine::DeleteInstance();
    // m_World.reset();

    rendering_device.reset();
    resource_manager.reset();
    device_window.reset();
    device_context.reset();

    scene_manager.reset();
    prefab_manager.reset();
    async_loader.reset();
    filesystem.reset();
    svfs_hooks.reset();
    script_module_manager.reset();

    AddLog(Debug, "Application finalized");
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
        auto h = std::make_unique<Runner::ScriptInitRunnerHook>(script_module_manager->GetCodeRunnerInterface());
        runner_hooks.InstallHook(h.get());
        svfs_hooks = std::move(h);
    }
    return svfs_hooks.get();
}

} // namespace MoonGlare

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