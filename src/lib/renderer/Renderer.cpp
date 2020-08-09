
#include<Foundation/Settings.h>

#include "nfRenderer.h"

#include "Renderer.h"

#include "Device/GLFWContext.h"
#include "Frame.h"
#include "RenderDevice.h"
#include "Resources/AsyncLoader.h"
#include "Resources/ResourceManager.h"
#include "ScriptApi.h"

namespace MoonGlare::Renderer {

std::unique_ptr<iRendererFacade> iRendererFacade::CreateInstance(InterfaceMap &ifaceMap) {
    return std::make_unique<RendererFacade>(ifaceMap);
}

//----------------------------------------------------------------------------------

RendererFacade::RendererFacade(InterfaceMap &ifaceMap) : interfaceMap(ifaceMap) {
    configuration.ResetToDefault();
}

RendererFacade::~RendererFacade() {
}

//----------------------------------------------------------------------------------

iContext* RendererFacade::GetContext() {
    return m_Context.get();
}

//----------------------------------------------------------------------------------

Settings::ApplyMethod RendererFacade::ValueChanged(const std::string &key, Settings* siface) {
    ReloadConfig();
    return Settings::ApplyMethod::DontCare;
}

void RendererFacade::ReloadConfig() {
    auto stt = interfaceMap.GetSharedInterface<Settings>();
    assert(stt);

    //configuration.m_Texture.m_Filtering =

    configuration.ResetToDefault();

    stt->Get("Renderer.Texture.Filtering", configuration.texture.m_Filtering);

    stt->Get("Renderer.Shadow.MapSize", configuration.shadow.shadowMapSize);
    stt->Get("Renderer.Shadow.Enabled", configuration.shadow.enableShadows);

    stt->Get("Renderer.Shader.GaussianDiscLength", configuration.shader.gaussianDiscLength);
    stt->Get("Renderer.Shader.GaussianDiscRadius", configuration.shader.gaussianDiscRadius);

    stt->Get("Renderer.Gamma", configuration.gammaCorrection);
}

//----------------------------------------------------------------------------------

void RendererFacade::Initialize(const ContextCreationInfo& ctxifo, iFileSystem *fileSystem) {
    assert(fileSystem);

    struct SettingsChangeCb : Settings::iChangeCallback {
        RendererFacade* owner;
        SettingsChangeCb(RendererFacade* o) :owner(o) {}
        Settings::ApplyMethod ValueChanged(const std::string &key, Settings* siface) {
            return owner->ValueChanged(key, siface);
        }
    };
    settingsChangeCallback = std::make_shared<SettingsChangeCb>(this);

    ReloadConfig();

    auto stt = interfaceMap.GetSharedInterface<Settings>();
    assert(stt);
    stt->Subscribe(settingsChangeCallback);

    if (!Device::GLFWContext::InitializeSubSystem()) {
        AddLogf(Error, "Context subsystem initialization failed!");
        throw InitFailureException("Context subsystem initialization failed!");
    }

    m_Device = Memory::make_aligned<RenderDevice>(interfaceMap);
    m_ResourceManager = Memory::make_aligned<Resources::ResourceManager>();

    m_Context = Memory::make_aligned<Device::GLFWContext>(ctxifo, m_Device.get());

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
    assert(this);
    return GetResourceManager()->GetLoader();
}

} //namespace MoonGlare::Renderer

