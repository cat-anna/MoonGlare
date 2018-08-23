#include "nfRenderer.h"
#include "Renderer.h"

#include "RenderDevice.h"
#include "Frame.h"
#include "Device/GLFWContext.h"
#include "Resources/ResourceManager.h"
#include "Resources/AsyncLoader.h"
#include "ScriptApi.h"

namespace MoonGlare::Renderer {

RendererFacade::RendererFacade() {
    configuration.ResetToDefault();
}

RendererFacade::~RendererFacade() {
}

//----------------------------------------------------------------------------------

iContext* RendererFacade::GetContext() {
    return m_Context.get();
}

void RendererFacade::SetConfiguration(Configuration::RuntimeConfiguration Configuration) {
    configuration = Configuration;
}

//----------------------------------------------------------------------------------

bool RendererFacade::Initialize(const ContextCreationInfo& ctxifo, Asset::AssetLoader *Assets, iFileSystem *fileSystem) {
    assert(Assets);
    assert(fileSystem);
    assets = Assets;

    if (!Device::GLFWContext::InitializeSubSystem()) {
        AddLogf(Error, "Context subsystem initialization failed!");
        return false;
    }

    m_Device = mem::make_aligned<RenderDevice>();
    m_ResourceManager = mem::make_aligned<Resources::ResourceManager>();

    m_Context = mem::make_aligned<Device::GLFWContext>(ctxifo);

    if(!m_Device->Initialize(this)) {
        AddLogf(Error, "Render device initialization failed!");
        return false;
    }

    if (!m_ResourceManager->Initialize(this, Assets, fileSystem)) {
        AddLogf(Error, "ResourceManager initialization failed!");
        return false;
    }

    return true;
}

bool RendererFacade::Finalize() {
    if (m_ResourceManager && !m_ResourceManager->Finalize()) {
        AddLogf(Error, "ResourceManager finalization failed!");
    }
    m_ResourceManager.reset();

    if(m_Device && !m_Device->Finalize()) {
        AddLogf(Error, "Render device finalization failed!");
    }
    m_Device.reset();

    m_Context.reset();

    if (!Device::GLFWContext::FinalizeSubSystem()) {
        AddLogf(Error, "Context subsystem finalization failed!");
    }

    return true;
}

//----------------------------------------------------------------------------------

void RendererFacade::EnterLoop() {
//    m_CanWork = true;
    m_CanWork = false;  //hue hue
}

void RendererFacade::Stop() {
    m_CanWork = false;
    if (m_StopObserver)
        m_StopObserver();
}

//----------------------------------------------------------------------------------

ScriptApi *RendererFacade::GetScriptApi() {
    if (!m_ScriptApi)
        m_ScriptApi = std::make_unique<ScriptApi>(this);
    return m_ScriptApi.get();
}

bool RendererFacade::AllResourcesLoaded() {
    return GetResourceManager()->GetLoader()->AllResoucecsLoaded();
}

//----------------------------------------------------------------------------------

iAsyncLoader* RendererFacade::GetAsyncLoader() {
    RendererAssert(this);
    return GetResourceManager()->GetLoaderIf();
}

} //namespace MoonGlare::Renderer
