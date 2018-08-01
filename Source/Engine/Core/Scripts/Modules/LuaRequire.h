#pragma once


#include <EngineBase/Scripts/iLuaRequire.h>
#include <EngineBase/Scripts/iDynamicModule.h>

namespace MoonGlare::Core::Scripts::Modules {
using namespace MoonGlare::Scripts;

class LuaRequireModule : public iDynamicScriptModule, public iRequireModule {
public:
    LuaRequireModule(lua_State *lua, World *world);
    ~LuaRequireModule() override;

    enum class ResultStoreMode {
        NoResult,
        DontStore,
        Store
    };

    void RegisterRequire(const std::string &name, iRequireRequest *iface) override;
    bool Querry(lua_State *lua, std::string_view name) override;
protected:
    std::unordered_map<std::string, iRequireRequest*> scriptRequireMap;
    World *world = nullptr;

    //require name shall be on top of the stack
    bool ProcessRequire(lua_State *lua, std::string_view name, int cachetableloc);

    ResultStoreMode TryLoadFileScript(lua_State *lua, const std::string &uri);

    ResultStoreMode HandleFileRequest(lua_State *lua, std::string_view name);
    ResultStoreMode HandleModuleRequest(lua_State *lua, std::string_view name);
    ResultStoreMode HandleScriptSearch(lua_State *lua, std::string_view name);

    static int lua_require(lua_State *lua);
    static int lua_dofile(lua_State *lua);
};

} //namespace MoonGlare::Core::Script::Modules
