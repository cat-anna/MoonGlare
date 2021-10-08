#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"

namespace MoonGlare::Systems {

class GlobalMatrixSystem : public ECS::SystemBase<GlobalMatrixSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 0;
    static constexpr ECS::SystemOrder kOrder = 10;
    static constexpr char kSystemName[] = "global_matrix_system";
    static constexpr bool kStepable = true;

    using SystemConfiguration = ECS::BaseSystemConfig;

    GlobalMatrixSystem(const ECS::SystemCreateInfo &create_info, SystemConfiguration config_data)
        : SystemBase(create_info, config_data) {}
    ~GlobalMatrixSystem() override = default;

    //iSystem

protected:
    //iSystem
    void DoStep(double time_delta) override;
};

} // namespace MoonGlare::Systems
