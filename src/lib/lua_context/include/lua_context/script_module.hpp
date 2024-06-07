#pragma once

#include "script_runner_interface.hpp"
#include <boost/noncopyable.hpp>
#include <lua.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace MoonGlare::Lua {

class iDynamicScriptModule : private boost::noncopyable {
public:
    iDynamicScriptModule(std::string_view name) : module_name(std::move(name)){};
    virtual ~iDynamicScriptModule() = default;
    virtual void InitContext(lua_State *lua) = 0;
    virtual std::string_view GetModuleName() const { return module_name; };

private:
    std::string_view module_name;
};

class iRequireRequest {
public:
    iRequireRequest(std::string_view name) : require_name(std::move(name)){};
    virtual ~iRequireRequest() = default;
    virtual bool OnRequire(lua_State *lua, std::string_view name) = 0;
    virtual std::string_view GetRequireName() const { return require_name; };

private:
    std::string_view require_name;
};

class iRequireModule {
public:
    virtual ~iRequireModule() = default;
    virtual void RegisterRequire(std::shared_ptr<iRequireRequest> iface, std::string name = "") = 0;
    virtual bool Query(lua_State *lua, std::string_view name) = 0;
};

class iScriptModuleManager {
public:
    virtual ~iScriptModuleManager() = default;
    virtual void AddModule(std::shared_ptr<iDynamicScriptModule> module) = 0;
};

class iScriptContext {
public:
    virtual ~iScriptContext() = default;

    virtual void Step(double dt) = 0;

    virtual void GcStep() = 0;
    virtual void SetGcStepSize(int size) = 0;
    virtual int GetGcStepSize() const = 0;
    virtual void CollectGarbage() = 0;

    virtual float GetMemoryUsage() const = 0;

    virtual iCodeChunkRunner *GetCodeRunnerInterface() = 0;
    virtual iScriptModuleManager *GetModuleManager() = 0;
};

} // namespace MoonGlare::Lua
