#pragma once

namespace MoonGlare::Core::Scripts {

class iScriptRequire {
public:
    virtual int OnRequire(lua_State *lua, const std::string_view& name) = 0;
};

} //namespace MoonGlare::Core::Script
