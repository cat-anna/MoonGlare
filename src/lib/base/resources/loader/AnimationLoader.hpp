#pragma once

#include "async_loader.hpp"
#include "resources/skeletal_animation.hpp"
#include "resources/skeletal_animation_manager.hpp"

namespace MoonGlare::Resources::Loader {

class AnimationLoader : public MultiAsyncFileSystemRequest {
public:
    AnimationLoader(SkeletalAnimationHandle handle, SkeletalAnimationManager &Owner);
    ~AnimationLoader();

    void OnFirstFile(const std::string &requestedURI, std::string &filedata) override;

private:
    SkeletalAnimationManager &owner;
    SkeletalAnimationHandle handle;
};

} // namespace MoonGlare::Resources::Loader
