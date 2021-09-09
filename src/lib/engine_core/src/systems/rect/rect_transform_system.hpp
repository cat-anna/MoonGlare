#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"

namespace MoonGlare::Systems::Rect {

class RectTransformSystem : public ECS::SystemBase<RectTransformSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 16;
    static constexpr char kSystemName[] = "rect_transform_system";
    static constexpr bool kStepable = true;

    using SystemConfiguration = ECS::BaseSystemConfig;

    RectTransformSystem(const ECS::SystemCreateInfo &create_info, SystemConfiguration config_data);
    ~RectTransformSystem() override = default;

    //iSystem

protected:
    Component::ComponentRevision current_revision = 1;

    //iSystem
    void DoStep(double time_delta) override;
};

} // namespace MoonGlare::Systems::Rect
