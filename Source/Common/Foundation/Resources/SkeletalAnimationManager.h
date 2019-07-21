#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <Foundation/InterfaceMap.h>
#include <Memory/BitampAllocator.h>

#include "SkeletalAnimation.h"        

namespace MoonGlare::Resources {

class iAsyncLoader;

class SkeletalAnimationManager final {
public:
    SkeletalAnimationManager(InterfaceMap &ifaceMap);
    ~SkeletalAnimationManager();

    SkeletalAnimationHandle LoadAnimation(const std::string &uri);
    bool IsHandleValid(SkeletalAnimationHandle &h) const;

    void ApplyAnimationData(SkeletalAnimationHandle handle, std::unique_ptr<uint8_t[]> memory, const SkeletalAnimation &animInfo);

    const SkeletalAnimation *GetAnimationData(SkeletalAnimationHandle handle);

    void ResetBlendState(AnimationBlendState &state, const char *animSetName, BoneState *boneStates);

    AnimationLoopState UpdateAnimation(AnimationBlendState &state, float dt, BoneState *boneStates);
private:
    template<typename T>
    using Array = std::array<T, Configuration::AnimationLimit>;

    Array<SkeletalAnimation> animation;
    Array<std::unique_ptr<uint8_t[]>> animationDataMemory;
    Array<uint16_t> generations;

    using Bitmap = Memory::LinearAtomicBitmapAllocator<Configuration::AnimationLimit>;
    Bitmap allocationBitmap;

    iAsyncLoader* loader;

    std::unordered_map<std::string, SkeletalAnimationHandle> loadedAnimations; //temporary solution

    SkeletalAnimationHandle Allocate();
    std::pair<SkeletalAnimationHandle, bool> Allocate(const std::string &uri);
};

} //namespace MoonGlare::Resources
