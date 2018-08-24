#pragma once

#include "../../Commands/CommandQueue.h"
#include "VAOBuilder.h"

#include "../../Mesh.h"

namespace MoonGlare::Renderer::Resources {
struct MeshData;
}

namespace MoonGlare::Renderer::Resources::Builder {

class MeshBuilder final {
public:
    void AllocateVAO();
    void UpdateVAO();
    void Set(const Mesh &meshes);
    void Set(MaterialResourceHandle materials);
    void Commit();

    VAOBuilder vaoBuilder;
    MeshManager &meshManager;
    Mesh &subMeshArray;
    MaterialResourceHandle &subMeshMaterialArray;
    MeshResourceHandle handle;
    MeshData &meshData;
    Commands::CommandQueue &queue;
    Commands::CommandKey currentKey;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<MaterialBuilder>::value, "must be trivial!");

} //namespace MoonGlare::Renderer::Resources::Builder
