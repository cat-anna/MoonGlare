
#include "SkeletalAnimationManager.h"
#include "iAsyncLoader.h"

#include "Loader/AssimpAnimationLoader.h" 
#include "Loader/AnimationLoader.h" 

#include <Foundation/Resources/Blob/AnimationBlob.h>

namespace MoonGlare::Resources {

union SkeletalAnimationHandleDetail {
    SkeletalAnimationHandle handle;
    uint32_t u32Value;
    struct {
        uint16_t generation;
        uint16_t index;
    };
};

SkeletalAnimationManager::SkeletalAnimationManager(InterfaceMap &ifaceMap) {
    loader = ifaceMap.GetInterface<iAsyncLoader>();               

    generations.fill(1);
    animation.fill({});
    allocationBitmap.ClearAllocation();
}

SkeletalAnimationManager::~SkeletalAnimationManager(){ }

//-------------------------------------------------------------------------------------------------

SkeletalAnimationHandle SkeletalAnimationManager::Allocate() {
    Bitmap::Index_t index;
    if (allocationBitmap.Allocate(index)) {
        SkeletalAnimationHandleDetail hout;
        hout.index = static_cast<uint16_t>(index);
        hout.generation = generations[index];
        return hout.handle;
    } else {
        AddLogf(Debug, "skeletal animation allocation failed");
        return {};
    }
}

std::pair<SkeletalAnimationHandle, bool> SkeletalAnimationManager::Allocate(const std::string &uri) {
    auto cache = loadedAnimations.find(uri);
    if (cache != loadedAnimations.end() && IsHandleValid(cache->second)) {
        AddLogf(Performance, "skeletal animation load cache hit");
        return { cache->second, true };
    }
    SkeletalAnimationHandle hout = Allocate();
    loadedAnimations[uri] = hout;
    return { hout, false };
}

bool SkeletalAnimationManager::IsHandleValid(SkeletalAnimationHandle &h) const {
    SkeletalAnimationHandleDetail hd;
    hd.handle = h;

    if (hd.index >= Configuration::AnimationLimit)
        return false;
    if (generations[hd.index] != hd.generation) 
        return false;
    
    return true;
}

//-------------------------------------------------------------------------------------------------

SkeletalAnimationHandle SkeletalAnimationManager::LoadAnimation(const std::string &uri) {
    auto[h, got] = Allocate(uri);
    if (got)
        return h;

    if (uri.find(".anim") == uri.size() - 5) {
        //lazy suffix check
        auto request = std::make_shared<Loader::AnimationLoader>(h, *this);
        loader->QueueRequest(uri, request);
        return h;
    }

    std::string subpath;
    std::string fileuri;

    auto pos = uri.find('@');
    if (pos == std::string::npos) {
        fileuri = uri;
    } else {
        fileuri = uri.substr(0, pos);
        subpath = uri.substr(pos + 1);
    }

    auto request = std::make_shared<Loader::AssimpAnimationLoader>(std::move(subpath), h, *this);
    loader->QueueRequest(fileuri, request);

    return h;
}   

void SkeletalAnimationManager::ApplyAnimationData(SkeletalAnimationHandle handle, std::unique_ptr<uint8_t[]> memory, const SkeletalAnimation &animInfo) {
#ifdef DEBUG_DUMP
    Resources::Blob::DumpAnimationBlob(animInfo);
#endif 

    if (!IsHandleValid(handle)) {
        //TODO: log sth?
        return;
    }
       
    SkeletalAnimationHandleDetail hd;
    hd.handle = handle;

    animation[hd.index].ready = false;
    animationDataMemory[hd.index].swap(memory);
    animation[hd.index] = animInfo;
    animation[hd.index].ready = true;
}

const SkeletalAnimation *SkeletalAnimationManager::GetAnimationData(SkeletalAnimationHandle handle) {
    if (!IsHandleValid(handle)) {
        //TODO: log sth?
        return nullptr;
    }

    SkeletalAnimationHandleDetail hd;
    hd.handle = handle;
    return &animation[hd.index];
}

//-------------------------------------------------------------------------------------------------   

void SkeletalAnimationManager::ResetBlendState(AnimationBlendState &state, const char *animSetName, BoneState *boneStates) {
    auto &as = state.state;
    auto *adata = GetAnimationData(as.handle);
    if (!adata)
        return;

    state.Invalidate();
    as.validBones = adata->channelCount;

    for (size_t i = 0; i < adata->animationSetCount; ++i) {
        if (strcmp(animSetName, adata->stringArrayBase + adata->animationSetNameOffset[i]) == 0) {
            as.animationSetIndex = (decltype(as.animationSetIndex))i;
            break;
        }
    }

    if(boneStates)
        UpdateAnimation(state, 0, boneStates);
}

AnimationLoopState SkeletalAnimationManager::UpdateAnimation(AnimationBlendState &state, float dt, BoneState *boneStates) {
    auto &as = state.state;
    auto *adata = GetAnimationData(as.handle);
    if (!adata)
        return { AnimationLoopState::LoopState::Error };

    if(as.animationSetIndex >= adata->animationSetCount)
        return { AnimationLoopState::LoopState::Finished };

    auto &aset = adata->animationSet[as.animationSetIndex];
    float frameCount = static_cast<float>(aset.endFrame - aset.firstFrame + 1);

    as.localTime += dt / (frameCount / adata->ticksPerSecond);
    bool loop = false;
    bool done = false;

    if (aset.loop) {
        loop = as.localTime > 1.0;
        as.localTime = fmod(as.localTime, 1.0f);
    } else {
        done = as.localTime > 1.0;
        if (done)
            as.localTime = 1.0f;
    }

    float frameTime = aset.firstFrame + as.localTime * frameCount;

    auto endFramePrim = aset.endFrame - 1.0e-4f;
    auto firstFramePrim = aset.firstFrame - 1.0e-4f;

    for (unsigned int a = 0; a < adata->channelCount; a++) {
        auto& channel = adata->channel[a];

        PositionKey::item_t presentPosition(0, 0, 0);
        if (channel.positionKeyCount > 0) {
            unsigned int frame = loop ? as.boneStates[a].positionKey : 0;
            while (frame < channel.positionKeyCount - 1) {
                if (frameTime < channel.positionKey[frame + 1].time)
                    break;
                frame++;
            }

            unsigned int nextFrame = (frame + 1) % channel.positionKeyCount;

            if (channel.positionKey[nextFrame].time > endFramePrim) {
                nextFrame = 0;
                while (nextFrame < channel.positionKeyCount - 1) {
                    if (firstFramePrim < channel.positionKey[nextFrame + 1].time)
                        break;
                    nextFrame++;
                }
            }

            const auto& key = channel.positionKey[frame];
            const auto& nextKey = channel.positionKey[nextFrame];
            double diffTime = nextKey.time - key.time;
            if (diffTime < 0.0)
                diffTime += frameCount;
            if (diffTime > 0) {
                float factor = float((frameTime - key.time) / diffTime);
                presentPosition = key.value + (nextKey.value - key.value) * factor;
            } else {
                presentPosition = key.value;
            }

            as.boneStates[a].positionKey = static_cast<uint16_t>(frame);
        }

        RotationKey::item_t presentRotation(0, 0, 0, 1);
        if (channel.rotationKeyCount > 0) {
            unsigned int frame = loop ? as.boneStates[a].rotationKey : 0;
            while (frame < channel.rotationKeyCount - 1) {
                if (frameTime < channel.rotationKey[frame + 1].time)
                    break;
                frame++;
            }

            unsigned int nextFrame = (frame + 1) % channel.rotationKeyCount;

            if (channel.rotationKey[nextFrame].time > endFramePrim) {
                nextFrame = 0;
                while (nextFrame < channel.rotationKeyCount - 1) {
                    if (firstFramePrim < channel.rotationKey[nextFrame + 1].time)
                        break;
                    nextFrame++;
                }
            }

            const auto& key = channel.rotationKey[frame];
            const auto& nextKey = channel.rotationKey[nextFrame];
            double diffTime = nextKey.time - key.time;
            if (diffTime < 0.0)
                diffTime += frameCount;
            if (diffTime > 0) {
                float factor = float((frameTime - key.time) / diffTime);
                presentRotation = key.value.slerp(factor, nextKey.value);
            } else {
                presentRotation = key.value;
            }

            as.boneStates[a].rotationKey = static_cast<uint16_t>(frame);
        }

        ScalingKey::item_t presentScaling(1, 1, 1);
        if (channel.scalingKeyCount > 0) {
            unsigned int frame = loop ? as.boneStates[a].scaleKey : 0;
            while (frame < channel.scalingKeyCount - 1) {
                if (frameTime < channel.scalingKey[frame + 1].time)
                    break;
                frame++;
            }

            presentScaling = channel.scalingKey[frame].value;
            as.boneStates[a].scaleKey = static_cast<uint16_t>(frame);
        }

        if (boneStates) {
            auto &b = boneStates->bones[a];
            b.position = presentPosition;
            b.rotation = presentRotation;
            b.scale = presentScaling;
        }
    }

    if (loop)
        return { AnimationLoopState::LoopState::Loop };
    if(done)
        return { AnimationLoopState::LoopState::Finished};
    return { AnimationLoopState::LoopState::None };
}

}
