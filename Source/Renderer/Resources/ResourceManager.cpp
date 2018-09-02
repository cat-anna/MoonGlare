

#include "AsyncLoader.h"
#include "Mesh/MeshResource.h"
#include "MaterialManager.h"

#include "Texture/TextureResource.h"
#include "Mesh/VAOResource.h"
#include "Shader/ShaderResource.h"

#include "ResourceManager.h"
#include "../Renderer.h"


namespace MoonGlare::Renderer::Resources {

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

iAsyncLoader* ResourceManager::GetLoader() {
    return m_AsyncLoader.get();
}

bool ResourceManager::Initialize(RendererFacade *Renderer, iFileSystem *fileSystem) {
    RendererAssert(Renderer);
    RendererAssert(fileSystem);

    m_RendererFacade = Renderer;

    m_AsyncLoader = std::make_unique<AsyncLoader>(this, fileSystem, m_RendererFacade->GetConfiguration());

//    auto conf = m_RendererFacade->GetConfiguration();

    textureResource = Memory::make_aligned<TextureResource>();
    textureResource->Initialize(this, fileSystem);

    vaoManager = Memory::make_aligned<VAOResource>(this);

    shaderResource = Memory::make_aligned<Shader::ShaderResource>();
    shaderResource->Initialize(this, fileSystem);

    materialManager = Memory::make_aligned<MaterialManager>(this);
    meshManager = Memory::make_aligned<MeshManager>(this);


    return true;
}

bool ResourceManager::Finalize() {
    m_AsyncLoader.reset();

    vaoManager.reset();
    materialManager.reset();

    if (shaderResource) {
        shaderResource->Finalize();
    }
    shaderResource.reset();

    textureResource->Finalize();
    textureResource.reset();

    return true;
}

const Configuration::RuntimeConfiguration * ResourceManager::GetConfiguration() const {
    return m_RendererFacade->GetConfiguration();
}

} //namespace MoonGlare::Renderer::Resources 
