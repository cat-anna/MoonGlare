#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Configuration.h"

#include <Foundation/xMath.h>

namespace MoonGlare::Resources {

enum class SkeletalAnimationHandle : uint32_t {
    Invalid = 0,
};

template<typename T>
struct AnimationKey {
    using time_t = float;
    using item_t = T;

    time_t time;
    T value;
};

using PositionKey = AnimationKey<emath::fvec3>;
using ScalingKey = AnimationKey<emath::fvec3>;
using RotationKey = AnimationKey<emath::Quaternion>;

struct AnimationChannel {
    uint32_t positionKeyCount;
    PositionKey *positionKey;

    uint32_t rotationKeyCount;
    RotationKey *rotationKey;

    uint32_t scalingKeyCount;
    ScalingKey *scalingKey;

    //C_ENUM aiAnimBehaviour mPreState;
    //C_ENUM aiAnimBehaviour mPostState;
};

struct AnimationInfo {
    uint16_t firstFrame, endFrame;
    uint8_t loop;
    uint8_t __padding8;
};

struct SkeletalAnimation {
    float duration; //in ticks
    float ticksPerSecond;
    uint16_t animationNameOffset;

    uint16_t channelCount; //channel is movement of single bone
    AnimationChannel channel[Configuration::BoneCountLimit];
    uint16_t channelNameOffset[Configuration::BoneCountLimit];

    uint32_t animationSetCount;
    AnimationInfo animationSet[Configuration::AnimationSetLimit];
    uint16_t animationSetNameOffset[Configuration::AnimationSetLimit];

    const char *stringArrayBase;

    bool ready;
    uint32_t memoryBlockSize;
    void *memoryBlockFront; //from this pointer all other should be relative within range of memoryBlockSize

    bool CheckPointers() const {
        auto *memEnd = ((const uint8_t*)memoryBlockFront) + memoryBlockSize;
        auto check = [this, memEnd](const auto *ptr) -> bool {
            auto *p = (const uint8_t*)ptr;
            return p == nullptr || (p >= (const uint8_t*)memoryBlockFront && p < memEnd);
        };

        for (uint16_t i = 0; i < channelCount; ++i) {
            if (!check(channel[i].positionKey) || !check(channel[i].rotationKey) || !check(channel[i].scalingKey))
                return false;
        }
        
        return 
            check(stringArrayBase) &&
            check(memoryBlockFront);
    }

    void UpdatePointers(intptr_t newBase) {
        if (reinterpret_cast<intptr_t>(memoryBlockFront) == newBase)
            return;

        ptrdiff_t offset = reinterpret_cast<ptrdiff_t>(memoryBlockFront) - static_cast<ptrdiff_t>(newBase);

        auto update = [offset](auto *&ptr) {
            if (ptr == nullptr)
                return;
            ptrdiff_t newPtr = reinterpret_cast<ptrdiff_t>(ptr) - offset;
            ptr = reinterpret_cast<decltype(ptr)>(newPtr);
        };

        for (uint16_t i = 0; i < channelCount; ++i) {
            update(channel[i].positionKey);
            update(channel[i].rotationKey);
            update(channel[i].scalingKey);
        }
        update(stringArrayBase);
        update(memoryBlockFront);
    }
};                                   

struct AnimationBlendState {
    struct BoneState {
        uint16_t positionKey, rotationKey, scaleKey;
    };

    struct AnimationState {
        SkeletalAnimationHandle handle;
        uint16_t animationSetIndex;
        uint16_t validBones;
        float localTime;
        //float localSpeed;
        BoneState boneStates[Configuration::BoneCountLimit];
    };

    AnimationState state;// [Configuration::AnimatonStateLimit];

    void Invalidate() {
        state.animationSetIndex = 0;
        state.localTime = 0.0;
        state.validBones = 0;
        memset(state.boneStates, 0, sizeof(state.boneStates));
    };
};

struct AnimationLoopState {
    enum class LoopState : uint8_t {
        None, Loop, Finished, Error,
    };

    LoopState state;// [Configuration::AnimatonStateLimit];
};

struct BoneState {
    struct Bone {
        PositionKey::item_t position;
        ScalingKey::item_t scale;
        RotationKey::item_t rotation;
    };
    Bone bones[Configuration::BoneCountLimit];
};

} //namespace MoonGlare::Resources
