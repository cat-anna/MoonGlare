#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Component/iSubsystem.h>

namespace MoonGlare::Component {     

class TransformComponent;

class SkinSystem : public iSubsystem {
public:
    static constexpr char* SystemName = "SkinSystem";
    using SystemFlags = std::tuple<Flags::Required>;


    SkinSystem(iSubsystemManager *Owner);
    virtual ~SkinSystem();
    virtual bool Initialize() override;
    virtual void Step(const SubsystemUpdateData &conf) override;
private:
    iSubsystemManager *subsystemManager;
    TransformComponent *transformComponent;
};

}
