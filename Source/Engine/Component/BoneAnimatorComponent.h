#pragma once

#include <Foundation/Component/iSubsystem.h>
#include <Foundation/Component/Entity.h>
#include <Foundation/Resources/SkeletalAnimation.h>

namespace MoonGlare::Component {

struct BoneAnimatorComponent {
    static constexpr uint32_t ComponentLimit = 64;
    static constexpr char* ComponentName = "BoneAnimator";

    uint8_t validBones = 0;
    Entity bones[Resources::Configuration::BoneCountLimit] = { };
    Resources::AnimationBlendState blendState = { };

    bool Load(ComponentReader &reader, Entity owner);
};

}
