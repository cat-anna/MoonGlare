#if 0

#pragma once

#include "../../Configuration.Renderer.h"
#include "../../nfRenderer.h"

#include "../../Commands/CommandQueue.h"
#include "../../Commands/OpenGL/ShaderCommands.h"
#include "../../Commands/OpenGL/TextureCommands.h"

#include "../MaterialManager.h"

#include "../../Material.h"

namespace MoonGlare::Renderer::Resources::Shader {

template<typename Descriptor>
struct ShaderBuilder {
//	static_assert(std::is_trivial<ShaderBuilder>::value, "must be trivial!");
//	static_assert((sizeof(ShaderBuilder) % 16) == 0, "Invalid size!");

    using Conf = Configuration::Shader;
    using Uniform = typename Descriptor::Uniform;
    using Sampler = typename Descriptor::Sampler;

    void SelfTest() {
        assert(this);
        assert(m_UniformsPtr);
        assert(m_HandlePtr);
        assert(m_ResourceManager);
    }

    void Bind(Commands::CommandKey key = Commands::CommandKey()) {
        SelfTest();
        m_Queue->MakeCommandKey<Commands::ShaderResourceBind>(key, m_HandlePtr);
    }
    void UnBind(Commands::CommandKey key = Commands::CommandKey()) {
        SelfTest();
        m_Queue->MakeCommandKey<Commands::ShaderBind>(key, InvalidShaderHandle);
    }

    template<Uniform u, typename T>
    void Set(const T &t, Commands::CommandKey key = Commands::CommandKey()) {
        auto arg = m_Queue->PushCommand<Commands::ShaderResourcSetUniform<T>>(key);
        arg->m_Handle = &(*m_UniformsPtr)[static_cast<uint32_t>(u)];
        arg->m_Value = t;
        if (*arg->m_Handle == -1) {
//            int i = 0;
        }
    }
    template<typename T>
    void Set(const char Name[], const T &t, Commands::CommandKey key = Commands::CommandKey()) {
        auto arg = m_Queue->PushCommand<Commands::ShaderResourcSetNamedUniform<T>>(key);
        arg->shaderHandle = m_HandlePtr;
        arg->value = t;
        arg->name = Name;
    }

    template<Sampler SamplerUnit>
    void Set(TextureResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        auto texres = m_Queue->PushCommand<Renderer::Commands::Texture2DResourceBindUnit>(key);
        texres->m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
        texres->m_HandlePtr = m_ResourceManager->GetTextureResource().GetHandleArrayBase() + h.index;
    }
    template<Sampler SamplerUnit>
    void Set(Device::TextureHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        auto texres = m_Queue->PushCommand<Renderer::Commands::Texture2DBindUnit>(key);
        texres->m_Texture = h;
        texres->m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
    }

    template<Sampler SamplerUnit>
    void SetCubeSampler(Device::TextureHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        auto texres = m_Queue->PushCommand<Renderer::Commands::TextureCubeBindUnit>(key);
        texres->m_Texture = h;
        texres->m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
    }
    template<Sampler SamplerUnit>
    void Set2DSampler(Device::TextureHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        auto texres = m_Queue->PushCommand<Renderer::Commands::Texture2DBindUnit>(key);
        texres->m_Texture = h;
        texres->m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
    }

    void SetMaterial(MaterialResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        auto *mat = h.deviceHandle;
            //this->m_ResourceManager->GetMaterialManager().GetMaterial(h);
        if (!mat) {
            //TODO: do sth
            return;
        }
        SetMaterial(*mat, key);
    }

