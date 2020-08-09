#pragma once

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Scripts/iDynamicModule.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <interface_map.h>

#include <Foundation/Component/nfComponent.h>

namespace MoonGlare::Scripts::Modules {

class LuaEventsModule : public iDynamicScriptModule, public iRequireRequest {
  public:
    LuaEventsModule(lua_State *lua, InterfaceMap *world);
    ~LuaEventsModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;
    static ApiInitializer RegisterScriptApi(ApiInitializer api);
    static ApiInitializer RegisterDebugScriptApi(ApiInitializer api);

  protected:
    Component::EventDispatcher *eventDispatcher;

    int EmitEvent(lua_State *lua);
};

} // namespace MoonGlare::Scripts::Modules
