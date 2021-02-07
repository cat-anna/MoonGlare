#pragma once

#include "scene_manager/lua_modules.hpp"
#include "scenes_manager_lua_module.hpp"

namespace MoonGlare::SceneManager {

void LoadAllLuaModules(Lua::iScriptModuleManager *script_module_manager, iScenesManager *scenes_manager) {
    script_module_manager->AddModule(std::make_shared<SceneManagerLuaModule>(scenes_manager));
}

} // namespace MoonGlare::SceneManager
