#pragma once

#include <SkeletalAnimation.h>
#include <SkeletalAnimationManager.h>
#include <iAsyncLoader.h>

namespace MoonGlare::Resources::Loader {

class AnimationLoader : public MultiAsyncFileSystemRequest {
public:
    AnimationLoader(SkeletalAnimationHandle handle, SkeletalAnimationManager &Owner);
    ~AnimationLoader();

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata) override;

private:
    SkeletalAnimationManager &owner;
    SkeletalAnimationHandle handle;
};

} // namespace MoonGlare::Resources::Loader
