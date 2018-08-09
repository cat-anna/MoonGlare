#pragma once

namespace MoonGlare::Renderer::Resources::Loader {

class CustomMeshLoader : public iAsyncTask
{
    using MeshConf = Configuration::Mesh;
public:
    CustomMeshLoader(MeshResourceHandle handle, MeshManager &Owner) :
        owner(Owner), handle(handle) { }

    void Do(ResourceLoadStorage &storage) override;

    char __padding[12];

    Renderer::Configuration::Mesh::SubMesh meshArray;

    MaterialResourceHandle materialArray;

private:
    MeshManager &owner;
    MeshResourceHandle handle;
};

} //namespace MoonGlare::Renderer::Resources::Loader 
