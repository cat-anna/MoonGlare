#pragma once

#include "resources/Mesh.hpp"
#include <memory>
#include <string>

struct aiScene;
namespace Assimp {
class Importer;
}

namespace MoonGlare::Resources::Importer {

struct MeshImport {
    std::unique_ptr<uint8_t[]> memory;
    MeshData mesh = {};
};

void ImportAssimpMesh(const aiScene *scene, int meshIndex, MeshSource &output);
void ImportMeshSource(const MeshSource &source, MeshImport &output);

} // namespace MoonGlare::Resources::Importer
