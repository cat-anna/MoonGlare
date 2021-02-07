#pragma once

#include "ecs/ecs_register.hpp"
#include <component/camera.hpp>
#include <component/global_matrix.hpp>
#include <component/light_source.hpp>
#include <component/mesh.hpp>
#include <component/name.hpp>
#include <component/parent.hpp>
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
}

} // namespace MoonGlare::Systems
