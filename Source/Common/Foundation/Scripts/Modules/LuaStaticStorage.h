#pragma once

#include <Foundation/InterfaceMap.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <Foundation/Scripts/iDynamicModule.h>

namespace MoonGlare::Scripts::Modules {

class LuaStaticStorageModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaStaticStorageModule(lua_State *lua, InterfaceMap *world);
    ~LuaStaticStorageModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;
protected:
    lua_State *luaState;

    void SaveStorage();
};

} 
