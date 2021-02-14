#pragma once

#include <chrono>
#include <engine_runner/engine_time.hpp>
#include <lua_context/script_module.hpp>

namespace MoonGlare::LuaModules {

class LuaTimeModule : public Lua::iDynamicScriptModule, public Lua::iRequireRequest {
public:
    LuaTimeModule(iEngineTime *_engine_time);
    ~LuaTimeModule() override;

    //iDynamicScriptModule
    void InitContext(lua_State *lua) override;

    //iRequireRequest
    bool OnRequire(lua_State *lua, std::string_view name) override;

private:
    iEngineTime *const engine_time;
};

} // namespace MoonGlare::LuaModules
