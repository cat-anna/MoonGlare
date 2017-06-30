/*
 * cRenderDevice.h
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */

#ifndef CRENDERDEVICE_H_
#define CRENDERDEVICE_H_

namespace Graphic {

class cRenderDevice : public cRootClass {
    SPACERTTI_DECLARE_CLASS_SINGLETON(cRenderDevice, cRootClass)
public:
    cRenderDevice();

    void RequestContextManip(LoadQueue::LoadRequestFunc func);
    void DelayedContextManip(LoadQueue::LoadRequestFunc func) { m_LoadQueue.QueueRequest(func); }
    void DispatchContextManipRequests();
protected:
    LoadQueue m_LoadQueue;
    std::thread::id m_InitThreadId;
}; 

} // namespace Graphic 

inline Graphic::cRenderDevice* GetRenderDevice() {
    return Graphic::cRenderDevice::Instance();
}

#endif // CRENDERDEVICE_H_ 
