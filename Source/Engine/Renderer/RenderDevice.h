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

	bool Finalize();

	void BeginFrame() { ++m_FrameIndex; }
	uint64_t FrameIndex() const { return m_FrameIndex; }

	void RequestContextManip(LoadQueue::LoadRequestFunc func);
	void DelayedContextManip(LoadQueue::LoadRequestFunc func) { m_LoadQueue.QueueRequest(func); }
	void DispatchContextManipRequests();

	static void RegisterDebugScriptApi(ApiInitializer &api);
protected:
	uint64_t m_FrameIndex = 0;
	LoadQueue m_LoadQueue;
	std::thread::id m_InitThreadId;
}; 

} // namespace Graphic 
#endif // CRENDERDEVICE_H_ 
