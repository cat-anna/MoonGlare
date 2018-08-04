#pragma once

#include <Foundation/InterfaceMap.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <Foundation/Scripts/iDynamicModule.h>
#include <Foundation/iFileSystem.h>

namespace MoonGlare::Scripts::Modules {

class LuaRequireModule : public iDynamicScriptModule, public iRequireModule {
public:
    LuaRequireModule(lua_State *lua, InterfaceMap *world);
    ~LuaRequireModule() override;

    void RegisterRequire(const std::string &name, iRequireRequest *iface) override;
    bool Querry(lua_State *lua, std::string_view name) override;
protected:
    std::unordered_map<std::string, iRequireRequest*> scriptRequireMap;
    iFileSystem *filesystem = nullptr;

    //require name shall be on top of the stack
    bool ProcessRequire(lua_State *lua, std::string_view name, int cachetableloc);

    enum class ResultStoreMode {
        NoResult,
        DontStore,
        Store
    };

    ResultStoreMode TryLoadFileScript(lua_State *lua, const std::string &uri);

    ResultStoreMode HandleFileRequest(lua_State *lua, std::string_view name);
    ResultStoreMode HandleModuleRequest(lua_State *lua, std::string_view name);
    ResultStoreMode HandleScriptSearch(lua_State *lua, std::string_view name);

    static int lua_require(lua_State *lua);
    static int lua_dofile(lua_State *lua);
};

} 