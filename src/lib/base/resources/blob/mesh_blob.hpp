#pragma once

#include "resources/mesh.hpp"
#include <gsl/span>
#include <memory>
#include <string>

namespace MoonGlare::Resources::Blob {

struct MeshLoad {
    std::unique_ptr<uint8_t[]> memory;
    MeshData mesh = {};
};

void WriteMeshBlob(std::ostream &output, const MeshData &meshData);
bool ReadMeshBlob(gsl::span<uint8_t> memory, MeshLoad &output);

void DumpMeshBlob(const MeshData &meshData, const std::string name = "");

} // namespace MoonGlare::Resources::Blob
