#pragma once

#include "component_common.hpp"

namespace MoonGlare::Component {

struct Parent : public ComponentBase<Parent> {
    static constexpr ComponentId kComponentId = 1;
    static constexpr char* kComponentName = "Parent";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = false;
};

} // namespace MoonGlare::Component
