#include <pch.h>
#include <nfMoonGlare.h>

#include <Component/TransformComponent.h>
#include <Core/Component/SubsystemManager.h>

#include "BoneAnimatorComponent.h"
#include "BoneAnimatorSystem.h"
#include "AnimationFinishedEvent.h"

namespace MoonGlare::Component {

BoneAnimatorSystem::BoneAnimatorSystem(iSubsystemManager * Owner) : subsystemManager(Owner) {
}

BoneAnimatorSystem::~BoneAnimatorSystem() {}

//------------------------------------------------------------------------------------------

bool BoneAnimatorSystem::Initialize() {
    auto *ssm = (Core::Component::SubsystemManager*)subsystemManager;
    transformComponent = ssm->GetComponent<TransformComponent>();
    if (!transformComponent) {
        AddLog(Error, "Failed to get TransformComponent instance!");
        return false;
    }

    subsystemManager->GetInterfaceMap().GetObject(skeletalAnimationManager);
    componentArray = &subsystemManager->GetComponentArray();
    return true;
}

void BoneAnimatorSystem::Step(const SubsystemUpdateData &conf) {
    componentArray->Visit<BoneAnimatorComponent>([this, &conf](Entity owner, BoneAnimatorComponent& item) {
        //TODO: visibility test

        Resources::BoneState boneState;
        Resources::AnimationLoopState r = {};

        r = skeletalAnimationManager->UpdateAnimation(item.blendState, (float)conf.timeDelta, &boneState);

        switch (r.state) {
        case Resources::AnimationLoopState::LoopState::Loop:
        case Resources::AnimationLoopState::LoopState::Finished: {
            AnimationFinishedEvent afe;
            afe.sender = owner;
            afe.recipient = owner;
            afe.loop = r.state == Resources::AnimationLoopState::LoopState::Loop;
            subsystemManager->GetEventDispatcher().Queue<>(afe);
            break;
        }

        case Resources::AnimationLoopState::LoopState::None:
            break;

        case Resources::AnimationLoopState::LoopState::Error:
        default:
            return;
        }

        for (size_t index = 0; index < item.validBones; ++index) {
            auto tri = transformComponent->GetComponentIndex(item.bones[index]);
            if (tri != ComponentIndex::Invalid) {
                auto &local = boneState.bones[index];
                transformComponent->SetPosition(tri, local.position);
                transformComponent->SetScale(tri, local.scale);
                transformComponent->SetRotation(tri, local.rotation);
            } else
                __debugbreak();
        }
    });
}

} 
