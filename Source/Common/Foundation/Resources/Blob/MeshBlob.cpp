
#include "BlobHeaders.h"
#include "MeshBlob.h"
#include "BlobIo.h"

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

bool ReadMeshBlob(gsl::span<uint8_t> memory, MeshLoad &output) {
    assert(!memory.empty());

    if (memory.size_bytes() < sizeof(BlobHeader)) {
        AddLogf(Error, "Cannot interpret mesh blob: empty data");
        return false;
    }

    const GenericHeader *headers = reinterpret_cast<const GenericHeader*>(memory.data());

    const BlobHeader *bh = reinterpret_cast<const BlobHeader*>(headers + 0);
    if (bh->magic != MagicValue::Blob || bh->contentMagic != MagicValue::Mesh || bh->headerCount != 4) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid headers");
        return false;
    }

    if (memory.size_bytes() < bh->headerCount * HeaderSize) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid header size");
        return false;
    }

    const MeshHeader *mh  = reinterpret_cast<const MeshHeader*>(headers + 1);
    if (mh->magic != MagicValue::Mesh || std::max(mh->blobDataHeaderIndex, mh->meshDataHeaderIndex) >= bh->headerCount) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid secondary header");
        return false;
    }

    const DataHeader *dhBlob = reinterpret_cast<const DataHeader*>(headers + mh->blobDataHeaderIndex);
    const DataHeader *dhMesh = reinterpret_cast<const DataHeader*>(headers + mh->meshDataHeaderIndex);

    if (dhBlob->magic != MagicValue::Data || dhMesh->magic != MagicValue::Data) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid data headers");
        return false;
    }                                                           

    size_t fileSize = std::max(dhBlob->fileOffset + dhBlob->fileDataSize, dhMesh->fileOffset + dhMesh->fileDataSize);
    if (memory.size_bytes() < fileSize) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid data size");
        return false;
    }

    //TODO: check header version
    BlobReader br(bh);

    if (dhMesh->fileDataSize != sizeof(output.mesh)) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid mesh size");
        return false;
    }

    bool r1 = br.ReadDataBlob(memory, *dhMesh, { (uint8_t*)&output.mesh , sizeof(output.mesh) });
    bool r2 = br.ReadDataBlob(memory, *dhBlob, output.memory);

    if (!r1 || !r2) {
        AddLogf(Error, "Cannot interpret mesh blob: failure during data reading");
        return false;
    }   

    output.mesh.UpdatePointers((intptr_t) output.memory.get());
    if (!output.mesh.CheckPointers()) {
        AddLogf(Error, "Cannot interpret mesh blob: pointer validation failed");
        return false;
    }
    
    return true;
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
