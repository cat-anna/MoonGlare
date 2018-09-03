#pragma once

#include "Configuration.Renderer.h"
#include "Commands/CommandQueueLayers.h"

#include "Resources/ShadowMap.h"

namespace MoonGlare::Renderer {

class alignas(16) Frame final {
    using Conf = Configuration::FrameBuffer;
    using ConfRes = Configuration::FrameResourceStorage;
public:
    using TextureRenderQueue = Space::Container::StaticVector<TextureRenderTask*, Configuration::TextureRenderTask::Limit>;
    using SubQueue = Commands::CommandQueue;
    using SubQueueTable = Space::Container::StaticVector<SubQueue, Conf::SubQueueCount>;

    template<typename T>
    using ByteArray = Space::Memory::StaticMemory<T, Conf::MemorySize>;
    using Allocator_t = Space::Memory::StackAllocator<ByteArray>;
    using CommandLayers = Commands::CommandQueueLayers<Conf::Layer>;

    CommandLayers& GetCommandLayers() { 
        assert(this); 
        return m_CommandLayers; 
    }
    CommandLayers::Queue& GetControllCommandQueue() { 
        assert(this); 
        return m_CommandLayers.Get<Conf::Layer::Controll>(); 
    }

    TextureRenderQueue& GetTextureRenderQueue() {
        assert(this);
        return m_QueuedTextureRender;
    }
    Allocator_t& GetMemory() {
        assert(this);
        return m_Memory;
    }

    SubQueue* AllocateSubQueue() {
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

    template<typename T>
    bool Allocate(Frame *frame, T &resH) {
        assert(this);
        auto &pool = GetPool(resH);
        if (pool.Allocate(resH))
            return true;
        if (!pool.HasStorage())
            return false;
        T h{ };
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

    template<typename T>
    bool AllocateFrameResource(T &resH) {
        assert(this);
        return Allocate(this, resH);
    }
    ShadowMap* AllocatePlaneShadowMap() {
        assert(this);
        if (!flags.shadowsEnabled) {
            //TODO: this is workaround, this should be done at higher level
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
    ShadowMap* AllocateCubeShadowMap() {
        assert(this);
        if (!flags.shadowsEnabled) {
            //TODO: this is workaround, this should be done at higher level
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
    RenderDevice* GetDevice() const { return m_RenderDevice; }
    Resources::ResourceManager* GetResourceManager() const { return m_ResourceManager; }
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
    ::Space::Container::StaticAllocationPool<VAOResourceHandle, ConfRes::VAOLimit> m_VAOs;
    ::Space::Container::StaticVector<ShadowMap, ConfRes::PlaneShadowMapLimit> planeShadowMaps;
    ::Space::Container::StaticVector<ShadowMap, ConfRes::CubeShadowMapLimit> cubeShadowMaps;

    Allocator_t m_Memory;

    //auto& GetPool(TextureResourceHandle) {
    //    assert(this);
    //    return m_Textures;
    //}
    auto& GetPool(VAOResourceHandle) {
        assert(this);
        return m_VAOs;
    }

};

//static_assert((sizeof(Frame) % 16) == 0, "Invalid size!");
//static_assert(std::is_pod<Frame>::value, "Must be a pod!");

} //namespace MoonGlare::Renderer 
