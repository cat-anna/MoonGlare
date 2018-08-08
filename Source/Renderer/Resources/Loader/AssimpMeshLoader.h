#pragma once

#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

namespace MoonGlare::Renderer::Resources::Loader {

class AssimpMeshLoader : public MultiAsyncFileSystemRequest
{
    using MeshConf = Configuration::Mesh;
public:
    AssimpMeshLoader(std::string subpath, MeshResourceHandle handle, MeshManager &Owner, MaterialManager &matmgr) : 
        owner(Owner), handle(handle), materialManager(matmgr), subpath(std::move(subpath)) { }

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override;
private:
    MeshManager &owner;
    MaterialManager &materialManager;

    MeshResourceHandle handle;

    int GetMeshIndex() const;

    std::string subpath;
    std::string ModelURI;
    std::string baseURI;
    std::unique_ptr<Assimp::Importer> importer;
    const aiScene* scene;

    void LoadMeshes(ResourceLoadStorage &storage);
    void LoadMaterial(unsigned index, MaterialResourceHandle &h, ResourceLoadStorage &storage);
};

} //namespace MoonGlare::Renderer::Resources::Loader 
