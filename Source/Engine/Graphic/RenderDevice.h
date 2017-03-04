/*
 * cRenderDevice.h
 *
 *  Created on: 19-11-2013
 *      Author: Paweu
 */

#ifndef CRENDERDEVICE_H_
#define CRENDERDEVICE_H_

namespace Graphic {

/**
	This class represents (more or less) current state of opengl context
*/
class cRenderDevice : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(cRenderDevice, cRootClass)
public:
	cRenderDevice(WindowPtr Context, Asset::AssetManager *AssetManager);
	virtual ~cRenderDevice();

	mem::aligned_ptr<RenderInput> CreateRenderInput();

	bool Initialize(); 
	bool Finalize();
	void CheckError() const;

	void BeginFrame() { ++m_FrameIndex; }
	void EndFrame() { GetContext()->SwapBuffers(); }
	uint64_t FrameIndex() const { return m_FrameIndex; }

	void ResetViewPort() { GetContext()->ResetViewPort(); }
	void ClearBuffer() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		 
	static float m_gamma;

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

	Asset::AssetManager *m_AssetManager;
	WindowPtr m_Context;

	math::mat4 m_ModelMatrix, m_WorldMatrix;
}; 

} // namespace Graphic 
#endif // CRENDERDEVICE_H_ 
