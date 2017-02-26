/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:46:28,36
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "nfRenderer.h"
#include "RenderDevice.h"
#include "Renderer.h"
#include "Frame.h"

#include "Device/ErrorHandler.h"
#include "Device/DeviceInfo.h"

namespace MoonGlare::Renderer {

bool RenderDevice::Initialize(RendererFacade *renderer) {
	RendererAssert(renderer);

	m_RendererFacade = renderer;

	CriticalCheck(glewInit() == GLEW_OK, "Unable to initialize GLEW!");
	AddLog(Debug, "GLEW initialized");
	AddLog(System, "GLEW version: " << (char*) glewGetString(GLEW_VERSION));

	Device::ErrorHandler::RegisterErrorCallback();
	Device::DeviceInfo::ReadInfo();

	for (uint8_t idx = 0; idx < Conf::Count; ++idx) {
		auto bit = 1u << idx;
		m_FreeFrameBuffers.fetch_or(bit);
		auto &buffer = m_Frames[idx];
		buffer = mem::make_aligned<Frame>();
		if (!buffer->Initialize(idx, this, renderer)) {
			AddLogf(Error, "Frame buffer initialization failed!");
			return false;
		}
	}

	m_UnusedTextureRender.fill(nullptr);
	for (auto &item : m_TextureRenderTask) {
		if (!item.Initialize()) {
			AddLogf(Error, "TextureRenderTask initialization failed!");
			return false;
		}
		m_UnusedTextureRender.push(&item);
	}

	m_SubmittedCtrlQueues.store(0);
	m_AllocatedCtrlQueues.ClearAllocation();
	for (auto & item : m_CtrlQueues) {
		item.Clear();
	}

	return true;
}

bool RenderDevice::Finalize() {

	for (auto &buffer : m_Frames) {
		if (!buffer->Finalize()) {
			AddLogf(Error, "Frame buffer finalization failed!");
		}
		buffer.reset();
	}

	return true;
}

//----------------------------------------------------------------------------------

Frame* RenderDevice::NextFrame() {
	for (uint32_t idx = 0; idx < Conf::Count; ++idx) {
		auto bit = 1u << idx;
		if ((m_FreeFrameBuffers.fetch_and(~bit) & bit) == bit) {
			auto fr = m_Frames[idx].get();
			fr->BeginFrame();
			return fr;
		}
	}

	return nullptr;
}

void RenderDevice::Submit(Frame *frame) {
	auto prevframe = m_PendingFrame.exchange(frame);
	if (prevframe) {
		auto bit = 1 << prevframe->Index();
		m_FreeFrameBuffers.fetch_or(bit);
		IncrementPerformanceCounter(DroppedFrames);
	}
}

void RenderDevice::ReleaseFrame(Frame *frame) {
	RendererAssert(frame);
	
	frame->EndFrame();

	auto &trtq = frame->GetTextureRenderQueue();
	for (auto *task : trtq) {
		m_UnusedTextureRender.push(task);
	}

	auto bit = 1 << frame->Index();
	m_FreeFrameBuffers.fetch_or(bit);
}

Frame *RenderDevice::PendingFrame() {
	return m_PendingFrame.exchange(nullptr);
}

//----------------------------------------------------------------------------------

void RenderDevice::Step() {
	auto frame = PendingFrame();
	if (!frame)
		return;

	ProcessFrame(frame);
	ReleaseFrame(frame);
}

void RenderDevice::ProcessFrame(Frame *frame) {
	RendererAssert(frame);

	ProcessPendingCtrlQueues();
	frame->GetCommandLayers().Execute();
	frame->EndFrame();

	IncrementPerformanceCounter(FramesProcessed);
}

//----------------------------------------------------------------------------------

RenderDevice::CtrlCommandQueue RenderDevice::AllocateCtrlQueue() {
	uint32_t index;
	if (!m_AllocatedCtrlQueues.Allocate(index)) {
		return CtrlCommandQueue{
			nullptr,
			0,
		};
	}
	m_CtrlQueues[index].ClearAllocation();
	return CtrlCommandQueue {
		&m_CtrlQueues[index],
		index,
	};
}

void RenderDevice::Submit(CtrlCommandQueue &queue) {
	if (queue.m_Handle > m_CtrlQueues.size()) {
		RendererAssert(false);
	}

	m_SubmittedCtrlQueues.fetch_or(1 << queue.m_Handle);
}

void RenderDevice::ProcessPendingCtrlQueues() {
	while (m_SubmittedCtrlQueues.load() != 0) {
		for (size_t bit = 0; bit < 32; ++bit) {
			auto mask = 1 << bit;
			if ((m_SubmittedCtrlQueues.fetch_and(~mask) & mask) != 0) {
				m_CtrlQueues[bit].Execute();
				m_AllocatedCtrlQueues.Release(bit);
			}
		}
	}
}

//----------------------------------------------------------------------------------

} //namespace MoonGlare::Renderer
