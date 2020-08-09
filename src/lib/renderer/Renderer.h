#pragma once

#include <Foundation/Settings.h>
#include <Foundation/iFileSystem.h>
#include <interface_map.h>

#include "Configuration.Renderer.h"

#include "iRendererFacade.h"

#include "iContext.h"

namespace MoonGlare::Renderer {

class RendererFacade final : public iRendererFacade {
  public:
    RendererFacade(InterfaceMap &ifaceMap);
    ~RendererFacade();

    // iRendererFacade
    iContext *GetContext() override;
    // void SetConfiguration(Configuration::RuntimeConfiguration Configuration)
    // override;
    void Initialize(const ContextCreationInfo &ctxifo,
                    iFileSystem *fileSystem) override;
    void Finalize() override;

    /** Shall work on main thread; does not return until stopped */
    void EnterLoop();
    void Stop();
    bool CanWork() const { return m_CanWork; }

    //	Context* CreateContext(const ContextCreationInfo& ctxifo);

    RenderDevice *GetDevice() {
        assert(this);
        return m_Device.get();
    }
    Resources::ResourceManager *GetResourceManager() {
        assert(this);
        return m_ResourceManager.get();
    }

    iAsyncLoader *GetAsyncLoader();

    template <typename T> void SetStopObserver(T &&t) {
        m_StopObserver = std::forward<T>(t);
    }

    ScriptApi *GetScriptApi();
    const Configuration::RuntimeConfiguration *GetConfiguration() {
        return &configuration;
    }

    // Settings::iChangeCallback
    Settings::ApplyMethod ValueChanged(const std::string &key,
                                       Settings *siface);

  private:
    InterfaceMap &interfaceMap;
    Configuration::RuntimeConfiguration configuration;

    bool m_CanWork = false;
    Memory::aligned_ptr<RenderDevice> m_Device;
    Memory::aligned_ptr<iContext> m_Context;
    Memory::aligned_ptr<Resources::ResourceManager> m_ResourceManager;
    std::unique_ptr<ScriptApi> m_ScriptApi;

    std::shared_ptr<Settings::iChangeCallback> settingsChangeCallback;

    std::function<void()> m_StopObserver;

    void ReloadConfig();
};

} // namespace MoonGlare::Renderer
