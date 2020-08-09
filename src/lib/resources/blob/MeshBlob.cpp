
#include "MeshBlob.h"
#include "BlobHeaders.h"
#include "BlobIo.h"
#include <math/EigenMath.h>
#include <orbit_logger.h>

namespace MoonGlare::Resources::Blob {

struct V_0 {
    using OffsetType = uint32_t;
    static constexpr VersionValue VersionValue = 0;
    static constexpr OffsetType InvalidOffsetValue = static_cast<OffsetType>(~0);
    struct MeshDataBlob {
        OffsetType verticlesOffset;
        OffsetType UV0;
        OffsetType normals;
        OffsetType tangents;
        OffsetType index;
        OffsetType vertexBones;
        OffsetType vertexBoneWeights;
        OffsetType boneMatrices;
        OffsetType boneNameValues;
        OffsetType boneNameOffsets;
        uint8_t boneCount;
        uint8_t __padding[3];

        uint32_t vertexCount;
        uint32_t indexCount;

        emath::fvec3 halfBoundingBox;
        float boundingRadius;

        uint32_t memoryBlockSize;

        MeshDataBlob() { memset(this, 0, sizeof(*this)); }

        void StoreData(const MeshData &data) {
            auto offset = [&](auto *ptr) -> OffsetType {
                if (ptr == nullptr) {
                    return InvalidOffsetValue;
                }
                uintptr_t base = reinterpret_cast<uintptr_t>(data.memoryBlockFront);
                auto diff = reinterpret_cast<uintptr_t>(ptr) - base;
                if (diff > std::numeric_limits<OffsetType>::max()) {
                    __debugbreak();
                    throw std::runtime_error("MeshDataBlob offset overflow");
                }
                return static_cast<OffsetType>(diff);
            };

            verticlesOffset = offset(data.verticles);
            UV0 = offset(data.UV0);
            normals = offset(data.normals);
            tangents = offset(data.tangents);
            index = offset(data.index);
            vertexBones = offset(data.vertexBones);
            vertexBoneWeights = offset(data.vertexBoneWeights);
            boneMatrices = offset(data.boneMatrices);
            boneNameValues = offset(data.boneNameValues);
            boneNameOffsets = offset(data.boneNameOffsets);

            boneCount = data.boneCount;
            vertexCount = data.vertexCount;
            indexCount = data.indexCount;
            halfBoundingBox = data.halfBoundingBox;
            boundingRadius = data.boundingRadius;
            memoryBlockSize = data.memoryBlockSize;
        }

