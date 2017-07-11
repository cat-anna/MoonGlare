#pragma once

namespace MoonGlare::Renderer::Resources::Loader {

class CustomMeshLoader : public iAsyncTask
{
    using MeshConf = Configuration::Mesh;
public:
    CustomMeshLoader(MeshResourceHandle handle, MeshManager &Owner, MaterialManager &matmgr) :
        owner(Owner), handle(handle), materialManager(matmgr) { }

    void Do(ResourceLoadStorage &storage) override;

    Renderer::Configuration::Mesh::SubMeshArray meshArray;
    Renderer::Configuration::Mesh::SubMeshMaterialArray materialArray;

private:
    MeshManager &owner;
    MaterialManager &materialManager;
    MeshResourceHandle handle;
};

} //namespace MoonGlare::Renderer::Resources::Loader 
