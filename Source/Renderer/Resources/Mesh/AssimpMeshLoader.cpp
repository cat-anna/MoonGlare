#include "pch.h"

#define NEED_MESH_BUILDER

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "MeshResource.h"
#include "AssimpMeshLoader.h"

#include "../MaterialManager.h"

#include "../Texture/FreeImageLoader.h"

#include <Commands/MemoryCommands.h>
#include "MeshUpdate.h"

namespace MoonGlare::Renderer::Resources::Loader {

void AssimpMeshLoader::OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) {
    importer = std::make_unique<Assimp::Importer>();

    auto loadflags =
        aiProcess_JoinIdenticalVertices |
        //aiProcess_PreTransformVertices |
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

    if (!materialURI.empty()) {
        if (materialURI.find(ModelURI) != 0)
            customMaterial = true;
         else 
            materialURI = materialURI.substr(ModelURI.size() + 1);
    }
                      
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

int AssimpMeshLoader::GetMaterialIndex() const {
    static constexpr std::string_view proto = "material://";

    if (materialURI.empty())
    {
        return -1;
    }
      
    if (materialURI.find(proto) != 0) {
        __debugbreak();
        return -1;
    }

    const char *beg = materialURI.c_str() + proto.size();

    if (*beg == '*') {
        ++beg;
        char *end = nullptr;
        long r = strtol(beg, &end, 10);
        if (end == beg) {
            __debugbreak();
            return -1;
        }
        if (r > (int)scene->mNumMaterials)
            return -1;
        return r;
    }

    return -1;
}

void AssimpMeshLoader::LoadMeshes(ResourceLoadStorage &storage) {
    uint32_t NumVertices = 0, NumIndices = 0;

    struct LoadInfo {
        size_t baseIndex;
    };

    Mesh meshes = {};
    MaterialResourceHandle material = {};

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

    MeshData meshData;
    meshData.verticles.resize(NumVertices);
    meshData.UV0.resize(NumVertices);
    meshData.normals.resize(NumVertices);
    meshData.index.resize(NumIndices);

    //for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        const aiMesh* mesh = scene->mMeshes[meshId];

        //TODO: check for success

        if (!customMaterial) {
            materialManager.Allocate(material);
            LoadMaterial(mesh->mMaterialIndex, material, storage);
        } else {
            material = materialManager.LoadMaterial(materialURI);
        }

        auto MeshVerticles = &meshData.verticles[meshes.baseVertex];
        auto MeshTexCords = &meshData.UV0[meshes.baseVertex];
        auto MeshNormals = &meshData.normals[meshes.baseVertex];

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

        auto meshIndices = &meshData.index[loadInfo.baseIndex];
        for (size_t face = 0; face < mesh->mNumFaces; face++) {
            aiFace *f = &mesh->mFaces[face];
            THROW_ASSERT(f->mNumIndices == 3, 0);
            meshIndices[face * 3 + 0] = f->mIndices[0];
            meshIndices[face * 3 + 1] = f->mIndices[1];
            meshIndices[face * 3 + 2] = f->mIndices[2];
        }
    }

    meshData.UpdateBoundary();
    owner.SetMeshData(handle, std::move(meshData));

    auto task = std::make_shared<Renderer::Resources::Loader::CustomMeshLoader>(handle, owner);
    task->materialArray = material;
    task->meshArray = meshes;
    loader->QueueTask(std::move(task));
}

void AssimpMeshLoader::LoadMaterial(unsigned index, MaterialResourceHandle h, ResourceLoadStorage &storage) {
    if (!materialURI.empty()) {
        auto m = GetMaterialIndex();
        if (m >= 0)
            index = m;
    }

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

#if 0
    Configuration::TextureLoad TexConfig = Configuration::TextureLoad::Default();

    TexConfig.m_Edges = Configuration::Texture::Edges::Repeat; //TODO: read from material

    if (Path.data[0] == '*') {
        auto matb = materialManager.GetMaterialBuilder(h, false);
        matb.SetDiffuseMap();
        matb.SetDiffuseColor(emath::fvec4(1,1,1,1));

        //internal texture
        auto idx = strtoul(Path.data + 1, nullptr, 10);
        if (idx >= scene->mNumTextures) {
            AddLogf(Error, "Invalid internal texture id!");
            return;
        }

        auto texptr = scene->mTextures[idx];

        if (texptr->mHeight == 0) {
            //raw image bytes
            auto matH = matb.m_MaterialPtr->mapTexture[0];
//TODO: not supported
            __debugbreak();
            //PostTask([matH, texptr, TexConfig](ResourceLoadStorage &storage) {
            //    Texture::FreeImageLoader::LoadTexture(storage, matH, texptr->pcData, texptr->mWidth, TexConfig);
            //});
        }
        else {
            AddLogf(Error, "Not compressed inner texture are not supported!");
        }
        return;
    }
    else {
        auto matb = materialManager.GetMaterialBuilder(h, true);
        matb.SetDiffuseMap(baseURI + Path.data, TexConfig);
        matb.SetDiffuseColor(emath::fvec4(1,1,1,1));
    }
#endif
}

} //namespace MoonGlare::Renderer::Resources::Loader 
