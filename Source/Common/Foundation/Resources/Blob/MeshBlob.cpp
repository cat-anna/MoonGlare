
#include "BlobHeaders.h"
#include "MeshBlob.h"

namespace MoonGlare::Resources::Blob {

void WriteMeshBlob(std::ostream& output, const MeshData &meshData) {
    auto data = meshData;
    data.UpdatePointers(0x1000'0000);

    size_t fileOffset = 0;

    BlobHeader blobH = { MagicValue::Blob, MagicValue::Mesh, 4 };
    MeshHeader meshH = { MagicValue::Mesh, 2, 3 };
    DataHeader meshDataH = { MagicValue::Data, 0, 0, 0 };
    DataHeader blobDataH = { MagicValue::Data, 0, 0, 0 };

    fileOffset = sizeof(blobH) + sizeof(meshH) + sizeof(meshDataH) + sizeof(blobDataH);

    meshDataH.dataSize = sizeof(data);
    meshDataH.fileDataSize = sizeof(data);
    meshDataH.fileOffset = fileOffset;
    fileOffset += meshDataH.fileDataSize;

    blobDataH.dataSize = meshData.memoryBlockSize;
    blobDataH.fileDataSize = meshData.memoryBlockSize;
    blobDataH.fileOffset = fileOffset;
    fileOffset += blobDataH.fileDataSize;

    output.write((char*)&blobH, sizeof(blobH));
    output.write((char*)&meshH, sizeof(meshH));
    output.write((char*)&meshDataH, sizeof(meshDataH));
    output.write((char*)&blobDataH, sizeof(blobDataH));
    output.write((char*)&data, sizeof(data));
    output.write((char*)meshData.memoryBlockFront, blobDataH.fileDataSize);
}

void DumpMeshBlob(const MeshData &meshData, const std::string name) {
    static int animIndex = 0;

    std::string outFile = "logs/mesh.";
    if (name.empty())
        outFile += std::to_string(animIndex++);
    else
        outFile += name;
    outFile += ".anim";

    std::ofstream of(outFile, std::ios::out | std::ios::binary);
    Blob::WriteMeshBlob(of, meshData);
}

}
