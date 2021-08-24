#pragma once

#include "ecs/ecs_register.hpp"

namespace MoonGlare::Systems {

class ECSRegister;

void RegisterAllSystems(ECS::ECSRegister &ecs_register);

} // namespace MoonGlare::Systems
