#include "engine_runner.hpp"
#include "engine_runner/script_init_runner_hook.hpp"
#include "lua_context/lua_script_context.hpp"
#include "lua_context/modules/lua_modules_all.hpp"
#include "threaded_async_loader.hpp"
#include <orbit_logger.h>
#include <resources/resource_manager.hpp>
#include <stdexcept>

namespace MoonGlare {

void EngineRunner::Execute() {
    do_soft_restart = false;
    try {
        Initialize();
        // if (!m_World->PreSystemStart()) {
        //     AddLogf(Error, "Failure during PreSystemStart");
        //     return;
        // }

        // WaitForFirstScene();
        // m_Renderer->GetContext()->SetVisible(true);

        engine_thread = std::thread([this]() {
            ::OrbitLogger::ThreadInfo::SetName("CORE");
            engine_core->EngineMain();
        });

        rendering_device->EnterLoop();

        if (engine_thread.joinable()) {
            engine_thread.join();
        }

        // if (!m_World->PreSystemShutdown()) {
        //     AddLogf(Error, "Failure during PreSystemShutdown");
        //     return;
        // }
    } catch (...) {
        Finalize();
        throw;
    }
    Finalize();
}

void EngineRunner::Initialize() {
    // m_World = std::make_unique<World>();
    // m_World->SetInterface(this);
    // m_World->CreateObject<Module::DebugContext, Module::iDebugContext>();
    // m_World->CreateObject<Tools::PerfView::PerfViewClient>();
    // m_World->CreateObject<Modules::BasicConsole, iConsole>();

    auto config = LoadConfiguration();
    script_module_manager = std::make_shared<Lua::LuaScriptContext>();

    // auto moveCfg = std::make_shared<Core::MoveConfig>();
    // m_World->SetSharedInterface(moveCfg);
    // m_World->SetInterface<Component::SubsystemUpdateData>(moveCfg.get());
    // auto ModManager = new ModulesManager(m_World.get());
    // if (!PreSystemInit()) {
    //     AddLogf(Error, "Pre system init action failed!");
    //     throw "Pre system init action failed";
    // }

    filesystem = CreateFilesystem();

    Lua::LoadAllLuaModules(script_module_manager.get(), filesystem);

    async_loader = std::make_shared<ThreadedAsyncLoader>(filesystem);
    LoadDataModules();
    ExecuteHooks([](auto *p) { p->AfterDataModulesLoad(); });

    // m_World->CreateObject<Resources::StringTables>();
    // if (!ModManager->Initialize()) {
    //     AddLogf(Error, "Unable to initialize modules manager!");
    //     throw "Unable to initialize modules manager";
    // }
    // SoundSystem::Component::SoundSystemRegister::Install(*m_World);
    // auto scrEngine = new ScriptEngine(m_World.get());

    device_context = CreateDeviceContext();
    rendering_device = std::make_shared<Renderer::RenderingDevice>(device_context);
    resource_manager = std::make_shared<Renderer::Resources::ResourceManager>(async_loader, rendering_device);
    device_window = device_context->CreateWindow(Renderer::WindowCreationInfo{config.window});

    // m_Renderer = Renderer::iRendererFacade::CreateInstance(*m_World);
    // auto *R = (Renderer::RendererFacade *)m_Renderer.get();
    // R->GetScriptApi()->Install(scrEngine->GetLua());
    // m_Renderer->Initialize(GetDisplaySettings(), GetFileSystem());
    // m_World->SetRendererFacade(R);
    // m_World->CreateObject<Resources::SkeletalAnimationManager>();
    // auto *dataMgr = new DataManager(m_World.get());
    // m_World->SetInterface(dataMgr);
    // LoadDataModules();

    // #ifdef DEBUG_DUMP
    // {
    //     std::ofstream fsvfs("logs/vfs.txt");
    //     GetFileSystem()->DumpStructure(fsvfs);
    // }
    // #endif

    engine_core = std::make_unique<EngineCore>();

    // if (!m_World->Initialize()) {
    //     AddLogf(Error, "Failed to initialize world!");
    //     throw "Failed to initialize world!";
    // }
    // dataMgr->InitFonts();
    // //Temporary solution which probably will be used for eternity
    // auto Input = Engine->GetWorld()->GetInputProcessor();
    // auto rctx = m_Renderer->GetContext();
    // rctx->SetInputHandler(Input);
    // Input->SetInputSource(rctx);

    AddLog(Debug, "Application initialized");

    // R->SetStopObserver([Engine]() { Engine->Exit(); });
    // if (!PostSystemInit()) {
    //     AddLogf(Error, "Post system init action failed!");
    //     throw "Post system init action failed!";
    // }

    // #ifdef DEBUG_DUMP
    //     m_World->DumpObjects();
    // #endif
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

    engine_core.reset();

    // ScriptEngine::DeleteInstance();
    // m_World.reset();

    rendering_device.reset();
    resource_manager.reset();
    device_window.reset();
    device_context.reset();

    async_loader.reset();
    filesystem.reset();
    svfs_hooks.reset();
    script_module_manager.reset();

    AddLog(Debug, "Application finalized");
}

void EngineRunner::Exit() {
    AddLog(Debug, "Exit called");
    if (engine_core) {
        engine_core->Exit();
    }
    if (rendering_device) {
        rendering_device->Stop();
    }
}

//---------------------------------------------------------------------------------------

StarVfs::iStarVfsHooks *EngineRunner::IntSvfsHooks() {
    if (!svfs_hooks) {
        auto h = std::make_unique<Runner::ScriptInitRunnerHook>(script_module_manager->GetCodeRunnerInterface());
        runner_hooks.emplace_back(h.get());
        svfs_hooks = std::move(h);
    }
    return svfs_hooks.get();
}

void EngineRunner::ExecuteHooks(std::function<void(Runner::iEngineRunnerHooks *)> functor) {
    for (auto &item : runner_hooks) {
        functor(item);
    }
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