#pragma once

#include <lua_context/script_module.hpp>

namespace MoonGlare::Lua {

class LuaRandomModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaRandomModule();

    bool OnRequire(lua_State *lua, std::string_view name) override;
    void InitContext(lua_State *lua) override;

protected:
};

} // namespace MoonGlare::Lua
