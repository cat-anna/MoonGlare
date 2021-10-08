#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"

namespace MoonGlare::Systems {

class TransformSystem : public ECS::SystemBase<TransformSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 4;
    static constexpr ECS::SystemOrder kOrder = 0;
    static constexpr char kSystemName[] = "transform_system";
    static constexpr bool kStepable = true;

    using SystemConfiguration = ECS::BaseSystemConfig;

    TransformSystem(const ECS::SystemCreateInfo &create_info, SystemConfiguration config_data)
        : SystemBase(create_info, config_data) {}
    ~TransformSystem() override = default;

    //iSystem

protected:
    //iSystem
    void DoStep(double time_delta) override;
};

} // namespace MoonGlare::Systems
