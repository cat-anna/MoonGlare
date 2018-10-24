#include <pch.h>
#include <nfMoonGlare.h>

#include "DirectAnimationComponent.h"

#include <Core/Component/ComponentRegister.h>
#include <Component/TransformComponent.h>

#include <DirectAnimationComponent.x2c.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

RegisterComponentID<DirectAnimationComponent> DirectAnimationComponentReg("DirectAnimation");

DirectAnimationComponent::DirectAnimationComponent(SubsystemManager * Owner)
    : TemplateStandardComponent(Owner)
    , m_TransformComponent(nullptr) {}

DirectAnimationComponent::~DirectAnimationComponent() {}

//------------------------------------------------------------------------------------------

bool DirectAnimationComponent::Initialize() {
    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }

    skeletalAnimationManager = GetManager()->GetWorld()->GetInterface<Resources::SkeletalAnimationManager>();
    if (!skeletalAnimationManager) {
        AddLog(Error, "Failed to get skeletalAnimationManager instance!");
        return false;
    }
    return true;
}

bool DirectAnimationComponent::Finalize() {
    return true;
}

void DirectAnimationComponent::Step(const Core::MoveConfig &conf) {
    for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
        auto &item = m_Array[i];

        if (!item.m_Flags.m_Map.m_Valid) {
            continue;
        }

        Resources::BoneState boneState;
        Resources::AnimationLoopState r = {};

        r = skeletalAnimationManager->UpdateAnimation(item.blendState, conf.timeDelta, &boneState);

        //static const char animName[64] = "walk";
        static const char animName[64] = "run scared";

        if (r.state == Resources::AnimationLoopState::LoopState::Finished)
            skeletalAnimationManager->ResetBlendState(item.blendState, animName, &boneState);

        for (size_t index = 0; index < item.validBones; ++index) {
            auto tri = m_TransformComponent->GetComponentIndex(item.bones[index]);
            if (tri != ComponentIndex::Invalid) {
                auto &local = boneState.bones[index];
                m_TransformComponent->SetPosition(tri, local.position);
                m_TransformComponent->SetScale(tri, local.scale);
                m_TransformComponent->SetRotation(tri,local.rotation);
            } else
                __debugbreak();
        }
    }
}

bool DirectAnimationComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
    x2c::Component::DirectAnimationComponent::DirectAnimationEntry_t dae;
    dae.ResetToDefault();

    if (!reader.Read(dae)) {
        AddLogf(Error, "Failed to read DirectAnimationEntry entry!");
        return false;
    }

    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();
    entry.Reset();

    entry.m_Owner = owner;
    entry.m_Flags.m_Map.m_Valid = true;

    auto uri = dae.m_AnimationFile + "@animation://*2";

    Resources::SkeletalAnimationHandle h = {};
    h = skeletalAnimationManager->LoadAnimation(uri.c_str());
    entry.blendState.state.handle = h;  
    entry.blendState.Invalidate();

    if (reader.localRelationsCount > 0) {
        //TODO: limit is 64
        memcpy(entry.bones, reader.localRelations, reader.localRelationsCount * sizeof(entry.bones[0]));
        entry.validBones = reader.localRelationsCount;
    }

    //auto *anim = scene->mAnimations[entry.m_CurrentAnimIndex];
    //for (size_t bone = 0; bone < anim->mNumChannels; ++bone) {
    //    auto e = m_TransformComponent->FindChildByName(owner, anim->mChannels[bone]->mNodeName.data);
    //    if (!e.has_value()) {
    //        __debugbreak();
    //    }
    //    entry.bones[bone] = e.value_or(Entity{});
    //}

    return true;
}

} 
