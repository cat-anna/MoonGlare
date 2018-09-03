#include "VAOResource.h"

#include "../../Frame.h"
#include "../../Commands/CommandQueue.h"
#include "../../Commands/OpenGL/ArrayCommands.h"

namespace MoonGlare::Renderer::Resources {

VAOResource::VAOResource(ResourceManager* Owner) {
    m_GLHandle.fill(Device::InvalidVAOHandle);
    m_GLVAOBuffsers.fill(VAOBuffers{ 0 });
    m_AllocationBitmap.ClearAllocation();
    generations.fill(1);
    m_ResourceManager = Owner;
}

VAOResource::~VAOResource() {
}

bool VAOResource::Allocate(Frame *frame, VAOResourceHandle &out) {
	assert(frame);
	return Allocate(frame->GetControllCommandQueue(), out);
}

void VAOResource::Release(Frame *frame, VAOResourceHandle &h) {
	assert(frame);
	return Release(frame->GetControllCommandQueue(), h);
}

bool VAOResource::Allocate(Commands::CommandQueue &queue, VAOResourceHandle &out) {
	Bitmap::Index_t index;
	if (m_AllocationBitmap.Allocate(index)) {
		if (m_GLHandle[index] == Device::InvalidVAOHandle) {
			auto arg = queue.PushCommand<Commands::VAOSingleAllocate>();
			arg->m_Out = &m_GLHandle[index];
		}
		out.index = static_cast<VAOResourceHandle::Index_t>(index);
		out.generation = generations[out.index];
        out.deviceHandle = &m_GLHandle[out.index];
		return true;
	}
	else {
		AddLogf(Debug, "VAO allocation failed");
		return false;
	}
}

void VAOResource::Release(Commands::CommandQueue &queue, VAOResourceHandle &h) {
    if (!IsHandleValid(h))
        return;

	if (m_AllocationBitmap.Release(h.index)) {
	}
	else {
		AddLogf(Debug, "VAO deallocation failed");
	}
	h.Zero();
}

bool VAOResource::IsHandleValid(VAOResourceHandle &h) const {
    if (h.index >= Conf::VAOLimit)
        return false;
    if (generations[h.index] != h.generation) {
        return false;
    }
    return true;
}

} //namespace MoonGlare::Renderer::Resources 
		