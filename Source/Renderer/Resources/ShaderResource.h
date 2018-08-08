/*
  * Generated by cppsrc.sh
  * On 2017-02-22 19:09:53,00
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "../nfRenderer.h"
#include "../Configuration.Renderer.h"

#include <Assets/AssetLoaderInterface.h>
#include "Builder/ShaderBuilder.h"

namespace MoonGlare::Renderer::Resources {

struct ShaderHandlerInterface {
    virtual ~ShaderHandlerInterface() {}
    virtual uint32_t InterfaceID() const = 0;

    virtual uint32_t UniformCount() = 0;
    virtual const char ** UniformName() = 0;

    virtual const char ** SamplerName() = 0;
    virtual uint32_t SamplerCount() = 0;
protected:
    using Conf = Configuration::Shader;
    static uint32_t AllocateID() {
        if (s_InterfaceIndexAlloc >= Conf::Limit) {
            throw "Shader interface limit is too small!";
        }
        return s_InterfaceIndexAlloc++; 
    }
private:
    static uint32_t s_InterfaceIndexAlloc;
};

template<typename desciptor>
struct ShaderHandlerInterfaceImpl : public ShaderHandlerInterface {
    static_assert(static_cast<uint32_t>(desciptor::Uniform::MaxValue) <= Conf::UniformLimit, "Invalid count!");
    virtual uint32_t InterfaceID() const override  {
        return s_InterfaceIndex;
    }
    
    virtual uint32_t UniformCount() override {
        return static_cast<uint32_t>(desciptor::Uniform::MaxValue);
    }
    virtual const char ** UniformName() {
        static std::array<const char*, static_cast<uint32_t>(desciptor::Uniform::MaxValue)> names;
        static bool init = false;

        if (!init) {
            for (auto i = 0u, j = UniformCount(); i < j; ++i) {
                names[i] = desciptor::GetName(static_cast<desciptor::Uniform>(i));
            }
            init = true;
        }

        return &names[0];
    }
    virtual uint32_t SamplerCount() override {
        return static_cast<uint32_t>(desciptor::Sampler::MaxValue);
    }
    virtual const char **SamplerName() {
        static std::array<const char*, static_cast<uint32_t>(desciptor::Sampler::MaxValue)> names;
        static bool init = false;

        if (!init) {
            for (auto i = 0u, j = SamplerCount(); i < j; ++i) {
                names[i] = desciptor::GetSamplerName(static_cast<desciptor::Sampler>(i));
            }
            init = true;
        }

        return &names[0];
    }
    
    static ShaderHandlerInterface* Instace() {
        static ShaderHandlerInterfaceImpl impl;
        return &impl;
    }
private:
    const static uint32_t s_InterfaceIndex;
};

template<typename desciptor>
const uint32_t ShaderHandlerInterfaceImpl<desciptor>::s_InterfaceIndex = ShaderHandlerInterface::AllocateID();

class 
    //alignas(16) 
    ShaderResource final {
    using ThisClass = ShaderResource;
    using ConfRes = Configuration::Resources;
    using Conf = Configuration::Shader;
public:
    bool Initialize(ResourceManager* Owner, Asset::ShaderCodeLoader *CodeLoader);
    bool Finalize();

    template<typename Descriptor_t>
    bool Load(ShaderResourceHandle<Descriptor_t> &out, const std::string &ShaderName) {
        RendererAssert(this);
        return LoadShader(out, ShaderName, ShaderHandlerInterfaceImpl<Descriptor_t>::Instace());
    }

    template<typename Descriptor_t>
    bool Load(ShaderResourceHandleBase &out, const std::string &ShaderName) {
        RendererAssert(this);
        return LoadShader(out, ShaderName, ShaderHandlerInterfaceImpl<Descriptor_t>::Instace());
    }

    template<typename Descriptor_t>
    ShaderBuilder<Descriptor_t> GetBuilder(Commands::CommandQueue &q, ShaderResourceHandleBase h) {
        RendererAssert(this);

        RendererAssert(h.m_TmpGuard == h.GuardValue);
        RendererAssert(h.m_Index < Conf::Limit);

        return ShaderBuilder<Descriptor_t> {
            &q,
                &m_ShaderUniform[h.m_Index],
                &m_ShaderHandle[h.m_Index],
                m_ResourceManager,
        };
    }

    template<typename Descriptor_t>
    ShaderBuilder<Descriptor_t> GetBuilder(Commands::CommandQueue &q, ShaderResourceHandle<Descriptor_t> h) {
        RendererAssert(this);

        RendererAssert(h.m_TmpGuard == h.GuardValue);
        RendererAssert(h.m_Index < Conf::Limit);

        return ShaderBuilder<Descriptor_t> {
            &q,
            &m_ShaderUniform[h.m_Index],
            &m_ShaderHandle[h.m_Index],
            m_ResourceManager,
        };
    }

    template<typename Descriptor_t>
    ShaderExecutor<Descriptor_t> GetExecutor(ShaderResourceHandle<Descriptor_t> h) {
        RendererAssert(this);

        RendererAssert(h.m_TmpGuard == h.GuardValue);
        RendererAssert(h.m_Index < Conf::Limit);

        return ShaderExecutor<Descriptor_t> {
            &m_ShaderUniform[h.m_Index],
            &m_ShaderHandle[h.m_Index],
            m_ResourceManager,
        };
    }

    void ReloadAll();
    bool Reload(const std::string &Name);
    bool GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, ShaderResourceHandleBase handle);
    void Dump(Space::OFmtStream &o);
private:
    template<typename T>
    using Array = std::array<T, Conf::Limit>;

    Array<std::atomic<bool>> m_ShaderLoaded;
    Array<Device::ShaderHandle> m_ShaderHandle;
    Array<Conf::UniformLocations> m_ShaderUniform;
    Array<std::string> m_ShaderName;
    Array<ShaderHandlerInterface*> m_ShaderInterface;
    ResourceManager *m_ResourceManager = nullptr;
    Asset::ShaderCodeLoader *m_ShaderCodeLoader = nullptr;

    std::string m_ShaderConfigurationDefs;
    void *_padding0;
    void *_padding1;
    void *_padding2;

    bool LoadShader(ShaderResourceHandleBase &out, const std::string &ShaderName, ShaderHandlerInterface *ShaderIface);
    bool GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, uint32_t ifindex);

    bool ReleaseShader(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    bool GenerateLoadCommand(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    bool InitializeUniforms(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    bool InitializeSamplers(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
}; 

//static_assert((sizeof(ShaderResource) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
