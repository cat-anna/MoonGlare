#include "LuaEvents.h"

#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/ErrorReporting.h>

#include <Foundation/Scripts/LuaWrap.h>
#include <Foundation/Component/EventDispatcher.h>
#include <Foundation/Component/EventInfo.h>

namespace MoonGlare::Scripts::Modules {

LuaEventsModule::LuaEventsModule(lua_State *lua, InterfaceMap *world) {
    world->GetInterface<iRequireModule>()->RegisterRequire("Event", this);

    eventDispatcher = world->GetInterface<Component::EventDispatcher>();
    assert(eventDispatcher);

    InitSelfTable(lua);
}

LuaEventsModule::~LuaEventsModule() {
}

//-------------------------------------------------------------------------------------------------

bool LuaEventsModule::OnRequire(lua_State *lua, std::string_view name) {
    lua_getglobal(lua, "Event");
    return true;
}

//-------------------------------------------------------------------------------------------------

void LuaEventsModule::InitSelfTable(lua_State *lua) {
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Event")
            .addCClosure("Emit", &LuaEventsModule::EmitEvent, (void*)this)
        .endNamespace();
}

//-------------------------------------------------------------------------------------------------

int LuaEventsModule::EmitEvent(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "Event::Emit";

    if (lua_gettop(lua) != 1) {
        LuaRunError(lua, "Invalid argument count", "Expected 1 argument, got {}", lua_gettop(lua) - 1);
        return 0;
    }

    //int type0 = lua_type(lua, -1);
    //const char *name0 = lua_typename(lua, type0);

    if (!lua_isuserdata(lua, -1)) {
        LuaRunError(lua, "Invalid argument #1", "Invalid argument type: {}", lua_typename(lua, lua_type(lua, 2)));
        return 0;
    }

    lua_getfield(lua, -1, "EventId");
    int type = lua_type(lua, -1);
    const char *name = lua_typename(lua, type);
    auto eid = lua_tointeger(lua, -1);
    lua_pop(lua, 1);

    lua_getfield(lua, -1, "EventName");
    //const char *evName = lua_tostring(lua, -1);

    auto &evInfo = Component::BaseEventInfo::GetEventTypeInfo(static_cast<Component::EventClassId>(eid));


    void *thisPtr = lua_touserdata(lua, lua_upvalueindex(1));
    assert(thisPtr);
    LuaEventsModule *This = reinterpret_cast<LuaEventsModule*>(thisPtr);

    auto success = evInfo.infoPtr->QueueFromLua(lua, 1, This->eventDispatcher);
    if (!success) {
        LuaRunWarning(lua, "Event emission failed", "");
    }

    return 0;
}            

} //namespace MoonGlare::Core::Scripts::Modules
