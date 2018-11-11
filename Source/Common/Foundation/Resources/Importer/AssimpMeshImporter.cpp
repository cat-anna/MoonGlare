
#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

#include <Foundation/Memory/AlignedPtr.h>

#include "AssimpMeshImporter.h"

namespace MoonGlare::Resources::Importer {

void ImportAssimpMesh(const aiScene *scene, int meshIndex, MeshSource &output) {
    assert(scene);
    assert(meshIndex >= 0 && meshIndex < scene->mNumMeshes);
    
    auto mesh = scene->mMeshes[meshIndex];
    size_t numIndices = mesh->mNumFaces * 3;
    size_t baseVertex = 0;// static_cast<uint16_t>(NumVertices);
    size_t baseIndex = 0;// static_cast<uint16_t>(numIndices);// *sizeof(uint32_t));

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
                aiVector3D &UVW = mesh->mTextureCoords[0][vertid]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
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
            assert(f->mNumIndices == 3, 0);
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

        unsigned BaseVertex = 0;// meshData.BaseVertex;
        for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            const auto *bone = mesh->mBones[boneIndex];
            meshData.boneNames[boneIndex] = bone->mName.data;
            meshData.boneOffsetMatrices[boneIndex] = glm::transpose(*(glm::fmat4*)&bone->mOffsetMatrix);

            std::string vs;
            for (unsigned k = 0; k < bone->mNumWeights; ++k) {

                auto &VertexWeight = bone->mWeights[k];

                auto vertexid = BaseVertex + VertexWeight.mVertexId;
                auto & vertexBones = meshData.vertexBones[vertexid];
                vs += std::to_string(vertexid) + " ";
                auto localboneid = AllocLocalBoneIndex(vertexid, vertexBones);
                if (localboneid == MeshSource::InvalidBoneIndex) {
                    __debugbreak();
                    //AddLogf(Warning, "Vertex %d cannot have more than 4 bones [%s]; Skipped influence of bone %s with weight %f", vertexid, subpath.c_str(), meshData.boneNames[boneIndex].c_str(), VertexWeight.mWeight);
                    continue;
                }

                meshData.vertexBones[vertexid][localboneid] = boneIndex;
                meshData.vertexBoneWeights[vertexid][localboneid] = VertexWeight.mWeight;
            }
        }
    }

    meshData.UpdateBoundary();
}   
                                                                                                                                                                                   
void ImportMeshSource(const MeshSource &source, MeshImport &output) {
    size_t memorySize = 0;

    size_t verticlesSize = source.verticles.size() * sizeof(source.verticles[0]);
    size_t UV0Size = source.UV0.size() * sizeof(source.UV0[0]);
    size_t normalsSize = source.normals.size() * sizeof(source.normals[0]);
    size_t tangentsSize = source.tangents.size() * sizeof(source.tangents[0]);
    size_t indexSize = source.index.size() * sizeof(source.index[0]);

    size_t vertexBonesSize = source.vertexBones.size() * sizeof(source.vertexBones[0]);
    size_t vertexBoneWeightsSize = source.vertexBoneWeights.size() * sizeof(source.vertexBoneWeights[0]);
    size_t boneMatricesSize = source.boneOffsetMatrices.size() * sizeof(source.boneOffsetMatrices[0]);

    size_t boneNamesArraySize = source.boneNames.size() * sizeof(const char*);
    uint16_t boneNamesValuesSize = 0;
    std::for_each(source.boneNames.begin(), source.boneNames.end(), [&boneNamesValuesSize](auto &item) { boneNamesValuesSize += item.size() + 1; });

    size_t verticlesOffset = memorySize;
    memorySize += verticlesSize;
    size_t UV0Offset = memorySize;
    memorySize += UV0Size;
    size_t normalsOffset = memorySize;
    memorySize += normalsSize;
    size_t tangentsOffset = memorySize;
    memorySize += tangentsSize;
    size_t indexOffset = memorySize;
    memorySize += indexSize;

    size_t vertexBonesOffset = memorySize;
    memorySize += vertexBonesSize;
    size_t vertexBoneWeightsOffset = memorySize;
    memorySize += vertexBoneWeightsSize;

    size_t boneNamesArrayOffset = memorySize;
    memorySize += boneNamesArraySize;
    size_t boneNamesValuesOffset = memorySize;
    memorySize += boneNamesValuesSize;

    size_t boneMatricesOffset = memorySize;
    memorySize += boneMatricesSize;

    output.memory.reset(new char[memorySize]);
    char *mem = output.memory.get();

    MeshData &md = output.mesh;
    md = {};

    md.verticles = (glm::fvec3*)(mem + verticlesOffset);
    memcpy(md.verticles, &source.verticles[0], verticlesSize);

    if (source.UV0.size() > 0) {
        md.UV0 = (glm::fvec2*)(mem + UV0Offset);
        memcpy(md.UV0, &source.UV0[0], UV0Size);
    }

    if (source.normals.size() > 0) {
        md.normals = (glm::fvec3*)(mem + normalsOffset);
        memcpy(md.normals, &source.normals[0], normalsSize);
    }

    if (source.tangents.size() > 0) {
        md.tangents = (glm::fvec3*)(mem + tangentsOffset);
        memcpy(md.tangents, &source.tangents[0], tangentsSize);
    }

    md.index = (uint32_t*)(mem + indexOffset);
    memcpy(md.index, &source.index[0], indexSize);

    if (source.vertexBones.size() > 0) {
        md.vertexBones = (glm::u8vec4*)(mem + vertexBonesOffset);
        memcpy(md.vertexBones, &source.vertexBones[0], vertexBonesSize);
    }
    if (source.vertexBoneWeights.size() > 0) {
        md.vertexBoneWeights = (glm::fvec4*)(mem + vertexBoneWeightsOffset);
        memcpy(md.vertexBoneWeights, &source.vertexBoneWeights[0], vertexBoneWeightsSize);
    }
    if (source.boneNames.size() > 0) {
        md.boneNameValues = (const char*)(mem + boneNamesValuesOffset);
        md.boneNameOffsets = (uint16_t*)(mem + boneNamesArrayOffset);
        uint16_t offset = 0;
        for (size_t i = 0; i < source.boneNames.size(); ++i) {
            char * str = (char*)(mem + boneNamesValuesOffset + offset);
            md.boneNameOffsets[i] = offset;
            size_t len = source.boneNames[i].size();
            memcpy(str, source.boneNames[i].c_str(), len);
            str[len] = '\0';
            offset += len + 1;
        }
        assert(offset == boneNamesValuesSize);
    }
    if (source.boneOffsetMatrices.size() > 0) {
        md.boneMatrices = (glm::fmat4*)(mem + boneMatricesOffset);
        memcpy(md.boneMatrices, &source.boneOffsetMatrices[0], boneMatricesSize);
    }
    md.boneCount = source.boneNames.size();

    md.vertexCount = source.verticles.size();
    md.indexCount = source.index.size();

    md.halfBoundingBox = source.halfBoundingBox;
    md.boundingRadius = source.boundingRadius;
    md.memoryBlockSize = memorySize;
    md.memoryBlockFront = mem;
    md.ready = false;
}
 
}
