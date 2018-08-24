#pragma once

#include "../../nfRenderer.h"
#include "../../Configuration.Renderer.h"

#include "ShaderBuilder.h"

#include <Foundation/iFileSystem.h>

namespace MoonGlare::Renderer::Resources::Shader {

class ShaderCodeLoader;

struct ShaderHandlerInterface {
    virtual ~ShaderHandlerInterface() {}
    virtual uint32_t InterfaceID() const = 0;

    virtual uint32_t UniformCount() = 0;
    virtual const char ** UniformName() = 0;

    virtual uint32_t SamplerCount() = 0;
    virtual const char ** SamplerName() = 0;
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

class ShaderResource final {
    using ConfRes = Configuration::Resources;
    using Conf = Configuration::Shader;
public:
    ShaderResource();
    ~ShaderResource();

    bool Initialize(ResourceManager* Owner, iFileSystem *fileSystem);
    bool Finalize();

    template<typename Descriptor_t>
    bool Load(ShaderResourceHandle<Descriptor_t> &out, const std::string &ShaderName) {
        assert(this);
        return LoadShader(out, ShaderName, ShaderHandlerInterfaceImpl<Descriptor_t>::Instace());
    }

    template<typename Descriptor_t>
    bool Load(ShaderResourceHandleBase &out, const std::string &ShaderName) {
        assert(this);
        return LoadShader(out, ShaderName, ShaderHandlerInterfaceImpl<Descriptor_t>::Instace());
    }

    template<typename Descriptor_t>
    ShaderBuilder<Descriptor_t> GetBuilder(Commands::CommandQueue &q, ShaderResourceHandleBase h) {
        assert(this);
        assert(h.m_TmpGuard == h.GuardValue);
        assert(h.m_Index < Conf::Limit);

        return ShaderBuilder<Descriptor_t> {
            &q,
                &m_ShaderUniform[h.m_Index],
                &m_ShaderHandle[h.m_Index],
                m_ResourceManager,
        };
    }

    template<typename Descriptor_t>
    ShaderBuilder<Descriptor_t> GetBuilder(Commands::CommandQueue &q, ShaderResourceHandle<Descriptor_t> h) {
        assert(this);
        assert(h.m_TmpGuard == h.GuardValue);
        assert(h.m_Index < Conf::Limit);

        return ShaderBuilder<Descriptor_t> {
            &q,
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
    std::unique_ptr<ShaderCodeLoader> shaderCodeLoader;

    bool LoadShader(ShaderResourceHandleBase &out, const std::string &ShaderName, ShaderHandlerInterface *ShaderIface);
    bool GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, uint32_t ifindex);

    bool ReleaseShader(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    bool GenerateLoadCommand(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    bool InitializeUniforms(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    bool InitializeSamplers(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
}; 

} //namespace MoonGlare::Renderer::Resources ::Shader
