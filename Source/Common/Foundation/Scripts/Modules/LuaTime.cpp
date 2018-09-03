#include "LuaTime.h"

#include <Foundation/Scripts/ErrorReporting.h>
#include <Foundation/Component/iSubsystem.h>
#include <Foundation/TimeUtils.h>

namespace MoonGlare::Scripts::Modules {

using Component::SubsystemUpdateData;
using TimePoint = Component::SubsystemUpdateData::TimePoint;

static const TimePoint startTimePoint = TimePoint::clock::now();

static int TimeIndex(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "Time::__index";

    SubsystemUpdateData *sud = reinterpret_cast<SubsystemUpdateData*>(lua_touserdata(lua, lua_upvalueindex(1)));

    auto what = luaL_checkstring(lua, -1);

    switch (Space::Utils::MakeHash32(what)) {
    case "delta"_Hash32:
        lua_pushnumber(lua, sud->timeDelta);
        return 1;
    case "global"_Hash32:
        lua_pushnumber(lua, sud->globalTime);
        return 1;
    case "scene"_Hash32:
        lua_pushnumber(lua, sud->localTime);
        return 1;
    //case "fps"_Hash32:
    //    lua_pushnumber(lua, static_cast<lua_Number>(w->GetEngine()->GetFrameRate()));
    //    return 1;
    default:
        LuaRunError(lua, "Invalid time type", "Provided '{}' is not valid time type", what);
        return 0;
    };
}

//-------------------------------------------------------------------------------------------------

LuaTimeModule::LuaTimeModule(lua_State *lua, InterfaceMap *world) {
    auto *sud = world->GetInterface<SubsystemUpdateData>();
    assert(sud);

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Time")
            .addCClosure("__index", &TimeIndex, (void*)sud)
        .endNamespace()
        ;
}

LuaTimeModule::~LuaTimeModule() {
}

//-------------------------------------------------------------------------------------------------

} //namespace MoonGlare::Core::Scripts::Modules
