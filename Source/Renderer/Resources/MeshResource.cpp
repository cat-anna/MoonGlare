#include "MeshResource.h"
#include "ResourceManager.h"

#include "Loader/AssimpMeshLoader.h"

namespace MoonGlare::Renderer::Resources {

MeshManager::MeshManager(ResourceManager *Owner) :
    resourceManager(Owner) {

    allocationBitmap.ClearAllocation();
    deviceHandle.fill(Device::InvalidVAOHandle);
    subMesh.fill({});
    materialHandle.fill({});

    generations.fill(1);//TODO: random?

    Conf::VAOBuffers vaob;
    vaob.fill(Device::InvalidBufferHandle);
    vaoBuffer.fill(vaob);
}

MeshManager::~MeshManager() {
}

//---------------------------------------------------------------------------------------

bool MeshManager::Allocate(HandleType &hout) {
    if (IsHandleValid(hout))
        return true;

    Bitmap::Index_t index;
    if (allocationBitmap.Allocate(index)) {
       //if (/*queue &&*/ deviceHandle[index] == Device::InvalidTextureHandle) {
       //    //auto arg = queue->PushCommand<Commands::TextureSingleAllocate>();
       //    //arg->m_Out = &m_GLHandle[index];
       //}
        hout.index = static_cast<HandleType::Index_t>(index);
        hout.generation = generations[index];
        hout.deviceHandle = &deviceHandle[index];
        return true;
    }
    else {
        AddLogf(Debug, "mesh allocation failed");
        return false;
    }
}

void MeshManager::Release(HandleType hin) {
    if (!IsHandleValid(hin))
        return;

    hin.deviceHandle = nullptr;
    ++generations[hin.index];

    allocationBitmap.Release(hin.index);
}

bool MeshManager::IsHandleValid(HandleType &h) const {
    if (h.index >= Conf::Limit)
        return false;
    if (generations[h.index] != h.generation) {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------

bool MeshManager::LoadMesh(const std::string &uri, HandleType &hout) {
    auto cache = loadedMeshes.find(uri);
    if (cache != loadedMeshes.end() && IsHandleValid(cache->second)) {
        AddLogf(Performance, "mesh load cache hit");
        hout = cache->second;
        return true;
    }

    if (!Allocate(hout)) {
        return false;
    }
    loadedMeshes[uri] = hout;

    auto request = std::make_shared<Loader::AssimpMeshLoader>(hout, this, resourceManager->GetMaterialManager());
    resourceManager->GetLoaderIf()->QueueRequest(uri, request);

    return true;
}

} //namespace MoonGlare::Renderer::Resources 
