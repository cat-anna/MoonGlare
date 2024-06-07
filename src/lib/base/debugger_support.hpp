#pragma once

#include "build_configuration.hpp"

namespace MoonGlare {

#define TriggerBreakPoint()                                                                        \
    {                                                                                              \
        if constexpr (::MoonGlare::kDebugBuild && ::MoonGlare::kIsOsWindows) {                     \
            __debugbreak();                                                                        \
        }                                                                                          \
    }

} // namespace MoonGlare