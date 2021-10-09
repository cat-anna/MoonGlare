#include "async_loader.hpp"
#include "build_configuration.hpp"
#include "command_queue.hpp"
#include "renderer/resources.hpp"
#include "resources/mesh/mesh_resource.hpp"
#include "resources/shader/preprocessor.hpp"
#include "resources/shader/shader_loader.hpp"
#include "resources/shader/shader_resource.hpp"
#include "resources/texture/free_image_loader.hpp"
#include "resources/texture/texture_resource.hpp"
#include <memory>

namespace MoonGlare::Renderer {

class ResourceManager final : public iResourceManager {
public:
    ResourceManager(gsl::not_null<iAsyncLoader *> async_loader,

                    gsl::not_null<iReadOnlyFileSystem *> file_system,
                    gsl::not_null<CommandQueue *> init_queue)
        : res_loader_proxy{nullptr}, shader_file_cache(file_system),
          shader_code_loader(async_loader, file_system,
                             [this]() { return CreateShaderPreprocessor(); }),
          shader_resource(async_loader, file_system, &shader_code_loader, &res_loader_proxy,
                          init_queue),
          free_image_loader(async_loader, &res_loader_proxy, &config.texture),
          texture_resource(&free_image_loader, init_queue) {
        //
    }
    ~ResourceManager() override = default;

    std::unique_ptr<Resources::Shader::iSharderPreprocesor> CreateShaderPreprocessor() {
        if constexpr (kDisableCaches) {
            shader_file_cache.Clear();
        }
        return std::make_unique<Resources::Shader::Preprocessor>(&shader_file_cache);
    }

    void SetResourceLoader(iContextResourceLoader *loader) { res_loader_proxy.SetTarget(loader); }

    //iRuntimeResourceLoader
    //Handled by double overrides

    // clang-format off

    //iShaderResource
    void ReloadAllShaders() override { return shader_resource.ReloadAllShaders(); }
    ShaderHandle LoadShader(const std::string &name) override { return shader_resource.LoadShader(name); }
    ResourceHandle LoadShaderResource(FileResourceId res_id) override { return shader_resource.LoadShaderResource(res_id);}
    const Resources::ShaderVariables *GetShaderVariables(ShaderHandle handle) override { return shader_resource.GetShaderVariables(handle); }

    //iTextureResource
    ResourceHandle LoadTextureResource(FileResourceId file_id) override { return texture_resource.LoadTextureResource(file_id); }
    TextureHandle LoadTexture(const std::string &name) override { return texture_resource.LoadTexture(name); }

    // clang-format on
private:
    ContextResourceLoaderProxy res_loader_proxy;
    Configuration::RendererConfiguration config;

    Resources::Shader::ShaderFileCache shader_file_cache;
    Resources::Shader::ShaderCodeLoader shader_code_loader;
    Resources::Shader::ShaderResource shader_resource;

    Resources::Texture::FreeImageLoader free_image_loader;
    Resources::Texture::TextureResource texture_resource;

    // std::unique_ptr<iMeshManager> mesh_manager;
    // Memory::aligned_ptr<Shader::ShaderResource> shaderResource;
    // Memory::aligned_ptr<TextureResource> textureResource;
    // Memory::aligned_ptr<MeshManager> meshManager;
    // Memory::aligned_ptr<MaterialManager> materialManager;
    // Memory::aligned_ptr<VAOResource> vaoManager;
};

//----------------------------------------------------------------------------------

std::unique_ptr<iResourceManager>
iResourceManager::CreteDefaultResourceManager(gsl::not_null<iAsyncLoader *> async_loader,
                                              gsl::not_null<iReadOnlyFileSystem *> file_system) {

    std::unique_ptr<CommandQueue> init_queue = std::make_unique<CommandQueue>();
    auto rm = std::make_unique<ResourceManager>(async_loader, file_system, init_queue.get());

    //TODO: init queue should not be created/executed in CreteDefaultResourceManager
    init_queue->Execute();

    return rm;
}

} // namespace MoonGlare::Renderer
