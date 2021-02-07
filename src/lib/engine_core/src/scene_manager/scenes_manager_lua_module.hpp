
#pragma once

#include "scene_manager/scenes_manager.hpp"
#include <lua_context/script_module.hpp>

namespace MoonGlare::SceneManager {

class SceneManagerLuaModule : public Lua::iDynamicScriptModule, public Lua::iRequireRequest {
public:
    SceneManagerLuaModule(iScenesManager *scenes_manager);
    ~SceneManagerLuaModule() override;

    //iDynamicScriptModule
    void InitContext(lua_State *lua) override;

    //iRequireRequest
    bool OnRequire(lua_State *lua, std::string_view name) override;

private:
    iScenesManager *const scenes_manager;
};

} // namespace MoonGlare::SceneManager
