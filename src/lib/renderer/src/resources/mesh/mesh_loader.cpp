#if 0

#include "../../iAsyncLoader.h"
#include "../../nfRenderer.h"
#include "MeshLoader.h"
#include "MeshResource.h"

#include "../MaterialManager.h"

#include <Foundation/Resources/Blob/MeshBlob.h>

namespace MoonGlare::Renderer::Resources::Loader {

MeshLoader::MeshLoader(MeshResourceHandle handle, MeshManager &Owner) :
    owner(Owner), handle(handle) {}

MeshLoader::~MeshLoader() {
}

void MeshLoader::OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) {
    Resources::Blob::MeshLoad ml;
    auto success = Resources::Blob::ReadMeshBlob({ filedata.get(), (ptrdiff_t)filedata.byte_size() }, ml);

    if (!success) {
        AddLogf(Error, "Failed to load mesh %s", requestedURI.c_str());
        return;
    }

    owner.ApplyMeshSource(handle, ml.mesh, std::move(ml.memory));
}

} //namespace MoonGlare::Renderer::Resources::Loader

#endif
