#pragma once

#include <boost/noncopyable.hpp>
#include <lua.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace MoonGlare::Lua {

class iDynamicScriptModule : private boost::noncopyable {
public:
    virtual ~iDynamicScriptModule() = default;
    virtual void InitContext(lua_State *lua) = 0;
};

class iRequireRequest {
public:
    virtual ~iRequireRequest() = default;
    virtual bool OnRequire(lua_State *lua, std::string_view name) = 0;
};

class iRequireModule {
public:
    virtual ~iRequireModule() = default;
    virtual void RegisterRequire(std::string name, iRequireRequest *iface) = 0;
    virtual bool Query(lua_State *lua, std::string_view name) = 0;
};

class iScriptModuleManager {
public:
    virtual ~iScriptModuleManager() = default;
    virtual void AddModule(std::shared_ptr<iDynamicScriptModule> module) = 0;
};

} // namespace MoonGlare::Lua
