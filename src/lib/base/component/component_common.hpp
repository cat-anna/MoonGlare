
#pragma once

#include <cstdint>
#include <fmt/format.h>

namespace MoonGlare::Component {

using ComponentId = uint8_t;
using ValidComponentsMap = uint64_t;
using ComponentRevision = uint64_t;
constexpr size_t kMaxComponents = 30;

constexpr size_t kComponentActiveFlagsOffset = kMaxComponents;
constexpr size_t kComponentFlagsOffset = kComponentActiveFlagsOffset + kMaxComponents;

//upper half of ValidComponentsMap
enum class ComponentFlags : ValidComponentsMap {
    kValid = 1llu << kComponentFlagsOffset,
};

template <typename T>
struct ComponentBase {
    //
};

} // namespace MoonGlare::Component
