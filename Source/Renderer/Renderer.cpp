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

void RendererFacade::Initialize(const ContextCreationInfo& ctxifo, iFileSystem *fileSystem) {
    assert(fileSystem);

    if (!Device::GLFWContext::InitializeSubSystem()) {
        AddLogf(Error, "Context subsystem initialization failed!");
        throw InitFailureException("Context subsystem initialization failed!");
    }

    m_Device = mem::make_aligned<RenderDevice>();
    m_ResourceManager = mem::make_aligned<Resources::ResourceManager>();

    m_Context = mem::make_aligned<Device::GLFWContext>(ctxifo);

    if(!m_Device->Initialize(this)) {
        AddLogf(Error, "Render device initialization failed!");
        throw InitFailureException("Render device initialization failed!");
    }

    m_Context->Flush();

    if (!m_ResourceManager->Initialize(this, fileSystem)) {
        AddLogf(Error, "ResourceManager initialization failed!");
        throw InitFailureException("ResourceManager initialization failed!");
    }
}

void RendererFacade::Finalize() {
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
}

//----------------------------------------------------------------------------------

void RendererFacade::EnterLoop() {
    m_CanWork = true;
    //m_CanWork = false;  //hue hue
    while (m_CanWork) {
        m_Device->Step();
    }
}

void RendererFacade::Stop() {
    m_CanWork = false;
    //if (m_StopObserver)
        //m_StopObserver();
}

//----------------------------------------------------------------------------------

ScriptApi *RendererFacade::GetScriptApi() {
    if (!m_ScriptApi)
        m_ScriptApi = std::make_unique<ScriptApi>(this);
    return m_ScriptApi.get();
}

//----------------------------------------------------------------------------------

iAsyncLoader* RendererFacade::GetAsyncLoader() {
    RendererAssert(this);
    return GetResourceManager()->GetLoader();
}

} //namespace MoonGlare::Renderer
