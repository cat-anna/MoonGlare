#include "runner_lua_modules.hpp"
#include "lua_application_module.hpp"

namespace MoonGlare::LuaModules {

void LoadAllRunnerLuaModules(Lua::iScriptModuleManager *script_module_manager, iEngineRunner *engine_runner) {
    script_module_manager->AddModule(std::make_shared<LuaApplicationModule>(engine_runner));
}

} // namespace MoonGlare::LuaModules
