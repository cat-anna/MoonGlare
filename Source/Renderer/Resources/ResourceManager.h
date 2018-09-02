
#pragma once
#include <Foundation/iFileSystem.h>
#include "../iAsyncLoader.h"

namespace MoonGlare::Renderer::Resources {

namespace Shader { class ShaderResource; }

class AsyncLoader;

class MeshManager;
class textureResource;
class MaterialManager;
class VAOResource;

class ResourceManager final {
public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize(RendererFacade *Renderer, iFileSystem *fileSystem);
    bool Finalize();

    const Configuration::RuntimeConfiguration* GetConfiguration() const;
    RendererFacade *GetRendererFacade() { return m_RendererFacade; }
    iAsyncLoader* GetLoader();

    TextureResource& GetTextureResource() {
        RendererAssert(this); 
        return *textureResource;
    }
    VAOResource& GetVAOResource() {
        RendererAssert(this);
        return *vaoManager;
    }
    Shader::ShaderResource& GetShaderResource() {
        RendererAssert(this);
        return *shaderResource;
    }
    MaterialManager& GetMaterialManager() {
        RendererAssert(this);
        return *materialManager;
    }
    MeshManager& GetMeshManager() {
        RendererAssert(this);
        return *meshManager;
    }

private: 
    RendererFacade *m_RendererFacade = nullptr;
    std::unique_ptr<AsyncLoader> m_AsyncLoader;
    void* padding;
    void* padding2;

    Memory::aligned_ptr<Shader::ShaderResource> shaderResource;
    Memory::aligned_ptr<TextureResource> textureResource;
    Memory::aligned_ptr<MeshManager> meshManager;
    Memory::aligned_ptr<MaterialManager> materialManager;
    Memory::aligned_ptr<VAOResource> vaoManager;
};

//static_assert((sizeof(ResourceManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
