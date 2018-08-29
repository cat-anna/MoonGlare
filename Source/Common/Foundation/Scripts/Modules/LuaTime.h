#pragma once

#include <Foundation/InterfaceMap.h>
#include <Foundation/Scripts/iDynamicModule.h>

namespace MoonGlare::Scripts::Modules {

class LuaTimeModule : public iDynamicScriptModule {
public:
    LuaTimeModule(lua_State *lua, InterfaceMap *world);
    ~LuaTimeModule() override;
protected:

};

}
