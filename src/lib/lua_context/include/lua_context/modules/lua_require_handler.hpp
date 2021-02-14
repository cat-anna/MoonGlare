#pragma once

#include <lua_context/script_module.hpp>
#include <lua_context/script_runner_interface.hpp>
#include <readonly_file_system.h>
#include <unordered_map>

namespace MoonGlare::Lua {

/*@ [Scripts/RequireModules] Dynamic modules
    This modules are accessible by
```lua
local module = require "module"
```
@*/

class LuaRequireModule : public iDynamicScriptModule, public iRequireModule {
public:
    LuaRequireModule(iReadOnlyFileSystem *_filesystem);
    ~LuaRequireModule() override;

    void RegisterRequire(std::shared_ptr<iRequireRequest> iface, std::string name) override;
    bool Query(lua_State *lua, std::string_view name) override;
    void InitContext(lua_State *lua) override;

protected:
    iReadOnlyFileSystem *const filesystem;

    std::unordered_map<std::string, std::shared_ptr<iRequireRequest>> scriptRequireMap;

    // require name shall be on top of the stack
    bool ProcessRequire(lua_State *lua, std::string_view name, int cachetableloc);

    enum class ResultStoreMode { NoResult, DontStore, Store };

    ResultStoreMode TryLoadFileScript(lua_State *lua, const std::string &file_path);

    ResultStoreMode HandleFileRequest(lua_State *lua, std::string_view name);
    ResultStoreMode HandleModuleRequest(lua_State *lua, std::string_view name);

    static int lua_require(lua_State *lua);
    static int lua_dofile(lua_State *lua);
};
} // namespace MoonGlare::Lua
