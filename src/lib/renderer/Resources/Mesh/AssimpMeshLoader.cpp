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

#include <Foundation/Resources/Importer/AssimpMeshImporter.h>

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

    MeshSource ms;
    Importer::ImportAssimpMesh(scene, meshId, ms);

    owner.ApplyMeshSource(handle, std::move(ms));
}

} //namespace MoonGlare::Renderer::Resources::Loader 
