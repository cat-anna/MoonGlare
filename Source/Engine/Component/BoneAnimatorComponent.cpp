#include <pch.h>

#include <Foundation/Resources/SkeletalAnimationManager.h>

#include "BoneAnimatorComponent.h"

namespace MoonGlare::Component {

bool BoneAnimatorComponent::Load(ComponentReader &reader, Entity owner) {
    std::string uri = reader.node.child("Animation").text().as_string("");

    auto *sam = reader.manager->GetInterfaceMap().GetInterface<Resources::SkeletalAnimationManager>();

    Resources::SkeletalAnimationHandle h = {};
    h = sam->LoadAnimation(uri.c_str());
    blendState.state.handle = h;
    blendState.Invalidate();

    if (reader.localRelationsCount > 0) {
        //TODO: limit is 64
        memcpy(bones, reader.localRelations, reader.localRelationsCount * sizeof(bones[0]));
        validBones = (uint8_t)reader.localRelationsCount;
    }

    return true;
}

}
