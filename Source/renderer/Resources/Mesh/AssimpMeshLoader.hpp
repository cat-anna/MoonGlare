#pragma once

#include <Foundation/Resources/Mesh.h>

struct aiScene;
namespace Assimp {
class Importer;
}

namespace MoonGlare::Renderer::Resources::Loader {
using namespace MoonGlare::Resources;

class AssimpMeshLoader : public MultiAsyncFileSystemRequest
{
    using MeshConf = Configuration::Mesh;
public:
    AssimpMeshLoader(std::string subpath, MeshResourceHandle handle, MeshManager &Owner);
    ~AssimpMeshLoader();

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override;
private:
    MeshManager &owner;
    MeshResourceHandle handle;

    int GetMeshIndex() const;

    std::string subpath;
    std::string ModelURI;
    std::string baseURI;
    std::unique_ptr<Assimp::Importer> importer;
    const aiScene* scene;

    void LoadMeshes(ResourceLoadStorage &storage);
};

} //namespace MoonGlare::Renderer::Resources::Loader 
