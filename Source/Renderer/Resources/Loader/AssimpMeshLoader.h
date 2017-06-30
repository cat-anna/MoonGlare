#pragma once

namespace MoonGlare::Renderer::Resources::Loader {

class AssimpMeshLoader : public MultiAsyncFileSystemRequest
{
    using MeshConf = Configuration::Mesh;
public:
    AssimpMeshLoader(MeshResourceHandle handle, MeshManager *Owner) : owner(Owner), handle(handle) { }

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override;
private:
    MeshManager *owner;
    MeshResourceHandle handle;

    std::string baseURI;
    std::unique_ptr<Assimp::Importer> importer;
    const aiScene* scene;

    void LoadMeshes(ResourceLoadStorage &storage);
    void LoadMaterial(unsigned index, unsigned submeshindex, ResourceLoadStorage &storage);
};

} //namespace MoonGlare::Renderer::Resources::Loader 
