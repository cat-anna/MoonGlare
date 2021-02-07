#pragma once

#include "ecs/ecs_register.hpp"

namespace MoonGlare::Systems {

class ECS::ECSRegister;

void RegisterAllComponents(ECS::ECSRegister &ecs_register);

} // namespace MoonGlare::Systems
