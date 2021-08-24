#pragma once

#define WANTS_SERIALIZATION

#include "ecs/ecs_register.hpp"
#include <component/camera.hpp>
#include <component/global_matrix.hpp>
#include <component/light_source.hpp>
#include <component/mesh.hpp>
#include <component/name.hpp>
#include <component/parent.hpp>
#include <component/rect/image.hpp>
#include <component/rect/rect_transform.hpp>
#include <component/revision.hpp>
#include <component/transform.hpp>

namespace MoonGlare::Systems {

void RegisterAllComponents(ECS::ECSRegister &ecs_register) {
    using namespace Component;
    ecs_register.RegisterComponent<Transform>();
    ecs_register.RegisterComponent<Mesh>();
    ecs_register.RegisterComponent<LightSource>();
    ecs_register.RegisterComponent<Name>();
    ecs_register.RegisterComponent<Camera>();
    ecs_register.RegisterComponent<Revision>();
    ecs_register.RegisterComponent<Parent>();
    ecs_register.RegisterComponent<GlobalMatrix>();

    ecs_register.RegisterComponent<Rect::RectTransform>();
    ecs_register.RegisterComponent<Rect::Image>();
}

} // namespace MoonGlare::Systems
