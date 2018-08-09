#pragma once

#include "../../Commands/CommandQueue.h"
#include "VAOBuilder.h"

namespace MoonGlare::Renderer::Resources {
struct MeshData;
}

namespace MoonGlare::Renderer::Resources::Builder {

class MeshBuilder final {
    using Conf = Configuration::Mesh;
    using ConfRes = Configuration::Resources;
public:
    void AllocateVAO();
    void UpdateVAO();
    void Set(const Conf::SubMesh &meshes);
    void Set(MaterialResourceHandle materials);
    void Commit();

    VAOBuilder vaoBuilder;
    MeshManager &meshManager;
    Conf::SubMesh &subMeshArray;
    MaterialResourceHandle &subMeshMaterialArray;
    MeshResourceHandle handle;
    MeshData &meshData;
    Commands::CommandQueue &queue;
    Commands::CommandKey currentKey;
};

//static_assert((sizeof(MaterialManager) % 16) == 0, "Invalid size!");
//static_assert(std::is_trivial<MaterialBuilder>::value, "must be trivial!");

} //namespace MoonGlare::Renderer::Resources::Builder
