#pragma once

#include "aligned_ptr.hpp"
#include "command_queue.hpp"
#include "memory/element_pool.hpp"
#include "memory/stack_allocator.hpp"
#include "renderer/device_types.hpp"
#include "renderer/render_target_interface.hpp"
#include "renderer/renderer_configuration.hpp"
#include <memory>

namespace MoonGlare::Renderer {

class alignas(16) BaseRenderTarget : public iRenderTarget {
public:
    using RenderTargetMemoryAllocator =
        Memory::StaticStackAllocator<Configuration::RenderTarget::kMemoryPoolSize>;

    using BufferElementPool =
        Memory::HandleElementPool<Device::BufferHandle,
                                  Configuration::RenderTarget::kDynamicBufferPoolSize, GLint,
                                  Device::kInvalidBufferHandle>;

    using VAOElementPool =
        Memory::HandleElementPool<Device::VaoHandle,
                                  Configuration::RenderTarget::kDynamicVaoPoolSize, GLint,
                                  Device::kInvalidVaoHandle>;

public:
    ~BaseRenderTarget() override = default;

    void ReleaseResources(CommandQueue *command_queue) override;

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

    math::ivec2 BufferSize() const override;

    //TODO
    // private:

    Device::FrameBufferHandle frame_buffer_handle = Device::kInvalidFrameBufferHandle;
    math::ivec2 frame_buffer_size;

    BufferElementPool dynamic_buffer_pool;
    VAOElementPool dynamic_vao_pool;

    RenderTargetMemoryAllocator memory;
    CommandQueue command_queue;

protected:
    BaseRenderTarget(CommandQueue *command_queue);

    void InitViewPortCommands();
};

} // namespace MoonGlare::Renderer
