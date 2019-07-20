#pragma once

#include "../iDynamicModule.h"
#include "../iLuaRequire.h"

namespace MoonGlare::Scripts::Modules {

class LuaDebugContextModule : public iDynamicScriptModule {
public:
    LuaDebugContextModule(lua_State *lua, InterfaceMap *world);
    ~LuaDebugContextModule() override;

    static ApiInitializer RegisterScriptApi(ApiInitializer api);
    static ApiInitializer RegisterDebugScriptApi(ApiInitializer api);
protected:
    InterfaceMap &interfaceMap;

    static int IndexMethod(lua_State *lua);
};

} //namespace MoonGlare::Scripts::Modules
