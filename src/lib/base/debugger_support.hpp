#pragma once

#include "build_configuration.hpp"

namespace MoonGlare {

inline void TriggerBreakPoint() {
    if constexpr (kDebugBuild && kIsOsWindows) {
        __debugbreak();
    }
}

} // namespace MoonGlare