#pragma once

namespace MoonGlare {
#ifdef DEBUG
static constexpr bool kDebugBuild = true;
#else
#error asd
static constexpr bool kDebugBuild = false;
#endif
} // namespace MoonGlare
