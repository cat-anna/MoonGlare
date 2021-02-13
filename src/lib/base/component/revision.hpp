#pragma once

#include "component_common.hpp"
#include "math/vector.hpp"
#include "static_string.hpp"
#include <cstddef>

namespace MoonGlare::Component {

#pragma pack(push, 1)

// alignas(16)
struct Revision : public ComponentBase<Revision> {
    static constexpr ComponentId kComponentId = 2;
    static constexpr char kComponentName[] = "revision";
    static constexpr bool kEditable = false;
    static constexpr bool kSerializable = true;

    uint64_t value;
};

#pragma pack(pop)

} // namespace MoonGlare::Component
