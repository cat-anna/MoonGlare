#pragma once

namespace MoonGlare::Renderer::Resources::Loader {

class AssimpMeshLoader : public MultiAsyncFileSystemRequest
{
    using MeshConf = Configuration::Mesh;
public:
    AssimpMeshLoader(MeshResourceHandle handle, MeshManager *Owner) : owner(Owner), handle(handle) { }

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage, iAsyncLoader *loader) override;
private:
    MeshManager *owner;
    MeshResourceHandle handle;

    std::string baseURI;
    std::unique_ptr<Assimp::Importer> importer;
    const aiScene* scene;

    void LoadScene(ResourceLoadStorage &storage);
};

} //namespace MoonGlare::Renderer::Resources::Loader 
