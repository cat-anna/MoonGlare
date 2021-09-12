#pragma once

#include "aligned_ptr.hpp"
#include "command_queue.hpp"
#include "memory/element_pool.hpp"
#include "memory/stack_allocator.hpp"
#include "renderer/device_types.hpp"
#include "renderer/frame_buffer_interface.hpp"
#include "renderer/renderer_configuration.hpp"
#include <memory>

namespace MoonGlare::Renderer {

class alignas(16) FrameBuffer : public iFrameBuffer {
public:
    using FrameBufferMemoryAllocator =
        Memory::StaticStackAllocator<Configuration::FrameBuffer::kMemoryPoolSize>;

    using BufferElementPool =
        Memory::HandleElementPool<Device::BufferHandle,
                                  Configuration::FrameBuffer::kDynamicBufferPoolSize, GLint,
                                  Device::kInvalidBufferHandle>;

    using VAOElementPool =
        Memory::HandleElementPool<Device::VaoHandle,
                                  Configuration::FrameBuffer::kDynamicVaoPoolSize, GLint,
                                  Device::kInvalidVaoHandle>;

    FrameBuffer(CommandQueue *command_queue);

    void ReleaseResources(CommandQueue *command_queue);

    void Reset() override {
        memory.ClearAllocation();
        command_queue.ClearAllocation();
        dynamic_vao_pool.ClearAllocation();
        dynamic_buffer_pool.ClearAllocation();
    }

    uint8_t *AllocateMemory(size_t byte_count, bool zero = false) override {
        auto base = memory.Allocate<uint8_t>(byte_count);
        if (zero) {
            memset(base, 0, byte_count);
        }
        return base;
    }

    //TODO
    // private:

    BufferElementPool dynamic_buffer_pool;
    VAOElementPool dynamic_vao_pool;

    FrameBufferMemoryAllocator memory;
    CommandQueue command_queue;
};

} // namespace MoonGlare::Renderer
