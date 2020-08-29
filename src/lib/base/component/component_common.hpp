
#pragma once

#include "component_serialiazation.hpp"
#include <cstdint>

namespace MoonGlare::Component {

using ComponentId = uint8_t;
constexpr size_t MaxComponents = sizeof(uint64_t) * 8;

template <typename T>
struct ComponentBase {
    //
};

} // namespace MoonGlare::Component
