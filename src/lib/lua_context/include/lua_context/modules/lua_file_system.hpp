#pragma once

#include <lua_context/script_module.hpp>
#include <readonly_file_system.h>
#include <string_view>
#include <tuple>

namespace MoonGlare::Lua {

class LuaFileSystemModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaFileSystemModule(std::shared_ptr<iReadOnlyFileSystem> filesystem);
    ~LuaFileSystemModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;
    void InitContext(lua_State *lua) override;

protected:
    std::shared_ptr<iReadOnlyFileSystem> filesystem;

    std::string ReadFileByPath(std::string_view file_name);
    std::tuple<bool, std::string> ReadFileByPathSafe(std::string_view file_name);
    // int ReadJSON(lua_State *lua);
    // int EnumerateFolder(lua_State *lua);
    // int FindFilesByExt(lua_State *lua);
};

} // namespace MoonGlare::Lua
