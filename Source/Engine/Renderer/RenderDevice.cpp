/*
 * cRenderDevice.cpp
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>

namespace Graphic {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(cRenderDevice)
RegisterDebugApi(Graphic, &cRenderDevice::RegisterDebugScriptApi, "Graphic");

cRenderDevice::cRenderDevice() :
        cRootClass(),
        m_InitThreadId() {
    SetThisAsInstance();

    m_InitThreadId = std::this_thread::get_id();
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

} // namespace Graphic 
