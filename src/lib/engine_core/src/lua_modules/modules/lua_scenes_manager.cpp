#include "lua_scenes_manager.hpp"
#include "lua_context/lua_context_build_config.hpp"
#include "scene_manager/configuration.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
#include <sol/sol.hpp>

namespace MoonGlare::LuaModules {

using namespace SceneManager;

/*@ [LuaModules/LuaScenesManagerModule] ScenesManage module
    This module allows to control scenes.
    `local scenes = require "moonglare.scenes"`
@*/

/*@ [LuaScenesManagerModule/ScenesManager] ScenesManager
@*/

/*@ [LuaScenesManagerModule/LuaScenesManagerModuleMethods] Methods
@*/

/*@ [LuaScenesManagerModule/LuaScenesManagerModuleProperties] Fields
@*/

SceneManagerLuaModule::SceneManagerLuaModule(SceneManager::iScenesManager *scenes_manager)
    : iDynamicScriptModule("SceneManagerLuaModule"), iRequireRequest("moonglare.scenes"),
      scenes_manager(scenes_manager) {
}

SceneManagerLuaModule::~SceneManagerLuaModule() {
}

void SceneManagerLuaModule::InitContext(lua_State *lua) {
    sol::state_view sol_view(lua);
    auto ns = sol_view[Lua::kInternalLuaNamespaceName].get_or_create<sol::table>();

    ns.new_usertype<SceneManager::iScenesManager>( //
        "iScenesManager", sol::no_constructor,

        /*@ [LuaScenesManagerModuleProperties/_] ScenesManager.LOADING_SCENE_NAME
            Use as name to select loading scene @*/
        "LOADING_SCENE_NAME", sol::var(kLoadingSceneName),

        /*@ [LuaScenesManagerModuleMethods/_] ScenesManager:create_scene(scene_resource, scene_name)
            TODO @*/
        "create_scene", &iScenesManager::CreateScene);
}

bool SceneManagerLuaModule::OnRequire(lua_State *lua, std::string_view name) {
    sol::stack::push(lua, scenes_manager);
    return true;
}

} // namespace MoonGlare::LuaModules
