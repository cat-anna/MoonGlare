
#include "resources/importer/AssimpAnimationImporter.hpp"

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#pragma warning(pop)

#include "AssimpAnimationLoader.hpp"
#include <aligned_ptr.hpp>

namespace MoonGlare::Resources::Loader {

AssimpAnimationLoader::AssimpAnimationLoader(std::string subpath, SkeletalAnimationHandle handle,
                                             SkeletalAnimationManager &Owner)
    : owner(Owner), handle(handle), subpath(std::move(subpath)) {
}

AssimpAnimationLoader::~AssimpAnimationLoader() {
}

void AssimpAnimationLoader::OnFirstFile(const std::string &requestedURI, std::string &filedata) {
    importer = std::make_unique<Assimp::Importer>();

    auto loadflags =
        // aiProcessPreset_TargetRealtime_Fast |
        aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_GenNormals |
        aiProcess_LimitBoneWeights |
        // aiProcess_ImproveCacheLocality |
        // aiProcess_PreTransformVertices |
        aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType | aiProcess_GlobalScale | 0;

    scene =
        importer->ReadFileFromMemory(filedata.data(), filedata.size(), loadflags, strrchr(requestedURI.c_str(), '.'));

    if (!scene) {
        AddLog(Error,
               fmt::format("Unable to animation model file[{}]. Error: {}", requestedURI, importer->GetErrorString()));
        return;
    }

    ModelURI = requestedURI;
    baseURI = requestedURI;
    baseURI.resize(baseURI.rfind('/') + 1);

    LoadFile(requestedURI + ".xml", [this](const std::string &, std::string &filedata) {
        if (!animSetXmlDoc.load_string(filedata.c_str())) {
            // TODO: sth
            __debugbreak();
        }
        LoadAnimation();
    });
}

int AssimpAnimationLoader::GetAnimIndex() const {
    static constexpr std::string_view meshProto = "animation://";

    if (subpath.empty()) {
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

void AssimpAnimationLoader::LoadAnimation() {
    int animId = GetAnimIndex();
    if (animId < 0) {
        __debugbreak();
        return;
    }

    Importer::AnimationImport ai;

    Importer::ImportAssimpAnimation(scene, animId, animSetXmlDoc.document_element(), ai);
    owner.ApplyAnimationData(handle, std::move(ai.memory), ai.animation);
}

} // namespace MoonGlare::Resources::Loader
