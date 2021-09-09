
#pragma once

#include "async_loader.hpp"
#include "command_queue.hpp"
#include "renderer/device_types.hpp"
#include "renderer/renderer_configuration.hpp"
#include "renderer/resources.hpp"
#include "renderer/resources/shader_resource_interface.hpp"
#include "shader_loader_interface.hpp"

namespace MoonGlare::Renderer::Resources::Shader {
#if 0

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

#endif

class ShaderResource final : public iShaderResource {
public:
    ShaderResource(gsl::not_null<iAsyncLoader *> async_loader,
                   gsl::not_null<iReadOnlyFileSystem *> _file_system,
                   gsl::not_null<iShaderCodeLoader *> _shader_loader,
                   gsl::not_null<iContextResourceLoader *> _context_loader,
                   gsl::not_null<CommandQueue *> init_commands);
    ~ShaderResource();

    //iShaderResource
    void ReloadAllShaders() override;
    ShaderHandle LoadShader(const std::string &name) override;
    ShaderHandle LoadShader(FileResourceId resource_id) override;
    const ShaderVariables *GetShaderVariables(ShaderHandle handle) override;

    // bool Reload(const std::string &Name);
    // bool GenerateReload(Commands::CommandQueue &queue, StackAllocator& Memory, ShaderResourceHandleBase handle);
    // void Dump(Space::OFmtStream &o);
private:
    iAsyncLoader *const async_loader;
    iReadOnlyFileSystem *const file_system;
    iShaderCodeLoader *const shader_loader;
    iContextResourceLoader *const context_loader;
    // std::mutex data_lock; //?

    template <typename T>
    using DataArray = std::array<T, Configuration::Shader::kLimit + 1>;

    DataArray<std::atomic<bool>> shader_loaded;
    DataArray<std::atomic<bool>> shader_allocated;
    // DataArray<Device::ShaderHandle> shader_handle;
    // Array<Conf::UniformLocations> m_ShaderUniform;
    DataArray<ShaderVariables> shader_variables;
    DataArray<std::string> shader_base_name;
    // Array<ShaderHandlerInterface*> m_ShaderInterface;
    // const Configuration::Shader *shaderConfiguration = nullptr;

    std::vector<Device::ShaderHandle> standby_shaders;

    ShaderHandle LoadShaderByPath(const std::string &base_path);

    void ScheduleReload(Device::ShaderHandle device_handle);

    void LoadShaderCode(Device::ShaderHandle device_handle,
                        const iShaderCodeLoader::ShaderCode &code) const;

    void ReloadUniforms(Device::ShaderHandle device_handle);

    // bool GenerateReload(Commands::CommandQueue &queue, StackAllocator &Memory, uint32_t ifindex);
    // bool ReleaseShader(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    // bool GenerateLoadCommand(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    // bool InitializeSamplers(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);

    // static std::string GenerateGaussianDisc(size_t length, float baseRadius);
};

} // namespace MoonGlare::Renderer::Resources::Shader
