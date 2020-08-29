#include "AnimationLoader.hpp"
#include "resources/blob/animation_blob.hpp"
#include <orbit_logger.h>

namespace MoonGlare::Resources::Loader {

AnimationLoader::AnimationLoader(SkeletalAnimationHandle handle, SkeletalAnimationManager &Owner)
    : owner(Owner), handle(handle) {
}

AnimationLoader::~AnimationLoader() {
}

void AnimationLoader::OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata) {
    // int animId = GetAnimIndex();
    // if (animId < 0) {
    //    __debugbreak();
    //    return;
    //}

    // Importer::AnimationImport ai;
    // Importer::ImportAssimpAnimation(scene, animId, animSetXmlDoc.document_element(), ai);

    Resources::Blob::AnimationLoad al;
    auto success = Resources::Blob::ReadAnimationBlob(gsl::span<uint8_t>{filedata.get8(), filedata.byte_size()}, al);

    if (!success) {
        AddLogf(Error, "Failed to load animation %s", requestedURI.c_str());
        return;
    }

    owner.ApplyAnimationData(handle, std::move(al.memory), al.animation);
}

} // namespace MoonGlare::Resources::Loader