#pragma once

namespace MoonGlare::Core::Scripts {

class iRequireRequest {
public:
    virtual int OnRequire(lua_State *lua, const std::string_view& name) = 0;
};

class iRequireModule {
public:
    virtual void RegisterRequire(const std::string &name, iRequireRequest *iface) = 0;
};

} //namespace MoonGlare::Core::Script
