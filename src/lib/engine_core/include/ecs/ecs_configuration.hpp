#pragma once

#include <cstdint>

namespace MoonGlare::ECS {

constexpr size_t kComponentPageSize = 8 * 1024;
constexpr size_t kEntityLimit = 8 * 1024;

using EntityIndexType = uint32_t;

}; // namespace MoonGlare::ECS
