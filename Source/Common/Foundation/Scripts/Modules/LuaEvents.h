#pragma once

#include <Foundation/InterfaceMap.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <Foundation/Scripts/iDynamicModule.h>

#include <Foundation/Component/nfComponent.h>

namespace MoonGlare::Scripts::Modules {

class LuaEventsModule : public iDynamicScriptModule, public iRequireRequest {
public:
    LuaEventsModule(lua_State *lua, InterfaceMap *world);
    ~LuaEventsModule() override;

    bool OnRequire(lua_State *lua, std::string_view name) override;
protected:
    Component::EventDispatcher *eventDispatcher;

    void InitSelfTable(lua_State *lua);

    static int EmitEvent(lua_State *lua);
};

}
