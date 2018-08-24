#include "pch.h"

#define NEED_MESH_BUILDER
#define NEED_MATERIAL_BUILDER

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "MeshResource.h"
#include "MeshUpdate.h"

namespace MoonGlare::Renderer::Resources::Loader {

void CustomMeshLoader::Do(ResourceLoadStorage &storage) {
    auto builder = owner.GetBuilder(storage.m_Queue, handle);
    builder.UpdateVAO();
    builder.Set(meshArray);
    builder.Set(materialArray);    
    builder.Commit();
}

} //namespace MoonGlare::Renderer::Resources::Loader 
