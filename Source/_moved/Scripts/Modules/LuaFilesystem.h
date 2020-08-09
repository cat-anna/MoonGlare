#pragma once

#include <Foundation/iFileSystem.h>

#include "../iDynamicModule.h"
#include "../iLuaRequire.h"

namespace MoonGlare::Scripts::Modules {

class LuaFileSystemModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaFileSystemModule(lua_State *lua, InterfaceMap *world);
    ~LuaFileSystemModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;

    static ApiInitializer RegisterScriptApi(ApiInitializer api);
protected:
    iFileSystem *fs;

    int ReadFileContent(lua_State *lua);
    int ReadJSON(lua_State *lua);
    int EnumerateFolder(lua_State *lua);
    int FindFilesByExt(lua_State *lua);
};

} //namespace MoonGlare::Scripts::Modules
