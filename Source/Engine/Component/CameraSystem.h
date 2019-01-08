#pragma once

#include <Foundation/Component/Entity.h>
#include <Foundation/Component/iSubsystem.h>

#include <Renderer/VirtualCamera.h>

namespace MoonGlare::Component {           

class TransformComponent;

class CameraSystem : public iSubsystem {
public:
    static constexpr char* SystemName = "CameraSystem";
    using SystemFlags = std::tuple<Flags::Required>;

    CameraSystem(iSubsystemManager *Owner);
	virtual ~CameraSystem();

	virtual bool Initialize() override;
	virtual void Step(const SubsystemUpdateData &conf) override;

    void ActivateCamera(Entity cameraOwner);
protected:
	TransformComponent *transformComponent = nullptr;
    ComponentArray *componentArray = nullptr;
    iSubsystemManager *subsystemManager = nullptr;
    bool gotActiveCamera = false;
    Entity activeCamera = { };

    bool SubmitCamera(Entity cameraEntity, const SubsystemUpdateData &xconf);
};

}

