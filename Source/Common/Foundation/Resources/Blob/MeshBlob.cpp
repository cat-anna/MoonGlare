
#include "MeshBlob.h"

namespace MoonGlare::Resources::Blob {

void WriteMeshBlob(std::ostream& output, const MeshData *meshData) {
    auto data = *meshData;
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

    blobDataH.dataSize = meshData->memoryBlockSize;
    blobDataH.fileDataSize = meshData->memoryBlockSize;
    blobDataH.fileOffset = fileOffset;
    fileOffset += blobDataH.fileDataSize;

    output.write((char*)&blobH, sizeof(blobH));
    output.write((char*)&meshH, sizeof(meshH));
    output.write((char*)&meshDataH, sizeof(meshDataH));
    output.write((char*)&blobDataH, sizeof(blobDataH));
    output.write((char*)&data, sizeof(data));
    output.write((char*)meshData->memoryBlockFront, blobDataH.fileDataSize);
}

}
