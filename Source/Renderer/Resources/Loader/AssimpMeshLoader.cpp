#include "pch.h"

#define NEED_MESH_BUILDER
#define NEED_MATERIAL_BUILDER

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "../MeshResource.h"
#include "AssimpMeshLoader.h"

#include "../MaterialManager.h"

#include "FreeImageLoader.h"

namespace MoonGlare::Renderer::Resources::Loader {

void AssimpMeshLoader::OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) {
    importer = std::make_unique<Assimp::Importer>();

    auto loadflags =
        //aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        aiProcess_Triangulate |
        //aiProcess_GenUVCoords |
        //aiProcess_SortByPType |
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

void AssimpMeshLoader::LoadMeshes(ResourceLoadStorage &storage) {
    uint32_t NumVertices = 0, NumIndices = 0;

    if (scene->mNumMeshes > MeshConf::SubMeshLimit) {
        AddLog(Error, "SubMeshLimit exceeded");
        return;
    }

    MeshConf::SubMeshArray meshes;
    MeshConf::SubMeshMaterialArray materials;
    meshes.fill({});
    materials.fill({});

    struct LoadInfo {
        size_t baseIndex;
    };

    std::array<LoadInfo, meshes.size()> loadInfo;
    loadInfo.fill({});

    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        meshes[i].valid = true;
        meshes[i].numIndices = scene->mMeshes[i]->mNumFaces * 3;
        meshes[i].baseVertex = NumVertices;
        meshes[i].baseIndex = NumIndices * sizeof(uint32_t);
        meshes[i].elementMode = GL_TRIANGLES;
        meshes[i].indexElementType = GL_UNSIGNED_INT;

        loadInfo[i].baseIndex = NumIndices;

        NumVertices += scene->mMeshes[i]->mNumVertices;
        NumIndices += meshes[i].numIndices;
    }

    auto &m = storage.m_Memory.m_Allocator;

    glm::vec3 *verticles = m.Allocate<glm::vec3>(NumVertices);
    glm::vec3 *normals = m.Allocate<glm::vec3>(NumVertices);
    glm::vec2 *texCords = m.Allocate<glm::vec2>(NumVertices);
    uint32_t *indices = m.Allocate<uint32_t>(NumIndices);

    if (!verticles || !normals || !texCords || !indices) {
        throw NotEnoughStorage{
            NumVertices * (sizeof(glm::vec3) * 2 + sizeof(glm::vec2)) +
            NumIndices * sizeof(uint32_t)
        };
    }

    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];

        LoadMaterial(scene->mMeshes[i]->mMaterialIndex, materials[i], storage);

        auto MeshTexCords = texCords + meshes[i].baseVertex;
        auto MeshVerticles = verticles + meshes[i].baseVertex;
        auto MeshNormals = normals + meshes[i].baseVertex;

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

            MeshVerticles[vertid] = glm::fvec3(vertex.x, vertex.y, vertex.z);
            MeshNormals[vertid] = glm::fvec3(normal.x, normal.y, normal.z);
        }

        auto meshIndices = indices + loadInfo[i].baseIndex;
        for (size_t face = 0; face < mesh->mNumFaces; face++) {
            aiFace *f = &mesh->mFaces[face];
            THROW_ASSERT(f->mNumIndices == 3, 0);
            meshIndices[face * 3 + 0] = f->mIndices[0];
            meshIndices[face * 3 + 1] = f->mIndices[1];
            meshIndices[face * 3 + 2] = f->mIndices[2];
        }
    }

    auto builder = owner->GetBuilder(storage.m_Queue, handle);

    builder.subMeshArray = meshes;
    builder.subMeshMaterialArray = materials;

    using ichannels = Renderer::Configuration::VAO::InputChannels;

    builder.AllocateVAO();
    builder.vaoBuilder.BeginDataChange();

    builder.vaoBuilder.CreateChannel(ichannels::Vertex);
    builder.vaoBuilder.SetChannelData<float, 3>(ichannels::Vertex, (const float*)(verticles), NumVertices);

    builder.vaoBuilder.CreateChannel(ichannels::Texture0);
    builder.vaoBuilder.SetChannelData<float, 2>(ichannels::Texture0, (const float*)(texCords), NumVertices);

    builder.vaoBuilder.CreateChannel(ichannels::Normals);
    builder.vaoBuilder.SetChannelData<float, 3>(ichannels::Normals, (const float*)(normals), NumVertices);

    builder.vaoBuilder.CreateChannel(ichannels::Index);
    builder.vaoBuilder.SetIndex(ichannels::Index, (const unsigned*)(indices), NumIndices);

    builder.vaoBuilder.EndDataChange();
    builder.vaoBuilder.UnBindVAO();
}

void AssimpMeshLoader::LoadMaterial(unsigned index, MaterialResourceHandle &h, ResourceLoadStorage &storage) {
    if (index >= scene->mNumMaterials) {
        AddLogf(Error, "Invalid material index");
        return;
    }

    const aiMaterial* aiMat = scene->mMaterials[index];
    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) <= 0) {
        AddLogf(Error, "No diffuse component");
        return;
    }

    aiString Path;
    if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
        AddLogf(Error, "Unable to load material");
        return;
    }

    if (Path.data[0] == '*') {
        auto matb = materialManager.GetMaterialBuilder(h, true);
        matb.SetDiffuseMap();
        matb.SetDiffuseColor(emath::fvec4(1));

        //internal texture
        auto idx = strtoul(Path.data + 1, nullptr, 10);
        if (idx >= scene->mNumTextures) {
            AddLogf(Error, "Invalid internal texture id!");
            return;
        }

        auto texptr = scene->mTextures[idx];

        if (texptr->mHeight == 0) {
            //raw image bytes
            auto matH = matb.m_MaterialPtr->m_DiffuseMap;

            PostTask([matH, texptr](ResourceLoadStorage &storage) {
                FreeImageLoader::LoadTexture(storage, matH, texptr->pcData, texptr->mWidth);
            });
        }
        else {
            AddLogf(Error, "Not compressed inner texture are not supported!");
        }
        return;
    }
    else {
        auto matb = materialManager.GetMaterialBuilder(h, true);
        matb.SetDiffuseMap(baseURI + Path.data);
        matb.SetDiffuseColor(emath::fvec4(1));
    }
}

} //namespace MoonGlare::Renderer::Resources::Loader 
