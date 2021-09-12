#pragma once

#include "systems/register_systems.hpp"
#include "ecs/system_info.hpp"
#include "global_matrix_system.hpp"
#include "rect/rect_image.hpp"
#include "rect/rect_transform_debug_draw_system.hpp"
#include "rect/rect_transform_system.hpp"
#include "transform_system.hpp"

namespace MoonGlare::Systems {

void RegisterAllSystems(ECS::ECSRegister &ecs_register) {
    ecs_register.RegisterSystem<GlobalMatrixSystem>();

    ecs_register.RegisterSystem<TransformSystem>();

    ecs_register.RegisterSystem<Rect::RectTransformSystem>();
    ecs_register.RegisterSystem<Rect::RectTransformDebugDrawSystem>();
    ecs_register.RegisterSystem<Rect::RectImageSystem>();
}

} // namespace MoonGlare::Systems