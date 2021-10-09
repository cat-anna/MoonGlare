#pragma once

#include "facade.hpp"
#include "math/vector.hpp"
#include "types.hpp"
#include <cassert>
#include <cstdint>

namespace MoonGlare::Renderer {

class iRenderTarget {
public:
    virtual ~iRenderTarget() = default;

    virtual void Reset() = 0;
    virtual void ReleaseResources(CommandQueueRef &command_queue) = 0;

    virtual uint8_t *AllocateMemory(size_t byte_count, bool zero = false) = 0;

    virtual math::fvec2 BufferSize() const = 0;

    // virtual void AttachShader(ShaderHandle shader) = 0;
    // virtual void AttachCamera(math::fvec4 *camera_matrix) = 0;

    template <typename T>
    T *AllocateElements(size_t count, bool zero = false) {
        return reinterpret_cast<T *>(AllocateMemory(count * sizeof(T), zero));
    }

    //  *** Element render ***

    struct ElementReserve {
        size_t index_count;
        size_t vertex_count;
        size_t texture0_count;
    };
    struct ElementBuffer {
        const ElementReserve reserve;
        RenderElementIndexType *const index_buffer = nullptr;
        math::fvec3 *const vertex_buffer = nullptr;
        math::fvec2 *const texture0_buffer = nullptr;
    };
    struct ElementRenderRequest {
        math::fmat4 position_matrix;
        GLenum element_mode;
        size_t index_count;
        //TODO
        // int material_handle
        ShaderHandle shader_handle;
        TextureHandle texture_handle = kInvalidTextureHandle;
        //layer?
    };

    virtual ElementBuffer ReserveElements(const ElementReserve &config) = 0;
    virtual void SubmitElements(const ElementBuffer &buffer,
                                const ElementRenderRequest &request) = 0;
};

class RenderTargetProxy final : public iRenderTarget {
public:
    RenderTargetProxy(iRenderTarget *target = nullptr) : target(target){};

    void SetTarget(iRenderTarget *_target) { target = _target; }

    void Reset() override {
        assert(target);
        target->Reset();
    }
    void ReleaseResources(CommandQueueRef &command_queue) override {
        assert(target);
        target->ReleaseResources(command_queue);
    }

    math::fvec2 BufferSize() const override {
        assert(target);
        return target->BufferSize();
    }
    uint8_t *AllocateMemory(size_t byte_count, bool zero = false) override {
        assert(target);
        return target->AllocateMemory(byte_count, zero);
    }

    ElementBuffer ReserveElements(const ElementReserve &config) override {
        assert(target);
        return target->ReserveElements(config);
    }
    void SubmitElements(const ElementBuffer &buffer, const ElementRenderRequest &request) override {
        assert(target);
        target->SubmitElements(buffer, request);
    }

private:
    iRenderTarget *target;
};

#if 0
class alignas(16) Frame final {
    using ConfRes = Configuration::FrameResourceStorage;

public:
    using TextureRenderQueue = Memory::StaticVector<TextureRenderTask *, Configuration::TextureRenderTask::Limit>;
    using SubQueue = Commands::CommandQueue;
    using SubQueueTable = Memory::StaticVector<SubQueue, Conf::SubQueueCount>;

    template <typename T> using ByteArray = Memory::StaticMemory<T, Conf::MemorySize>;
    using Allocator_t = Memory::StackAllocator<ByteArray>;
    using CommandLayers = Commands::CommandQueueLayers<Conf::Layer>;

    CommandLayers &GetCommandLayers() {
        assert(this);
        return m_CommandLayers;
    }
    CommandLayers::Queue &GetControllCommandQueue() {
        assert(this);
        return m_CommandLayers.Get<Conf::Layer::Controll>();
    }

    TextureRenderQueue &GetTextureRenderQueue() {
        assert(this);
        return m_QueuedTextureRender;
    }
    Allocator_t &GetMemory() {
        assert(this);
        return m_Memory;
    }

