#pragma once

#include "component_common.hpp"
#include "component_serialiazation.hpp"

namespace MoonGlare::Component {

struct Parent : public ComponentBase<Parent> {
    static constexpr ComponentId kComponentId = 1;
    static constexpr char kComponentName[] = "parent";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = false;
};

} // namespace MoonGlare::Component