        bool ReadData(MeshData &data) const {
            if (memoryBlockSize != data.memoryBlockSize || data.memoryBlockFront == nullptr) {
                return false;
            }

            auto read = [&](OffsetType offset, auto *&ptr) -> void {
                if (offset == InvalidOffsetValue) {
                    ptr = nullptr;
                    return;
                }
                uintptr_t base = reinterpret_cast<uintptr_t>(data.memoryBlockFront);
                uintptr_t position = base + offset;
                ptr = reinterpret_cast<decltype(ptr)>(position);
            };

            read(verticlesOffset, data.verticles);
            read(UV0, data.UV0);
            read(normals, data.normals);
            read(tangents, data.tangents);
            read(index, data.index);
            read(vertexBones, data.vertexBones);
            read(vertexBoneWeights, data.vertexBoneWeights);
            read(boneMatrices, data.boneMatrices);
            read(boneNameValues, data.boneNameValues);
            read(boneNameOffsets, data.boneNameOffsets);

            data.boneCount = boneCount;
            data.vertexCount = vertexCount;
            data.indexCount = indexCount;
            data.halfBoundingBox = halfBoundingBox;
            data.boundingRadius = boundingRadius;

            return true;
        }
    };
};

void WriteMeshBlob(std::ostream &output, const MeshData &meshData) {
    using Version = V_0;

    Version::MeshDataBlob data;
    data.StoreData(meshData);

    BlobHeader blobH = {MagicValue::Blob, MagicValue::Mesh, 4};
    MeshHeader meshH = {MagicValue::Mesh, 2, 3, Version::VersionValue, 0};
    DataHeader meshDataH = {MagicValue::Data, 0, 0, 0};
    DataHeader blobDataH = {MagicValue::Data, 0, 0, 0};

    uint32_t fileOffset = sizeof(blobH) + sizeof(meshH) + sizeof(meshDataH) + sizeof(blobDataH);

    meshDataH.dataSize = sizeof(data);
    meshDataH.fileDataSize = sizeof(data);
    meshDataH.fileOffset = fileOffset;
    fileOffset += meshDataH.fileDataSize;

    blobDataH.dataSize = meshData.memoryBlockSize;
    blobDataH.fileDataSize = meshData.memoryBlockSize;
    blobDataH.fileOffset = fileOffset;
    fileOffset += blobDataH.fileDataSize;

    output.write((char *)&blobH, sizeof(blobH));
    output.write((char *)&meshH, sizeof(meshH));
    output.write((char *)&meshDataH, sizeof(meshDataH));
    output.write((char *)&blobDataH, sizeof(blobDataH));
    output.write((char *)&data, sizeof(data));
    output.write((char *)meshData.memoryBlockFront, blobDataH.fileDataSize);
}

bool ReadMeshBlob(gsl::span<uint8_t> memory, MeshLoad &output) {
    using Version = V_0;

    assert(!memory.empty());

    if (memory.size_bytes() < sizeof(BlobHeader)) {
        AddLogf(Error, "Cannot interpret mesh blob: empty data");
        return false;
    }

    const GenericHeader *headers = reinterpret_cast<const GenericHeader *>(memory.data());

    const BlobHeader *bh = reinterpret_cast<const BlobHeader *>(headers + 0);
    if (bh->magic != MagicValue::Blob || bh->contentMagic != MagicValue::Mesh || bh->headerCount != 4) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid headers");
        return false;
    }

    if (static_cast<size_t>(memory.size_bytes()) < bh->headerCount * HeaderSize) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid header size");
        return false;
    }

    const MeshHeader *mh = reinterpret_cast<const MeshHeader *>(headers + 1);
    if (mh->magic != MagicValue::Mesh ||
        std::max(mh->blobDataHeaderIndex, mh->meshDataHeaderIndex) >= bh->headerCount) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid secondary header");
        return false;
    }

    const DataHeader *dhBlob = reinterpret_cast<const DataHeader *>(headers + mh->blobDataHeaderIndex);
    const DataHeader *dhMesh = reinterpret_cast<const DataHeader *>(headers + mh->meshDataHeaderIndex);

    if (dhBlob->magic != MagicValue::Data || dhMesh->magic != MagicValue::Data) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid data headers");
        return false;
    }

    size_t fileSize = std::max(dhBlob->fileOffset + dhBlob->fileDataSize, dhMesh->fileOffset + dhMesh->fileDataSize);
    if (static_cast<size_t>(memory.size_bytes()) < fileSize) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid data size");
        return false;
    }

    // TODO: check header version
    BlobReader br(bh);
    Version::MeshDataBlob dataBlob;
    constexpr size_t meshBlobSize = sizeof(dataBlob);

    if (dhMesh->fileDataSize != meshBlobSize) {
        AddLogf(Error, "Cannot interpret mesh blob: invalid mesh size");
        return false;
    }

    bool r1 = br.ReadDataBlob(memory, *dhMesh, {(uint8_t *)&dataBlob, (intptr_t)meshBlobSize});
    bool r2 = br.ReadDataBlob(memory, *dhBlob, output.memory);

    if (!r1 || !r2) {
        AddLogf(Error, "Cannot interpret mesh blob: failure during data reading");
        return false;
    }

    output.mesh = {};
    output.mesh.memoryBlockFront = output.memory.get();
    output.mesh.memoryBlockSize = dhBlob->dataSize;
    output.mesh.ready = false;

    if (!dataBlob.ReadData(output.mesh)) {
        AddLogf(Error, "Blob data read failed");
        return false;
    }

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

} // namespace MoonGlare::Resources::Blob
