/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:46:28,36
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "nfRenderer.h"
#include "Configuration.Renderer.h"

#include "IndexBuffer.h"
#include "TextureRenderTask.h"

namespace MoonGlare::Renderer {

class RendererFacade;

class RenderDevice final {
	using ThisClass = RenderDevice;
	using Conf = Configuration::FrameBuffer;
public:
 	RenderDevice();
 	~RenderDevice();

	bool Initialize(RendererFacade *renderer);
	bool Finalize ();
	
	Frame* NextFrame();
	void Submit(Frame *frame);

	void Step();

	bool AllocateTexture(TextureHandle &out) { return m_TextureIndexBuffer.Allocate(out); }
	bool ReleaseTexture(TextureHandle idx) { return m_TextureIndexBuffer.Release(idx); }

	TextureRenderTask* AllocateTextureRenderTask() { return m_UnusedTextureRender.pop(nullptr); }
private:
	std::array<std::unique_ptr<Frame>, Conf::Count> m_Frames;
	std::atomic<uint32_t> m_FreeFrameBuffers;
	std::atomic<Frame*> m_PendingFrame;

	TextureIndexBuffer m_TextureIndexBuffer;
	Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit> m_UnusedTextureRender;
	std::array<TextureRenderTask, Configuration::TextureRenderTask::Limit> m_TextureRenderTask;

	DeclarePerformanceCounter(DroppedFrames);
	DeclarePerformanceCounter(FramesProcessed);
	
	void ProcessFrame(Frame *frame);
};

} //namespace MoonGlare::Renderer
