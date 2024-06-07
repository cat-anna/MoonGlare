#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"

namespace MoonGlare::Systems {

#if 0

class CameraSystem : public ECS::SystemBase<CameraSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 3;
    static constexpr ECS::SystemOrder kOrder = 0;
    static constexpr char kSystemName[] = "camera_system";
    static constexpr bool kStepable = true;

    using SystemConfiguration = ECS::BaseSystemConfig;

    GlobalMatrixSystem(const ECS::SystemCreateInfo &create_info, SystemConfiguration config_data)
        : SystemBase(create_info, config_data) {}
    ~CameraSystem() override = default;

    //iSystem

protected:
    //iSystem
    void DoStep(double time_delta) override;
};

#endif

} // namespace MoonGlare::Systems
