#include "scenes_manager_lua_module.hpp"
#include "lua_context/lua_context_build_config.hpp"
#include "scene_manager/configuration.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>
#include <sol/sol.hpp>


namespace MoonGlare::SceneManager {

/*@ [LuaModules/LuaScenesManagerModule] ScenesManage module
    This module allows to control current scene.
    `local scenes = require "moonglare.scenes"`
@*/

SceneManagerLuaModule::SceneManagerLuaModule(iScenesManager *scenes_manager)
    : iDynamicScriptModule("SceneManagerLuaModule"), iRequireRequest("moonglare.scenes"),
      scenes_manager(scenes_manager) {
}

SceneManagerLuaModule::~SceneManagerLuaModule() {
}

void SceneManagerLuaModule::InitContext(lua_State *lua) {
    sol::state_view sol_view(lua);
    auto ns = sol_view[Lua::kInternalLuaNamespaceName].get_or_create<sol::table>();

    ns.new_usertype<iScenesManager>("iScenesManager", sol::no_constructor,        //
                                    "loading_scene", sol::var(kLoadingSceneName), //
                                    "create_scene", &iScenesManager::CreateScene  //
    );
}

bool SceneManagerLuaModule::OnRequire(lua_State *lua, std::string_view name) {
    sol::stack::push(lua, scenes_manager);
    return true;
}

} // namespace MoonGlare::SceneManager
