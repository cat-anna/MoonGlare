#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"
#include "renderer/resources.hpp"

namespace MoonGlare::Systems::Rect {

class RectTransformDebugDrawSystem : public ECS::SystemBase<RectTransformDebugDrawSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 17;
    static constexpr ECS::SystemOrder kOrder = 100;
    static constexpr char kSystemName[] = "rect_transform_debug_draw_system";
    static constexpr bool kStepable = true;

    static constexpr size_t kInitialElementCount = 64;

    using SystemConfiguration = ECS::BaseSystemConfig;

    RectTransformDebugDrawSystem(const ECS::SystemCreateInfo &create_info,
                                 SystemConfiguration config_data);
    ~RectTransformDebugDrawSystem() override = default;

    //iSystem

protected:
    size_t last_known_element_count = kInitialElementCount;

    Renderer::ShaderHandle shader_handle;

    //iSystem
    void DoStep(double time_delta) override;
};

} // namespace MoonGlare::Systems::Rect
