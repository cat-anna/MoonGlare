/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:34:50,23
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "nfRenderer.h"
#include "Renderer.h"

#include "RenderDevice.h"
#include "Frame.h"
#include "Context.h"
#include "Resources/ResourceManager.h"
#include "Resources/AsyncLoader.h"
#include "ScriptApi.h"

namespace MoonGlare::Renderer {

RendererFacade::RendererFacade() {
    SetConfiguration(nullptr);
}

RendererFacade::~RendererFacade() {
}

//----------------------------------------------------------------------------------

iContext* RendererFacade::GetContext() {
    return m_Context.get();
}

//----------------------------------------------------------------------------------

void RendererFacade::SetConfiguration(const Configuration::RuntimeConfiguration *Configuration) {
    if (m_Device) {
        throw "Cannot change configuration after initialization!";
    }
    if (!Configuration) {
        static Configuration::RuntimeConfiguration rtcfg;
        rtcfg.ResetToDefault();
        m_Configuration = &rtcfg;
    } else {
        m_Configuration = Configuration;
    }
}

bool RendererFacade::Initialize(const ContextCreationInfo& ctxifo, Asset::AssetLoader *Assets) {
    RendererAssert(Assets);

    if (!Context::InitializeSubSystem()) {
        AddLogf(Error, "Context subsystem initialization failed!");
        return false;
    }

    m_Context = mem::make_aligned<Context>();
    m_Device = mem::make_aligned<RenderDevice>();
    m_ResourceManager = mem::make_aligned<Resources::ResourceManager>();

    m_Context->Initialize(ctxifo, this, m_Device.get());

    if(!m_Device->Initialize(this)) {
        AddLogf(Error, "Render device initialization failed!");
        return false;
    }

    if (!m_ResourceManager->Initialize(this, Assets)) {
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

    if (m_Context)
        m_Context->Finalize();
    m_Context.reset();

    if (!Context::FinalizeSubSystem()) {
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

} //namespace MoonGlare::Renderer
