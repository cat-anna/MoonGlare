/*
  * Generated by cppsrc.sh
  * On 2017-02-13 22:45:35,93
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#include "VAOResource.h"

#include "../Frame.h"
#include "../Commands/CommandQueue.h"
#include "../Commands/OpenGL/ArrayCommands.h"

namespace MoonGlare::Renderer::Resources {

bool VAOResource::Initialize(ResourceManager *Owner) {
	RendererAssert(Owner);

	m_GLHandle.fill(InvalidVAOHandle);
	m_GLVAOBuffsers.fill(VAOBuffers{ 0 });
	m_AllocationBitmap.ClearAllocation();

	if (Conf::VAOInitial > 0) {
		//TBD!!!
	}

	m_ResourceManager = Owner;
	return true;
}

bool VAOResource::Finalize() {
	return true;
}

bool VAOResource::Allocate(Frame *frame, VAOResourceHandle &out) {
	RendererAssert(frame);
	return Allocate(frame->GetControllCommandQueue(), out);
}

void VAOResource::Release(Frame *frame, VAOResourceHandle &h) {
	RendererAssert(frame);
	return Release(frame->GetControllCommandQueue(), h);
}

bool VAOResource::Allocate(Commands::CommandQueue &queue, VAOResourceHandle &out) {
	Bitmap::Index_t index;
	if (m_AllocationBitmap.Allocate(index)) {
		if (m_GLHandle[index] == InvalidVAOHandle) {
			IncrementPerformanceCounter(OpenGLAllocations);
			auto arg = queue.PushCommand<Commands::VAOSingleAllocate>();
			arg->m_Out = &m_GLHandle[index];
		}
		out.m_Index = static_cast<VAOResourceHandle::Index_t>(index);
		out.m_TmpGuard = GuardValue;
		IncrementPerformanceCounter(SuccessfulAllocations);
		return true;
	}
	else {
		AddLogf(Debug, "VAO allocation failed");
		IncrementPerformanceCounter(FailedAllocations);
		return false;
	}
}

void VAOResource::Release(Commands::CommandQueue &queue, VAOResourceHandle &h) {
	RendererAssert(h.m_TmpGuard == GuardValue);
	RendererAssert(h.m_Index < Conf::VAOLimit);

	if (m_AllocationBitmap.Release(h.m_Index)) {
		IncrementPerformanceCounter(SuccessfulDellocations);
	}
	else {
		AddLogf(Debug, "VAO deallocation failed");
		IncrementPerformanceCounter(FailedDellocations);
	}
	h.Reset();
}

} //namespace MoonGlare::Renderer::Resources 
		