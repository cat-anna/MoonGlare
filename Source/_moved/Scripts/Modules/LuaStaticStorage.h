#pragma once

#include <Foundation/Scripts/iDynamicModule.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <interface_map.h>

namespace MoonGlare::Scripts::Modules {

class LuaStaticStorageModule : public iDynamicScriptModule,
                               public iRequireRequest {
  public:
    LuaStaticStorageModule(lua_State *lua, InterfaceMap *world);
    ~LuaStaticStorageModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;

  protected:
    lua_State *luaState;

    void SaveStorage();
};

} // namespace MoonGlare::Scripts::Modules
