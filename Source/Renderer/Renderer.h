#pragma once

#include <Foundation/iFileSystem.h>

#include "Configuration.Renderer.h"

#include "iRendererFacade.h"

#include "iContext.h"

namespace MoonGlare::Renderer {

class RendererFacade final : public iRendererFacade {
public:
    RendererFacade();
    ~RendererFacade();

    //iRendererFacade
    iContext* GetContext() override;
    void SetConfiguration(Configuration::RuntimeConfiguration Configuration) override;
    void Initialize(const ContextCreationInfo& ctxifo, iFileSystem *fileSystem) override;
    void Finalize() override;

   


    /** Shall work on main thread; does not return until stopped */
    void EnterLoop();
    void Stop();
    bool CanWork() const { return m_CanWork; }

//	Context* CreateContext(const ContextCreationInfo& ctxifo);

    RenderDevice* GetDevice() {
        RendererAssert(this);
        return m_Device.get();
    }
    Resources::ResourceManager* GetResourceManager() {
        RendererAssert(this);
        return m_ResourceManager.get();
    }

    iAsyncLoader* GetAsyncLoader();

    template<typename T>
    void SetStopObserver(T&& t) {
        m_StopObserver = std::forward<T>(t);
    }

    bool AllResourcesLoaded();

    ScriptApi* GetScriptApi();
    const Configuration::RuntimeConfiguration* GetConfiguration() { return &configuration; }
private:
    bool m_CanWork = false;

    mem::aligned_ptr<RenderDevice> m_Device;
    mem::aligned_ptr<iContext> m_Context;
    mem::aligned_ptr<Resources::ResourceManager> m_ResourceManager;
    std::unique_ptr<ScriptApi> m_ScriptApi;
    Configuration::RuntimeConfiguration configuration;

    std::function<void()> m_StopObserver;
};

} //namespace MoonGlare::Renderer
