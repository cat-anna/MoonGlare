#pragma once

#include "aligned_ptr.hpp"
#include "command_queue.hpp"
#include "core/engine_time.hpp"
#include "memory/element_pool.hpp"
#include "memory/stack_allocator.hpp"
#include "renderer/device_types.hpp"
#include "renderer/render_target_interface.hpp"
#include "renderer/renderer_configuration.hpp"
#include "renderer/resources.hpp"
#include <bitset>
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

    void ReleaseResources(CommandQueueRef &command_queue) override;

    void Reset() override {
        memory.ClearAllocation();
        command_queue.ClearAllocation();
        dynamic_vao_pool.ClearAllocation();
        dynamic_buffer_pool.ClearAllocation();
        attached_shaders.reset();
    }

    uint8_t *AllocateMemory(size_t byte_count, bool zero = false) final {
        auto base = memory.Allocate<uint8_t>(byte_count);
        if (zero) {
            memset(base, 0, byte_count);
        }
        return base;
    }

    template <typename T>
    T *AllocElements(size_t count) {
        auto byte_count = sizeof(T) * count;
        auto base = memory.Allocate<uint8_t>(byte_count);
        return reinterpret_cast<T *>(base);
    }

    math::fvec2 BufferSize() const override;

    void AttachShader(ShaderHandle shader);

    ElementBuffer ReserveElements(const ElementReserve &config) override;
    void SubmitElements(const ElementBuffer &buffer, const ElementRenderRequest &request) override;

    void ExecuteQueue() { command_queue.Execute(); }

protected:
    iEngineTime *const engine_time;
    iResourceManager *const resource_manager;

    Device::FrameBufferHandle frame_buffer_handle = Device::kInvalidFrameBufferHandle;
    math::fvec2 frame_buffer_size;
    std::bitset<Configuration::Shader::kLimit> attached_shaders;

    BufferElementPool dynamic_buffer_pool;
    VAOElementPool dynamic_vao_pool;

    RenderTargetMemoryAllocator memory;
    CommandQueue command_queue;

protected:
    BaseRenderTarget(gsl::not_null<CommandQueue *> command_queue,
                     gsl::not_null<iEngineTime *> engine_time,
                     gsl::not_null<iResourceManager *> res_manager);

    void InitViewPortCommands();
};

} // namespace MoonGlare::Renderer
