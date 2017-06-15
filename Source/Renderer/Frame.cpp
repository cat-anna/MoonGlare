/*
  * Generated by cppsrc.sh
  * On 2017-02-10 15:35:25,52
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "nfRenderer.h"
#include "Frame.h"

#include "Renderer.h"
#include "Context.h"

#include "TextureRenderTask.h"

namespace MoonGlare::Renderer {

bool Frame::Initialize(uint8_t BufferIndex, RenderDevice *device, RendererFacade *rfacade) {
	RendererAssert(BufferIndex < Configuration::FrameBuffer::Count);
	RendererAssert(device);
	RendererAssert(rfacade);
	
	m_BufferIndex = BufferIndex;
	m_RenderDevice = device;
	m_ResourceManager = rfacade->GetResourceManager();

	m_QueuedTextureRender.ClearAllocation();
	m_SubQueueTable.ClearAllocation();

	m_CommandLayers.Clear();
	m_WindowLayers.Clear();

	m_Memory.Clear();

	for (auto &q : m_SubQueueTable)
		q.Clear();

	auto ctx = rfacade->GetContextImpl();
	RendererAssert(ctx);
	ctx->InitializeWindowLayer(m_WindowLayers.Get<ConfCtx::Window::First>(), this);
	
	return true;
}

bool Frame::Finalize() {
	return true;
}

//----------------------------------------------------------------------------------

void Frame::BeginFrame() {
	m_QueuedTextureRender.ClearAllocation();
	m_SubQueueTable.ClearAllocation();

	m_CommandLayers.ClearAllocation();
	m_WindowLayers.ClearAllocation();

	m_FrameResourceStorage.Clear();

	m_Memory.Clear();
}

void Frame::EndFrame() {
}

//----------------------------------------------------------------------------------
 
bool Frame::Submit(TextureRenderTask *trt) {
	RendererAssert(trt);
	m_CommandLayers.Get<Conf::Layer::PreRender>().PushQueue(&trt->GetCommandQueue());
	return m_QueuedTextureRender.push(trt);
}

bool Frame::Submit(SubQueue *q, ConfCtx::Window WindowLayer, Commands::CommandKey Key) {
	RendererAssert(q);
	m_WindowLayers[WindowLayer].PushQueue(q, Key);
	return false;
}

bool Frame::Submit(SubQueue *q, Conf::Layer Layer, Commands::CommandKey Key) {
	RendererAssert(q);
	m_CommandLayers[Layer].PushQueue(q, Key);
	return false;
}

//----------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer 