    void SetMaterial(Material& mat, Commands::CommandKey key = Commands::CommandKey()) {
        auto &cmd = *m_Queue->PushCommand<Renderer::Commands::ShaderBindMaterialResource>(key);
        cmd.diffuseColor = mat.diffuseColor;
        cmd.specularColor = mat.specularColor;
        cmd.emissiveColor = mat.emissiveColor;
        cmd.shiness = mat.shiness;
        cmd.mapUnit = { (uint8_t)Sampler::DiffuseMap,(uint8_t)Sampler::NormalMap,(uint8_t)Sampler::ShinessMap,(uint8_t)Sampler::SpecularMap, };
        for (size_t i = 0; i < cmd.mapHandle.size(); ++i) {
            auto ptr = mat.mapTexture[(Material::MapType)i].deviceHandle;
            cmd.mapHandle[(Material::MapType)i] = ptr ? *ptr : Device::InvalidTextureHandle;
        }
        cmd.diffuseColorLocation = (*m_UniformsPtr)[static_cast<uint32_t>(Uniform::DiffuseColor)];
        cmd.specularColorLocation = (*m_UniformsPtr)[static_cast<uint32_t>(Uniform::SpecularColor)];
        cmd.emissiveColorLocation = (*m_UniformsPtr)[static_cast<uint32_t>(Uniform::EmissiveColor)];
        cmd.shinessLocation = (*m_UniformsPtr)[static_cast<uint32_t>(Uniform::Shiness)];
        cmd.useNormalMapLocation = (*m_UniformsPtr)[static_cast<uint32_t>(Uniform::UseNormalMap)];
    }

    Commands::CommandQueue *m_Queue;
    Conf::UniformLocations *m_UniformsPtr;
    Device::ShaderHandle* m_HandlePtr;
    ResourceManager* m_ResourceManager;
};

#if 0
template<typename Descriptor>
struct
    //alignas(16)
    ShaderExecutor {
    //	static_assert(std::is_trivial<ShaderBuilder>::value, "must be trivial!");
    //	static_assert((sizeof(ShaderBuilder) % 16) == 0, "Invalid size!");
    using Conf = Configuration::Shader;
    using Uniform = typename Descriptor::Uniform;
    using Sampler = typename Descriptor::Sampler;

    void SelfTest() {
        assert(this);
        assert(m_UniformsPtr);
        assert(m_HandlePtr);
        assert(m_ResourceManager);
    }

    void Bind() {
        SelfTest();
        Run<Commands::ShaderResourceBind>(m_HandlePtr);
    }
    void UnBind() {
        SelfTest();
        Run<Commands::ShaderBind>(InvalidShaderHandle);
    }

    template<Uniform u, typename T>
    void Set(const T &t, Commands::CommandKey key = Commands::CommandKey()) {
        SelfTest();
        (void)key;
        Run<Commands::ShaderResourcSetUniform<T>>(&(*m_UniformsPtr)[static_cast<uint32_t>(u)], t);
    }

    template<Sampler SamplerUnit>
    void Set(TextureResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        Renderer::Commands::Texture2DResourceBindUnit::Argument arg;
        arg.m_Handle = h;
        arg.m_HandleArray = m_ResourceManager->GetTextureResource().GetHandleArrayBase();
        arg.m_UnitIndex = static_cast<uint16_t>(SamplerUnit);
        Run<Renderer::Commands::Texture2DResourceBindUnit>(arg);
    }

    void SetMaterial(MaterialResourceHandle h, Commands::CommandKey key = Commands::CommandKey()) {
        auto *mat = this->m_ResourceManager->GetMaterialManager().GetMaterial(h);
        if (!mat) {
            //TODO: do sth
            return;
        }
        SetMaterial(*mat, key);
    }

    void SetMaterial(Material& mat, Commands::CommandKey key = Commands::CommandKey()) {
        Set<Sampler::DiffuseMap>(mat.m_DiffuseMap, key);
        Set<Uniform::DiffuseColor>(mat.m_DiffuseColor, key);
    }

    Conf::UniformLocations *m_UniformsPtr;
    Device::ShaderHandle* m_HandlePtr;
    ResourceManager* m_ResourceManager;
private:
    template<typename T, typename ... ARG>
    void Run(ARG && ...arg) {
        T::Argument a{ std::forward<ARG>(arg)... };
        a.Execute(&a);
    }
};

#endif

} //namespace MoonGlare::Renderer::Resources

#endif
