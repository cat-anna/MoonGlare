#pragma once

#include "engine_configuration.hpp"
#include "engine_core.hpp"
#include "engine_runner/engine_runner_hooks.hpp"
#include <async_loader.hpp>
#include <device_context.hpp>
#include <lua_context/script_module.hpp>
#include <readonly_file_system.h>
#include <rendering_device.hpp>
#include <resources.hpp>
#include <svfs/svfs_hooks.hpp>
#include <thread>
#include <vector>

namespace MoonGlare {

class EngineRunner {
public:
    EngineRunner() = default;
    virtual ~EngineRunner() = default;

    // void SaveSettings();
    // void Restart();

    void Initialize();
    void Execute();
    void Finalize();

    // virtual bool PreSystemInit();
    // virtual bool PostSystemInit();

    void Exit();
    // virtual void OnActivate();
    // virtual void OnDeactivate();
    // virtual std::string ApplicationPath() const = 0;
    // virtual std::string SettingsPath() const;

    // bool IsActive() const { return m_Flags.m_Active; }
    bool WantsSoftRestart() const { return do_soft_restart; }
    // void SetRestart(bool v) { m_Flags.m_Restart = v; }

protected:
    std::shared_ptr<iReadOnlyFileSystem> filesystem;
    std::shared_ptr<Lua::iScriptModuleManager> script_module_manager;
    std::shared_ptr<Renderer::iDeviceContext> device_context;
    std::shared_ptr<Renderer::iDeviceWindow> device_window;
    std::shared_ptr<Renderer::iResourceManager> resource_manager;
    std::shared_ptr<iAsyncLoader> async_loader;
    std::shared_ptr<Renderer::RenderingDevice> rendering_device;
    std::unique_ptr<EngineCore> engine_core;
    std::unique_ptr<StarVfs::iStarVfsHooks> svfs_hooks;

    std::vector<Runner::iEngineRunnerHooks *> runner_hooks;

    bool do_soft_restart = false;
    std::thread engine_thread;

    virtual std::shared_ptr<iReadOnlyFileSystem> CreateFilesystem() = 0;
    virtual void LoadDataModules() = 0;
    virtual std::shared_ptr<Renderer::iDeviceContext> CreateDeviceContext() = 0;
    virtual EngineConfiguration LoadConfiguration() const = 0;
    virtual StarVfs::iStarVfsHooks *IntSvfsHooks();

    void ExecuteHooks(std::function<void(Runner::iEngineRunnerHooks *)> functor);

    // Renderer::UniqueRenderer m_Renderer;
    // std::shared_ptr<Settings> settings;
    // void WaitForFirstScene();
    // virtual std::shared_ptr<Settings> GetUpperLayerSettings() { return nullptr; };
};

} //namespace MoonGlare