    SubQueue *AllocateSubQueue() {
        assert(this);
        auto q = m_SubQueueTable.Allocate();
        if (q)
            q->ClearAllocation();
        return q;
    }

    void BeginFrame(uint64_t index);
    void EndFrame();

    bool Submit(TextureRenderTask *trt);
    bool Submit(SubQueue *q, Conf::Layer Layer, Commands::CommandKey Key = Commands::CommandKey());

    template <typename T> bool Allocate(Frame *frame, T &resH) {
        assert(this);
        auto &pool = GetPool(resH);
        if (pool.Allocate(resH))
            return true;
        if (!pool.HasStorage())
            return false;
        T h{};
        if (!AllocateResource(h))
            return false;
        if (!pool.PushAndAlloc(h, resH)) {
            ReleaseResource(h);
            return false;
        }
        return true;
    }

    void ReleaseResource(TextureResourceHandle &texres);
    void ReleaseResource(VAOResourceHandle &vaores);
    bool AllocateResource(TextureResourceHandle &resH);
    bool AllocateResource(VAOResourceHandle &resH);

    template <typename T> bool AllocateFrameResource(T &resH) {
        assert(this);
        return Allocate(this, resH);
    }
    ShadowMap *AllocatePlaneShadowMap() {
        assert(this);
        if (!flags.shadowsEnabled) {
            // TODO: this is workaround, this should be done at higher level
            return nullptr;
        }
        auto *ptr = planeShadowMaps.Allocate();
        if (!ptr) {
            AddLog(Warning, "Out of PlaneShadowMaps");
            return nullptr;
        }
        if (!ptr->Valid()) {
            ptr->InitPlane(GetControllCommandQueue(), configuration->shadow);
        }
        return ptr;
    }
    ShadowMap *AllocateCubeShadowMap() {
        assert(this);
        if (!flags.shadowsEnabled) {
            // TODO: this is workaround, this should be done at higher level
            return nullptr;
        }
        auto *ptr = cubeShadowMaps.Allocate();
        if (!ptr) {
            AddLog(Warning, "Out of PlaneShadowMaps");
            return nullptr;
        }
        if (!ptr->Valid()) {
            ptr->InitCube(GetControllCommandQueue(), configuration->shadow);
        }
        return ptr;
    }

    bool Initialize(uint8_t BufferIndex, RenderDevice *device, RendererFacade *rfacade);
    bool Finalize();

    uint8_t Index() const { return m_BufferIndex; }
    RenderDevice *GetDevice() const { return m_RenderDevice; }
    Resources::ResourceManager *GetResourceManager() const { return m_ResourceManager; }
    uint64_t FrameIndex() const { return frameIndex; }

private:
    const Configuration::RuntimeConfiguration *configuration;
    uint64_t frameIndex;
    uint8_t m_BufferIndex;
    struct {
        bool shadowsEnabled : 1;
    } flags;
    RenderDevice *m_RenderDevice;
    Resources::ResourceManager *m_ResourceManager;

    CommandLayers m_CommandLayers;
    SubQueueTable m_SubQueueTable;

    TextureRenderQueue m_QueuedTextureRender;

    //::Space::Container::StaticAllocationPool<TextureResourceHandle, Conf::TextureLimit> m_Textures;
    Memory::StaticVector<ShadowMap, ConfRes::PlaneShadowMapLimit> planeShadowMaps;
    Memory::StaticVector<ShadowMap, ConfRes::CubeShadowMapLimit> cubeShadowMaps;

    Allocator_t m_Memory;

    // auto& GetPool(TextureResourceHandle) {
    //    assert(this);
    //    return m_Textures;
    //}
    auto &GetPool(VAOResourceHandle) {
        assert(this);
        return m_VAOs;
    }
};
#endif

// static_assert((sizeof(Frame) % 16) == 0, "Invalid size!");
// static_assert(std::is_pod<Frame>::value, "Must be a pod!");

} // namespace MoonGlare::Renderer
