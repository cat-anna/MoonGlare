/*
 * cRenderDevice.cpp
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

#include <Renderer/RenderInput.h>

namespace Graphic {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(cRenderDevice)
RegisterDebugApi(Graphic, &cRenderDevice::RegisterDebugScriptApi, "Graphic");
inline Window* WindowInstance() { return GetRenderDevice()->GetContext().get(); }
RegisterApiDynamicInstance(Window, &WindowInstance, "Window");

//float cRenderDevice::m_gamma = 0;
  
cRenderDevice::cRenderDevice(WindowPtr Context, Asset::AssetManager *AssetManager) :
        cRootClass(),
        m_AssetManager(AssetManager),
        m_InitThreadId(),
        m_Context() {
    SetThisAsInstance();
    m_Context.swap(Context);

    m_InitThreadId = std::this_thread::get_id();
}  

mem::aligned_ptr<RenderInput> cRenderDevice::CreateRenderInput() {
    return mem::make_aligned<RenderInput>();
}

bool cRenderDevice::Finalize() {
    AddLog(Performance, "Frames drawn: " << m_FrameIndex);
    return true;
}

//----------------------------------------------------------------------------------
 
void cRenderDevice::RequestContextManip(LoadQueue::LoadRequestFunc func) {
    if (std::this_thread::get_id() == m_InitThreadId)
        return func();
    m_LoadQueue.QueueRequest(func);
}

void cRenderDevice::DispatchContextManipRequests() {
    if (std::this_thread::get_id() != m_InitThreadId) {
        AddLog(Error, "OpenGL init requests cannot be dispatched from non owning thread!");
        return;
    }
    m_LoadQueue.DispatchAllRequests();
}

//----------------------------------------------------------------------------------

#ifdef DEBUG
void cRenderDevice::RegisterDebugScriptApi(ApiInitializer &api) {
    struct T {
        static void SetWireFrameMode(bool value) {
            if (value) glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            else glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
    };
    api
    .addFunction("SetWireFrameMode", &T::SetWireFrameMode)
    //.addVariable("Gamma", &m_gamma, true)
    ;
}
#endif

//----------------------------------------------------------------------------------

void cRenderDevice::ReadScreenPixels(void *memory, const uvec2 &size, unsigned &glType) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); 
    glReadPixels(0, 0, size[0], size[1], GL_RGB, GL_UNSIGNED_BYTE, memory);
    glType = Flags::fBGR;
}

} // namespace Graphic 
