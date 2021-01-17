#pragma once

#include <Foundation/Scripts/iDynamicModule.h>
#include <interface_map.h>

namespace MoonGlare::Scripts::Modules {

class LuaTimeModule : public iDynamicScriptModule {
  public:
    LuaTimeModule(lua_State *lua, InterfaceMap *world);
    ~LuaTimeModule() override;

  protected:
};

} // namespace MoonGlare::Scripts::Modules
