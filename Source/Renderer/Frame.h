/*
  * Generated by cppsrc.sh
  * On 2017-02-10 15:35:25,52
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "Configuration.Renderer.h"
#include "Commands/CommandQueueLayers.h"

#include "Resources/PlaneShadowMap.h"

namespace MoonGlare::Renderer {

class FrameResourceStorage {
    using Conf = Configuration::FrameResourceStorage;
public:
    void Clear() {
        RendererAssert(this);
        m_Textures.ClearAllocation();
        m_VAOs.ClearAllocation();
        planeShadowMaps.ClearAllocation();
    }

    template<typename T>
    bool Allocate(Frame *frame, T &resH) {
        RendererAssert(this);
        auto &pool = GetPool(resH);
        if (pool.Allocate(resH))
            return true;
        if (!pool.HasStorage())
            return false;
        T h{ 0 };
        if (!frame->AllocateResource(h))
            return false;
        if (!pool.PushAndAlloc(h, resH)) {
            frame->ReleaseResource(h);
            return false;
        }
        return true;
    }

    //TODO: this is resource leak!
    PlaneShadowMap* AllocPlaneShadowMap(Frame *frame);

    void Initialize(const Configuration::RuntimeConfiguration *conf);
private:
    auto& GetPool(TextureResourceHandle) {
        RendererAssert(this);
        return m_Textures;
    }
    auto& GetPool(VAOResourceHandle) {
        RendererAssert(this);
        return m_VAOs;
    }

    ::Space::Container::StaticAllocationPool<TextureResourceHandle, Conf::TextureLimit> m_Textures;
    uint32_t padding1[2];
    ::Space::Container::StaticAllocationPool<VAOResourceHandle, Conf::VAOLimit> m_VAOs;
    uint32_t padding2[2];
    ::Space::Container::StaticVector<PlaneShadowMap, Conf::PlaneShadowMapLimit> planeShadowMaps;
};

//TODO: static_assert((sizeof(FrameResourceStorage) % 16) == 0, "Invalid size!");

class alignas(16) Frame final {
    using Conf = Configuration::FrameBuffer;
    using ConfCtx = Configuration::Context;
public:
    using TextureRenderQueue = Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit>;
    using SubQueue = Commands::CommandQueue;
    using SubQueueTable = Space::Container::StaticVector<SubQueue, Conf::SubQueueCount>;

    template<typename T>
    using ByteArray = Space::Memory::StaticMemory<T, Conf::MemorySize>;
    using Allocator_t = Space::Memory::StackAllocator<ByteArray>;
    using CommandLayers = Commands::CommandQueueLayers<Conf::Layer>;
    using WindowLayers = Commands::CommandQueueLayers<ConfCtx::Window>;

    CommandLayers& GetCommandLayers() { 
        RendererAssert(this); 
        return m_CommandLayers; 
    }
    CommandLayers::Queue& GetControllCommandQueue() { 
        RendererAssert(this); 
        return m_CommandLayers.Get<Conf::Layer::Controll>(); 
    }

    WindowLayers& GetWindowLayers() { 
        RendererAssert(this); 
        return m_WindowLayers; 
    }
    WindowLayers::Queue& GetFirstWindowLayer() {
        RendererAssert(this);
        return m_WindowLayers.Get<ConfCtx::Window::First>();
    }

    TextureRenderQueue& GetTextureRenderQueue() {
        RendererAssert(this);
        return m_QueuedTextureRender;
    }
    Allocator_t& GetMemory() {
        RendererAssert(this);
        return m_Memory;
    }

    SubQueue* AllocateSubQueue() {
        RendererAssert(this);
        auto q = m_SubQueueTable.Allocate();
        if (q)
            q->ClearAllocation();
        return q;
    }

    void BeginFrame(uint64_t index);
    void EndFrame();

    bool Submit(TextureRenderTask *trt);
    bool Submit(SubQueue *q, ConfCtx::Window WindowLayer, Commands::CommandKey Key = Commands::CommandKey());
    bool Submit(SubQueue *q, Conf::Layer Layer, Commands::CommandKey Key = Commands::CommandKey());

    template<typename T>
    void ReleaseResource(T &resH) {
        RendererAssert(this);
        return GetResourceManager()->Release(this, resH);
    }
    template<typename T>
    bool AllocateResource(T &resH) {
        RendererAssert(this);
        return GetResourceManager()->Allocate(this, resH);
    }

    template<typename T>
    bool AllocateFrameResource(T &resH) {
        RendererAssert(this);
        return m_FrameResourceStorage.Allocate(this, resH);
    }
    PlaneShadowMap* AllocatePlaneShadowMap() {
        RendererAssert(this);
        if (!flags.shadowsEnabled) {
            //TODO: this is workaround, this should be done at higher level
            return nullptr;
        }
        return m_FrameResourceStorage.AllocPlaneShadowMap(this);
    }

    bool Initialize(uint8_t BufferIndex, RenderDevice *device, RendererFacade *rfacade);
    bool Finalize();

    uint8_t Index() const { return m_BufferIndex; }
    RenderDevice* GetDevice() const { return m_RenderDevice; }
    Resources::ResourceManager* GetResourceManager() const { return m_ResourceManager; }
    uint64_t FrameIndex() const { return frameIndex; }
private:
    uint8_t m_BufferIndex;
    uint8_t padding8[2];
    struct {
        bool shadowsEnabled : 1;
    } flags;
    RenderDevice *m_RenderDevice;
    Resources::ResourceManager *m_ResourceManager;
    void *paddingptr;

    CommandLayers m_CommandLayers;
    WindowLayers m_WindowLayers;
    SubQueueTable m_SubQueueTable;

    TextureRenderQueue m_QueuedTextureRender;
    FrameResourceStorage m_FrameResourceStorage;
    Allocator_t m_Memory;

    uint64_t frameIndex;
};

static_assert((sizeof(Frame) % 16) == 0, "Invalid size!");
//static_assert(std::is_pod<Frame>::value, "Must be a pod!");

} //namespace MoonGlare::Renderer 
