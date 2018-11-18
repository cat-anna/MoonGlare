#pragma once

#include <Foundation/Resources/SkeletalAnimation.h>
#include <Foundation/Resources/SkeletalAnimationManager.h>
#include <Foundation/Resources/iAsyncLoader.h>

namespace MoonGlare::Resources::Loader {

class AnimationLoader : public MultiAsyncFileSystemRequest
{
public:
    AnimationLoader(SkeletalAnimationHandle handle, SkeletalAnimationManager &Owner);
    ~AnimationLoader();

    void OnFirstFile(const std::string &requestedURI, StarVFS::ByteTable &filedata) override;
private:
    SkeletalAnimationManager &owner;
    SkeletalAnimationHandle handle;
};

} 
