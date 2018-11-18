#pragma once

#include <Foundation/Resources/Mesh.h>

namespace MoonGlare::Renderer::Resources::Loader {
using namespace MoonGlare::Resources;

class MeshLoader : public MultiAsyncFileSystemRequest
{
    using MeshConf = Configuration::Mesh;
public:
    MeshLoader(MeshResourceHandle handle, MeshManager &Owner);
    ~MeshLoader();

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) override;
private:
    MeshManager &owner;
    MeshResourceHandle handle;
};

} //namespace MoonGlare::Renderer::Resources::Loader 
