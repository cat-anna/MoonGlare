#include "pch.h"

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "MeshResource.h"
#include "AssimpMeshLoader.h"

#include "../MaterialManager.h"

#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

namespace MoonGlare::Renderer::Resources::Loader {

AssimpMeshLoader::AssimpMeshLoader(std::string subpath, MeshResourceHandle handle, MeshManager &Owner) :
    owner(Owner), handle(handle), subpath(std::move(subpath)) {}

AssimpMeshLoader::~AssimpMeshLoader() {
}

void AssimpMeshLoader::OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) {
    importer = std::make_unique<Assimp::Importer>();

    auto loadflags =
        //aiProcessPreset_TargetRealtime_Fast | 
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_LimitBoneWeights |
        //aiProcess_ImproveCacheLocality |
        //aiProcess_PreTransformVertices |
        aiProcess_Triangulate |
        aiProcess_GenUVCoords |
        aiProcess_SortByPType |
        aiProcess_GlobalScale |
        0;

    scene = importer->ReadFileFromMemory(filedata.get(), filedata.size(), loadflags, strrchr(requestedURI.c_str(), '.'));

    if (!scene) {
        AddLog(Error, fmt::format("Unable to load model file[{}]. Error: {}", requestedURI, importer->GetErrorString()));
        return;
    }

    ModelURI = requestedURI;
    baseURI = requestedURI;
    baseURI.resize(baseURI.rfind('/') + 1);

    LoadMeshes(storage);
}

int AssimpMeshLoader::GetMeshIndex() const {
    static constexpr std::string_view meshProto = "mesh://";

    if (subpath.empty())
    {
        return 0;
    }

    if (subpath.find(meshProto) != 0) {
        __debugbreak();
        return -1;
    }

    const char *beg = subpath.c_str() + meshProto.size();

    if (*beg == '*') {
        ++beg;
        char *end = nullptr;
        long r = strtol(beg, &end, 10);
        if (end == beg) {
            __debugbreak();
            return -1;
        }
        if (r > (int)scene->mNumMeshes)
            return -1;
        return r;
    }

    std::string name(beg);
    for (unsigned i = 0; i < scene->mNumMeshes; i++) {
        auto mesh = scene->mMeshes[i];
        if (name == mesh->mName.data) {
            return i;
        }
    }

    __debugbreak();
    return -1;
}

void AssimpMeshLoader::LoadMeshes(ResourceLoadStorage &storage) {
    uint32_t NumVertices = 0, NumIndices = 0;

    struct LoadInfo {
        size_t baseIndex;
    };

    Mesh meshes = {};
    LoadInfo loadInfo;

    int meshId = GetMeshIndex();
    if (meshId < 0) {
        __debugbreak();
        return;
    }
          
    {
        auto mesh = scene->mMeshes[meshId];
        meshes.valid = true;
        meshes.numIndices = mesh->mNumFaces * 3;
        meshes.baseVertex = static_cast<uint16_t>(NumVertices);
        meshes.baseIndex = static_cast<uint16_t>(NumIndices * sizeof(uint32_t));
        meshes.elementMode = GL_TRIANGLES;
        meshes.indexElementType = GL_UNSIGNED_INT;
        loadInfo.baseIndex = NumIndices;
        NumVertices = mesh->mNumVertices;
        NumIndices = meshes.numIndices;
    }

    MeshSource meshData;
    meshData.verticles.resize(NumVertices);
    meshData.UV0.resize(NumVertices);
    meshData.normals.resize(NumVertices);
    meshData.tangents.resize(NumVertices);
    meshData.index.resize(NumIndices);

    const aiMesh* mesh = scene->mMeshes[meshId];
    {
        auto MeshVerticles = &meshData.verticles[meshes.baseVertex];
        auto MeshTexCords = &meshData.UV0[meshes.baseVertex];
        auto MeshNormals = &meshData.normals[meshes.baseVertex];
        auto MeshTangents = &meshData.tangents[meshes.baseVertex];

        for (size_t vertid = 0; vertid < mesh->mNumVertices; vertid++) {
            aiVector3D &vertex = mesh->mVertices[vertid];
            aiVector3D &normal = mesh->mNormals[vertid];

            if (mesh->mTextureCoords[0]) {
                aiVector3D &UVW = mesh->mTextureCoords[0][vertid]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
                MeshTexCords[vertid] = glm::fvec2(UVW.x, UVW.y);
            }
            else {
                MeshTexCords[vertid] = glm::fvec2();
            }
            if (mesh->mTangents) {
                aiVector3D &tangent = mesh->mTangents[vertid];
                MeshTangents[vertid] = glm::fvec3(tangent.x, tangent.y, tangent.z);
            }

            MeshVerticles[vertid] = glm::fvec3(vertex.x, vertex.y, vertex.z);
            MeshNormals[vertid] = glm::fvec3(normal.x, normal.y, normal.z);
        }

        auto meshIndices = &meshData.index[loadInfo.baseIndex];
        for (size_t face = 0; face < mesh->mNumFaces; face++) {
            aiFace *f = &mesh->mFaces[face];
            THROW_ASSERT(f->mNumIndices == 3, 0);
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
                    AddLogf(Warning, "Vertex %d cannot have more than 4 bones [%s]; Skipped influence of bone %s with weight %f", vertexid, subpath.c_str(), meshData.boneNames[boneIndex].c_str(), VertexWeight.mWeight);
                    continue;
                }

                meshData.vertexBones[vertexid][localboneid] = boneIndex;
                meshData.vertexBoneWeights[vertexid][localboneid] = VertexWeight.mWeight;
            }
        }
    }

    meshData.UpdateBoundary();

    owner.ApplyMeshSource(handle, std::move(meshData));
}

} //namespace MoonGlare::Renderer::Resources::Loader 
