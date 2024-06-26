#pragma once

#include <Foundation/Resources/SkeletalAnimationManager.h>

namespace MoonGlare::Component {

class TransformComponent;

class BoneAnimatorSystem : public iSubsystem {
public:
    static constexpr char* SystemName = "BoneAnimatorSystem";
    using SystemFlags = std::tuple<Flags::Required>;

    BoneAnimatorSystem(iSubsystemManager *Owner);
    virtual ~BoneAnimatorSystem();
    virtual bool Initialize() override;
    virtual void Step(const SubsystemUpdateData &conf) override;
private:
    Resources::SkeletalAnimationManager* skeletalAnimationManager = nullptr;
    ComponentArray *componentArray = nullptr;
    TransformComponent *transformComponent = nullptr;
    iSubsystemManager *subsystemManager = nullptr;
};

} 
