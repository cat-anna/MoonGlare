#pragma once

#include <engine_runner/engine_runner_interface.hpp>
#include <lua_context/script_module.hpp>


namespace MoonGlare {

void LoadAllRunnerLuaModules(Lua::iScriptModuleManager *script_module_manager, iEngineRunner *engine_runner);

} // namespace MoonGlare
