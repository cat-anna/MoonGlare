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

cRenderDevice::cRenderDevice() :
        cRootClass(),
        m_InitThreadId() {
    SetThisAsInstance();

    m_InitThreadId = std::this_thread::get_id();
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

} // namespace Graphic 
