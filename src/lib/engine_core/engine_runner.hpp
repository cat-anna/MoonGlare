#pragma once

#include "engine_configuration.hpp"
#include "engine_core.hpp"
#include <async_loader.hpp>
#include <device_context.hpp>
#include <readonly_file_system.h>
#include <rendering_device.hpp>
#include <resources.hpp>
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
    std::shared_ptr<Renderer::iDeviceContext> device_context;
    std::shared_ptr<Renderer::iDeviceWindow> device_window;
    std::shared_ptr<Renderer::iResourceManager> resource_manager;
    std::shared_ptr<iAsyncLoader> async_loader;
    std::shared_ptr<Renderer::RenderingDevice> rendering_device;
    std::unique_ptr<EngineCore> engine_core;

    bool do_soft_restart = false;
    std::thread engine_thread;

    virtual std::shared_ptr<iReadOnlyFileSystem> CreateFilesystem() = 0;
    virtual std::shared_ptr<Renderer::iDeviceContext> CreateDeviceContext() = 0;
    virtual EngineConfiguration LoadConfiguration() const = 0;

    // Renderer::UniqueRenderer m_Renderer;
    // std::shared_ptr<Settings> settings;
    // void WaitForFirstScene();
    // virtual std::shared_ptr<Settings> GetUpperLayerSettings() { return nullptr; };
};

} //namespace MoonGlare
