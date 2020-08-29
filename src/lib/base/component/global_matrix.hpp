#pragma once

#include "component_common.hpp"
#include "math/vector.hpp"

namespace MoonGlare::Component {

alignas(16) struct GlobalMatrix : public ComponentBase<GlobalMatrix> {
    static constexpr ComponentId kComponentId = 3;
    static constexpr char *kComponentName = "GlobalMatrix";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = false;

    math::Transform transform;
};

static_assert((sizeof(GlobalMatrix) % 16) == 0);
static_assert((offsetof(GlobalMatrix, transform) % 16) == 0);

} // namespace MoonGlare::Component
