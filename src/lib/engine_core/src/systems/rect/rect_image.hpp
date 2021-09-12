#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"

namespace MoonGlare::Systems::Rect {

class RectImageSystem : public ECS::SystemBase<RectImageSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 18;
    static constexpr char kSystemName[] = "rect_image";
    static constexpr bool kStepable = true;

    using SystemConfiguration = ECS::BaseSystemConfig;

    RectImageSystem(const ECS::SystemCreateInfo &create_info, SystemConfiguration config_data);
    ~RectImageSystem() override = default;

    //iSystem

protected:
    //iSystem
    void DoStep(double time_delta) override;
};

} // namespace MoonGlare::Systems::Rect
