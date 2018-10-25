#pragma once

#include "BlobHeaders.h"
#include <Foundation/Resources/Mesh.h>

namespace MoonGlare::Resources::Blob {

void WriteMeshBlob(std::ostream& output, const MeshData *meshData);

}
