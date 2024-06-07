#pragma once

#include <lua_context/script_module.hpp>

namespace MoonGlare::Lua {

class LuaContextModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaContextModule(iScriptContext *_lua_context);
    ~LuaContextModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;
    void InitContext(lua_State *lua) override;

protected:
    iScriptContext *const lua_context;
};

} // namespace MoonGlare::Lua
