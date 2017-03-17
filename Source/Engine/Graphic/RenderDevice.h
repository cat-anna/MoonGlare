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
	cRenderDevice(WindowPtr Context);

	mem::aligned_ptr<RenderInput> CreateRenderInput();

	bool Finalize();

	void BeginFrame() { ++m_FrameIndex; }
	uint64_t FrameIndex() const { return m_FrameIndex; }

	const WindowPtr& GetContext() const { return m_Context; }

	const math::uvec2& GetContextSize() const { return m_Context->Size(); }

	void RequestContextManip(LoadQueue::LoadRequestFunc func);
	void DelayedContextManip(LoadQueue::LoadRequestFunc func) { m_LoadQueue.QueueRequest(func); }
	void DispatchContextManipRequests();

	/** Read pixels from opengl screen */
	void ReadScreenPixels(void *memory, const uvec2 &size, unsigned &glType);

	static void RegisterDebugScriptApi(ApiInitializer &api);
protected:
	volatile uint64_t m_FrameIndex = 0;
	math::mat4 m_CameraMatrix;
	LoadQueue m_LoadQueue;
	std::thread::id m_InitThreadId;

	WindowPtr m_Context;
}; 

} // namespace Graphic 
#endif // CRENDERDEVICE_H_ 
