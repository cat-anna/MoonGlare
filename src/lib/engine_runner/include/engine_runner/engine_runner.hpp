#pragma once

#include "async_loader.hpp"
#include "core/engine_core.hpp"
#include "core/engine_runner_interface.hpp"
#include "core/stop_interface.hpp"
#include "ecs/ecs_register.hpp"
#include "engine_runner/engine_configuration.hpp"
#include "engine_runner/engine_runner_hooks.hpp"
#include "input_handler/input_processor.hpp"
#include "lua_context/script_module.hpp"
#include "readonly_file_system.h"
#include "renderer/device_context.hpp"
#include "renderer/facade.hpp"
#include "renderer/rendering_device.hpp"
#include "renderer/resources.hpp"
#include "scene_manager/prefab_manager_interface.hpp"
#include "scene_manager/scenes_manager_interface.hpp"
#include "svfs/svfs_hooks.hpp"
#include <thread>
#include <vector>

namespace MoonGlare {

class EngineRunner : public iEngineRunner {
public:
    EngineRunner();
    ~EngineRunner() override;

    void Execute();

    bool WantsSoftRestart() const override { return do_soft_restart; }
    void SetSoftRestart(bool v) { do_soft_restart = v; }

    void Stop() override;

protected:
    std::shared_ptr<iReadOnlyFileSystem> filesystem;
    std::shared_ptr<iAsyncLoader> async_loader;
    std::unique_ptr<EngineCore> engine_core;

    std::shared_ptr<Lua::iScriptContext> script_context;

    std::shared_ptr<Renderer::iDeviceContext> device_context;
    std::shared_ptr<Renderer::iDeviceWindow> device_window;
    std::shared_ptr<Renderer::iResourceManager> resource_manager;
    std::shared_ptr<Renderer::RenderingDevice> rendering_device;

    std::unique_ptr<StarVfs::iStarVfsHooks> svfs_hooks;

    std::unique_ptr<InputHandler::InputProcessor> input_processor;

    std::unique_ptr<SceneManager::iScenesManager> scene_manager;
    std::unique_ptr<SceneManager::iPrefabManager> prefab_manager;

    ECS::ECSRegister ecs_register;

    bool do_soft_restart = false;
    std::thread engine_thread;

    virtual std::shared_ptr<iReadOnlyFileSystem> CreateFilesystem() = 0;
    virtual void LoadDataModules() = 0;
    virtual std::shared_ptr<Renderer::iDeviceContext> CreateDeviceContext() = 0;
    virtual EngineConfiguration LoadConfiguration() const = 0;
    virtual StarVfs::iStarVfsHooks *IntSvfsHooks();

    void Initialize();
    void InitSceneManager();
    void Teardown();

    // Renderer::UniqueRenderer m_Renderer;
    // std::shared_ptr<Settings> settings;
    // void WaitForFirstScene();
    // virtual std::shared_ptr<Settings> GetUpperLayerSettings() { return nullptr; };
private:
    Runner::EngineRunnerHooksHost runner_hooks;
};

} //namespace MoonGlare
