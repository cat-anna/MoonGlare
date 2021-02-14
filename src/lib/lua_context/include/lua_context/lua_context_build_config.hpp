#pragma once

#include <build_configuration.hpp>

namespace MoonGlare::Lua {

static constexpr bool kEnableDebugScriptApi = kDebugBuild;

static constexpr auto kInternalLuaNamespaceName = "_classes";

} // namespace MoonGlare::Lua
