#pragma once

#include "global_matrix_system.hpp"
#include "component/global_matrix.hpp"
#include "component/local_matrix.hpp"
#include "ecs/component_array.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Systems {

using namespace MoonGlare::Component;

void GlobalMatrixSystem::DoStep(double time_delta) {
    GetComponentArray()->VisitWithParent<GlobalMatrix, LocalMatrix>(
        [this](const GlobalMatrix &parent, GlobalMatrix &child, LocalMatrix &child_matrix) {
            //TODO: optimize
            child.transform = parent.transform * child_matrix.transform;
        });
}

} // namespace MoonGlare::Systems
