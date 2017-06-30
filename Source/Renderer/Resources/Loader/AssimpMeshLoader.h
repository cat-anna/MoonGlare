#pragma once

namespace MoonGlare::Renderer::Resources::Loader {

class AssimpMeshLoader : public MultiAsyncFileSystemRequest
{
    using MeshConf = Configuration::Mesh;
public:
    AssimpMeshLoader(MeshResourceHandle handle, MeshManager *Owner, MaterialManager &matmgr) : 
        owner(Owner), handle(handle), materialManager(matmgr) { }

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override;
private:
    MeshManager *owner;
    MaterialManager &materialManager;

    MeshResourceHandle handle;

    std::string ModelURI;
    std::string baseURI;
    std::unique_ptr<Assimp::Importer> importer;
    const aiScene* scene;

    void LoadMeshes(ResourceLoadStorage &storage);
    void LoadMaterial(unsigned index, MaterialResourceHandle &h, ResourceLoadStorage &storage);
};

} //namespace MoonGlare::Renderer::Resources::Loader 
