#pragma once

namespace MoonGlare {

#ifdef DEBUG
static constexpr bool kDebugBuild = true;
#else
static constexpr bool kDebugBuild = false;
#endif

static constexpr bool kDebugDumpEnabled = true;
static constexpr bool kDisableCaches = true;

#ifdef WINDOWS
static constexpr bool kIsOsWindows = true;
static constexpr bool kIsOsLinux = false;
#endif

#ifdef LINUX
static constexpr bool kIsOsWindows = false;
static constexpr bool kIsOsLinux = true;
#endif

} // namespace MoonGlare
