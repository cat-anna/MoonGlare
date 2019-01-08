#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Component/iSubsystem.h>

namespace MoonGlare::Component {

class TransformComponent;

class LightSystem : public iSubsystem {
public:
    static constexpr char* SystemName = "LightSystem";
    using SystemFlags = std::tuple<Flags::Required>;

    LightSystem(iSubsystemManager *Owner);
	virtual ~LightSystem();
	virtual bool Initialize() override;
	virtual void Step(const SubsystemUpdateData &conf) override;
private:
    TransformComponent *transformComponent = nullptr;
    ComponentArray *componentArray = nullptr;
    iSubsystemManager *subsystemManager = nullptr;
};

}
