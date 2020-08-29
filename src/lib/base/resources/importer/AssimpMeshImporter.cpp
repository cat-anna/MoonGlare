
#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning(pop)

#include "AssimpMeshImporter.hpp"
#include <aligned_ptr.hpp>
#include <glm/glm.hpp>

namespace MoonGlare::Resources::Importer {

void ImportAssimpMesh(const aiScene *scene, int meshIndex, MeshSource &output) {
    assert(scene);
    assert(meshIndex >= 0 && static_cast<unsigned>(meshIndex) < scene->mNumMeshes);

    auto mesh = scene->mMeshes[meshIndex];
    size_t numIndices = mesh->mNumFaces * 3;
    size_t baseVertex = 0; // static_cast<uint16_t>(NumVertices);
    size_t baseIndex = 0;  // static_cast<uint16_t>(numIndices);// *sizeof(uint32_t));

    //meshes.elementMode = GL_TRIANGLES;
    //meshes.indexElementType = GL_UNSIGNED_INT;

    size_t NumVertices = mesh->mNumVertices;

    MeshSource &meshData = output;
    meshData.verticles.resize(NumVertices);
    meshData.UV0.resize(NumVertices);
    meshData.normals.resize(NumVertices);
    meshData.tangents.resize(NumVertices);
    meshData.index.resize(numIndices);

    {
        auto MeshVerticles = &meshData.verticles[baseVertex];
        auto MeshTexCords = &meshData.UV0[baseVertex];
        auto MeshNormals = &meshData.normals[baseVertex];
        auto MeshTangents = &meshData.tangents[baseVertex];

        for (size_t vertid = 0; vertid < mesh->mNumVertices; vertid++) {
            aiVector3D &vertex = mesh->mVertices[vertid];
            aiVector3D &normal = mesh->mNormals[vertid];

            if (mesh->mTextureCoords[0]) {
                aiVector3D &UVW =
                    mesh->mTextureCoords[0][vertid]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
                MeshTexCords[vertid] = glm::fvec2(UVW.x, UVW.y);
            } else {
                MeshTexCords[vertid] = glm::fvec2();
            }
            if (mesh->mTangents) {
                aiVector3D &tangent = mesh->mTangents[vertid];
                MeshTangents[vertid] = glm::fvec3(tangent.x, tangent.y, tangent.z);
            }

            MeshVerticles[vertid] = glm::fvec3(vertex.x, vertex.y, vertex.z);
            MeshNormals[vertid] = glm::fvec3(normal.x, normal.y, normal.z);
        }

        auto meshIndices = &meshData.index[baseIndex];
        for (size_t face = 0; face < mesh->mNumFaces; face++) {
            aiFace *f = &mesh->mFaces[face];
            assert(f->mNumIndices == 3);
            meshIndices[face * 3 + 0] = f->mIndices[0];
            meshIndices[face * 3 + 1] = f->mIndices[1];
            meshIndices[face * 3 + 2] = f->mIndices[2];
        }
    }

    if (mesh->HasBones()) {
        meshData.vertexBones.resize(NumVertices, MeshSource::InvalidBoneIndexSlot());
        meshData.vertexBoneWeights.resize(NumVertices);
        meshData.boneNames.resize(mesh->mNumBones);
        meshData.boneOffsetMatrices.resize(mesh->mNumBones);

        auto AllocLocalBoneIndex = [](size_t index, const glm::u8vec4 &vec) -> uint8_t {
            for (uint8_t i = 0; i < 4; ++i)
                if (vec[i] == MeshSource::InvalidBoneIndex)
                    return i;
            return MeshSource::InvalidBoneIndex;
        };

        unsigned BaseVertex = 0; // meshData.BaseVertex;
        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            const auto *bone = mesh->mBones[boneIndex];
            meshData.boneNames[boneIndex] = bone->mName.data;
            meshData.boneOffsetMatrices[boneIndex] = glm::transpose(*(glm::fmat4 *)&bone->mOffsetMatrix);

            std::string vs;
            for (unsigned k = 0; k < bone->mNumWeights; ++k) {

                auto &VertexWeight = bone->mWeights[k];

                auto vertexid = BaseVertex + VertexWeight.mVertexId;
                auto &vertexBones = meshData.vertexBones[vertexid];
                vs += std::to_string(vertexid) + " ";
                auto localboneid = AllocLocalBoneIndex(vertexid, vertexBones);
                if (localboneid == MeshSource::InvalidBoneIndex) {
                    __debugbreak();
                    //AddLogf(Warning, "Vertex %d cannot have more than 4 bones [%s]; Skipped influence of bone %s with weight %f", vertexid, subpath.c_str(), meshData.boneNames[boneIndex].c_str(), VertexWeight.mWeight);
                    continue;
                }

                meshData.vertexBones[vertexid][localboneid] = static_cast<uint8_t>(boneIndex);
                meshData.vertexBoneWeights[vertexid][localboneid] = VertexWeight.mWeight;
            }
        }
    }

    meshData.UpdateBoundary();
}

