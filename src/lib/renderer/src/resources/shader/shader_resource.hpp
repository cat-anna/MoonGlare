
#pragma once

#include "async_loader.hpp"
#include "command_queue.hpp"
#include "memory/element_pool.hpp"
#include "renderer/device_types.hpp"
#include "renderer/renderer_configuration.hpp"
#include "renderer/resources.hpp"
#include "renderer/resources/shader_resource_interface.hpp"
#include "shader_loader_interface.hpp"
#include <gsl/gsl>

namespace MoonGlare::Renderer::Resources::Shader {

class ShaderResource final : public iShaderResource {
public:
    ShaderResource(gsl::not_null<iAsyncLoader *> async_loader,
                   gsl::not_null<iReadOnlyFileSystem *> _file_system,
                   gsl::not_null<iShaderCodeLoader *> _shader_loader,
                   gsl::not_null<iContextResourceLoader *> _context_loader,
                   gsl::not_null<CommandQueue *> init_commands);
    ~ShaderResource();

    using ShaderResourceHandlePool =
        Memory::HandleElementPool<Device::ShaderHandle, Configuration::Shader::kLimit, GLint,
                                  Device::kInvalidShaderHandle>;

    void ReleaseResources(CommandQueue *command_queue); //TODO

    //iShaderResource
    void ReloadAllShaders() override;
    ShaderHandle LoadShader(const std::string &name) override;
    ResourceHandle LoadShaderResource(FileResourceId resource_id) override;
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

    ShaderResourceHandlePool standby_shaders_pool;

    ShaderHandle LoadShaderByPath(const std::string &base_path);

    void ScheduleReload(Device::ShaderHandle device_handle);

    void LoadShaderCode(CommandQueue *queue, Device::ShaderHandle device_handle,
                        const iShaderCodeLoader::ShaderCode &code) const;

    void ReloadUniforms(CommandQueue *queue, Device::ShaderHandle device_handle);

    // bool GenerateReload(Commands::CommandQueue &queue, StackAllocator &Memory, uint32_t ifindex);
    // bool ReleaseShader(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    // bool GenerateLoadCommand(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);
    // bool InitializeSamplers(Commands::CommandQueue &q, StackAllocator& Memory, uint32_t ifindex);

    // static std::string GenerateGaussianDisc(size_t length, float baseRadius);
};

} // namespace MoonGlare::Renderer::Resources::Shader
