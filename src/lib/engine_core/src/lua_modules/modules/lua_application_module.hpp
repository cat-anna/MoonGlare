#pragma once

// #include <Foundation/Scripts/iDynamicModule.h>
#include "core/engine_runner_interface.hpp"
#include <chrono>
#include <lua_context/script_module.hpp>


namespace MoonGlare::LuaModules {

class LuaApplicationModule : public Lua::iDynamicScriptModule, public Lua::iRequireRequest {
public:
    LuaApplicationModule(iEngineRunner *_engine_runner);
    ~LuaApplicationModule() override;

    //iDynamicScriptModule
    void InitContext(lua_State *lua) override;

    //iRequireRequest
    bool OnRequire(lua_State *lua, std::string_view name) override;

private:
    iEngineRunner *const engine_runner;
};

} // namespace MoonGlare::LuaModules
