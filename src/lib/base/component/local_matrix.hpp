#pragma once

#include "component_common.hpp"
#include "math/vector.hpp"

namespace MoonGlare::Component {

struct alignas(16) LocalMatrix : public ComponentBase<LocalMatrix> {
    static constexpr ComponentId kComponentId = 5;
    static constexpr char kComponentName[] = "local_matrix";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = false;
    static constexpr bool kHasResources = false;

    math::Transform transform;

    static LocalMatrix Identity() { return LocalMatrix{.transform = math::Transform::Identity()}; }
};

static_assert((sizeof(LocalMatrix) % 16) == 0);
static_assert((offsetof(LocalMatrix, transform) % 16) == 0);

} // namespace MoonGlare::Component