void ImportMeshSource(const MeshSource &source, MeshImport &output) {
    uint32_t memorySize = 0;

    uint32_t verticlesSize = static_cast<uint32_t>(source.verticles.size() * sizeof(source.verticles[0]));
    uint32_t UV0Size = static_cast<uint32_t>(source.UV0.size() * sizeof(source.UV0[0]));
    uint32_t normalsSize = static_cast<uint32_t>(source.normals.size() * sizeof(source.normals[0]));
    uint32_t tangentsSize = static_cast<uint32_t>(source.tangents.size() * sizeof(source.tangents[0]));
    uint32_t indexSize = static_cast<uint32_t>(source.index.size() * sizeof(source.index[0]));

    uint32_t vertexBonesSize = static_cast<uint32_t>(source.vertexBones.size() * sizeof(source.vertexBones[0]));
    uint32_t vertexBoneWeightsSize =
        static_cast<uint32_t>(source.vertexBoneWeights.size() * sizeof(source.vertexBoneWeights[0]));
    uint32_t boneMatricesSize =
        static_cast<uint32_t>(source.boneOffsetMatrices.size() * sizeof(source.boneOffsetMatrices[0]));

    uint32_t boneNamesArraySize = static_cast<uint32_t>(source.boneNames.size() * sizeof(const char *));
    uint16_t boneNamesValuesSize = 0;
    for (const auto &item : source.boneNames) {
        boneNamesValuesSize += static_cast<uint16_t>(item.size() + 1);
    }

    uint32_t verticlesOffset = memorySize;
    memorySize += verticlesSize;
    uint32_t UV0Offset = memorySize;
    memorySize += UV0Size;
    uint32_t normalsOffset = memorySize;
    memorySize += normalsSize;
    uint32_t tangentsOffset = memorySize;
    memorySize += tangentsSize;
    uint32_t indexOffset = memorySize;
    memorySize += indexSize;

    uint32_t vertexBonesOffset = memorySize;
    memorySize += vertexBonesSize;
    uint32_t vertexBoneWeightsOffset = memorySize;
    memorySize += vertexBoneWeightsSize;

    uint32_t boneNamesArrayOffset = memorySize;
    memorySize += boneNamesArraySize;
    uint32_t boneNamesValuesOffset = memorySize;
    memorySize += boneNamesValuesSize;

    uint32_t boneMatricesOffset = memorySize;
    memorySize += boneMatricesSize;

    output.memory.reset(new uint8_t[memorySize]);
    uint8_t *mem = output.memory.get();

    MeshData &md = output.mesh;
    md = {};

    md.verticles = (glm::fvec3 *)(mem + verticlesOffset);
    memcpy(md.verticles, &source.verticles[0], verticlesSize);

    if (source.UV0.size() > 0) {
        md.UV0 = (glm::fvec2 *)(mem + UV0Offset);
        memcpy(md.UV0, &source.UV0[0], UV0Size);
    }

    if (source.normals.size() > 0) {
        md.normals = (glm::fvec3 *)(mem + normalsOffset);
        memcpy(md.normals, &source.normals[0], normalsSize);
    }

    if (source.tangents.size() > 0) {
        md.tangents = (glm::fvec3 *)(mem + tangentsOffset);
        memcpy(md.tangents, &source.tangents[0], tangentsSize);
    }

    md.index = (uint32_t *)(mem + indexOffset);
    memcpy(md.index, &source.index[0], indexSize);

    if (source.vertexBones.size() > 0) {
        md.vertexBones = (glm::u8vec4 *)(mem + vertexBonesOffset);
        memcpy(md.vertexBones, &source.vertexBones[0], vertexBonesSize);
    }
    if (source.vertexBoneWeights.size() > 0) {
        md.vertexBoneWeights = (glm::fvec4 *)(mem + vertexBoneWeightsOffset);
        memcpy(md.vertexBoneWeights, &source.vertexBoneWeights[0], vertexBoneWeightsSize);
    }
    if (source.boneNames.size() > 0) {
        md.boneNameValues = (const char *)(mem + boneNamesValuesOffset);
        md.boneNameOffsets = (uint16_t *)(mem + boneNamesArrayOffset);
        uint16_t offset = 0;
        for (size_t i = 0; i < source.boneNames.size(); ++i) {
            char *str = (char *)(mem + boneNamesValuesOffset + offset);
            md.boneNameOffsets[i] = offset;
            size_t len = source.boneNames[i].size();
            memcpy(str, source.boneNames[i].c_str(), len);
            str[len] = '\0';
            offset += static_cast<uint16_t>(len + 1);
        }
        assert(offset == boneNamesValuesSize);
    }
    if (source.boneOffsetMatrices.size() > 0) {
        md.boneMatrices = (glm::fmat4 *)(mem + boneMatricesOffset);
        memcpy(md.boneMatrices, &source.boneOffsetMatrices[0], boneMatricesSize);
    }
    md.boneCount = static_cast<uint8_t>(source.boneNames.size());

    md.vertexCount = static_cast<uint32_t>(source.verticles.size());
    md.indexCount = static_cast<uint32_t>(source.index.size());

    md.halfBoundingBox = source.halfBoundingBox;
    md.boundingRadius = source.boundingRadius;
    md.memoryBlockSize = memorySize;
    md.memoryBlockFront = mem;
    md.ready = false;
}

} // namespace MoonGlare::Resources::Importer
