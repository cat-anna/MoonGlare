/*
  * Generated by cppsrc.sh
  * On 2017-02-10 15:35:25,52
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "Configuration.Renderer.h"
#include "Commands/CommandQueue.h"

namespace MoonGlare::Renderer {

class alignas(16) Frame final {
public:
	using TextureRenderQueue = Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit>;

	template<typename T>
	using ByteArray = Space::Memory::StaticMemory<T, Configuration::FrameBuffer::MemorySize>;
	using Allocator_t = Space::Memory::StackAllocator<ByteArray>;

	TextureRenderQueue& GetTextureRenderQueue() { return m_QueuedTextureRender; }
	Commands::CommandQueue& GetControllCommandQueue() {
		RendererAssert(this);
		return m_ControllQueue;
	}
	Allocator_t& GeMemory() { return m_Memory; }

	void BeginFrame();
	void EndFrame();

	bool Submit(TextureRenderTask *trt) { return m_QueuedTextureRender.push(trt);}

	bool Initialize(uint8_t BufferIndex, RenderDevice *device);
	bool Finalize();

	uint8_t Index() const { return m_BufferIndex; }
	RenderDevice* GetDevice() const { return m_RenderDevice; }
private: 
	uint8_t m_BufferIndex;
	uint8_t padding8[3];
	RenderDevice *m_RenderDevice;
	Commands::CommandQueue m_ControllQueue;

	TextureRenderQueue m_QueuedTextureRender;
	Allocator_t m_Memory;
};

static_assert((sizeof(Frame) % 16) == 0, "Invalid size!");
//static_assert(std::is_pod<Frame>::value, "Must be a pod!");

} //namespace MoonGlare::Renderer 
