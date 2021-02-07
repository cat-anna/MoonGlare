#pragma once

#include "scene_manager/scenes_manager_interface.hpp"
#include <lua_context/script_module.hpp>

namespace MoonGlare::SceneManager {

void LoadAllLuaModules(Lua::iScriptModuleManager *script_module_manager,
                       iScenesManager *scenes_manager);

} // namespace MoonGlare::SceneManager
