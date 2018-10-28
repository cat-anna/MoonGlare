#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Component/iSubsystem.h>

namespace MoonGlare::Component {     

class TransformComponent;

class MeshSystem : public iSubsystem {
public:
    static constexpr char* SystemName = "MeshSystem";
    using SystemFlags = std::tuple<Flags::Required>;

    MeshSystem(iSubsystemManager *Owner);
    virtual ~MeshSystem();
    virtual bool Initialize() override;
    virtual void Step(const SubsystemUpdateData &conf) override;
private:
    TransformComponent *transformComponent = nullptr;
    ComponentArray *componentArray = nullptr;
    iSubsystemManager *subsystemManager = nullptr;
};

}
