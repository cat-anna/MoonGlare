#pragma once

#include <Foundation/Resources/Mesh.h>

namespace MoonGlare::Resources::Blob {

void WriteMeshBlob(std::ostream& output, const MeshData &meshData);

void DumpMeshBlob(const MeshData &meshData, const std::string name = "");

}
