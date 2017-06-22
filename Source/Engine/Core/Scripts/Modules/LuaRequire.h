#pragma once

#include "../iDynamicModule.h"
#include "../iLuaRequire.h"

namespace MoonGlare::Core::Scripts::Modules {

class LuaRequireModule : public iDynamicScriptModule, public iRequireModule {
public:
    LuaRequireModule(lua_State *lua, World *world);
    ~LuaRequireModule() override;

    void RegisterRequire(const std::string &name, iRequireRequest *iface) override;
protected:
    std::unordered_map<std::string, iRequireRequest*> scriptRequireMap;
    World *world = nullptr;

    static int lua_RequireQuerry(lua_State *lua);
};

} //namespace MoonGlare::Core::Script::Modules
