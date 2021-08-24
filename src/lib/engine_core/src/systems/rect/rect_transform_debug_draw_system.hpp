#pragma once

#include "core/stepable_interface.hpp"
#include "ecs/system_interface.hpp"

namespace MoonGlare::Systems::Rect {

class RectTransformDebugDrawSystem : public ECS::SystemBase<RectTransformDebugDrawSystem> {
public:
    static constexpr ECS::SystemId kSystemId = 17;
    static constexpr char kSystemName[] = "rect_transform_debug_draw_system";
    static constexpr bool kStepable = true;

    using SystemConfiguration = ECS::BaseSystemConfig;

    RectTransformDebugDrawSystem(const ECS::SystemCreateInfo &create_info,
                                 SystemConfiguration config_data)
        : SystemBase(create_info, config_data) {}
    ~RectTransformDebugDrawSystem() override = default;

    //iSystem

protected:
    //iSystem
    void DoStep(double time_delta) override;

#if 0
        void DebugDraw(const Core::MoveConfig &conf, RectTransformComponent *Component);
private:
    bool ready = false;
    Renderer::ShaderResourceHandleBase shaderHandle;
#endif
};

} // namespace MoonGlare::Systems::Rect
