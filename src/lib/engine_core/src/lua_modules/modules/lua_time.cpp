#include "lua_time.hpp"
#include "lua_context/lua_context_build_config.hpp"

namespace MoonGlare::LuaModules {

/*@ [LuaModules/LuaTimeModule] Time module
    This module allows to control time flow.
    `local time = require "moonglare.time"`
@*/

LuaTimeModule::LuaTimeModule(iEngineTime *_engine_time)
    : iDynamicScriptModule("LuaTimeModule"), iRequireRequest("moonglare.time"),
      engine_time(_engine_time) {
}

LuaTimeModule::~LuaTimeModule() {
}

void LuaTimeModule::InitContext(lua_State *lua) {
    // sol::state_view sol_view(lua);
    // auto ns = sol_view[Lua::kInternalLuaNamespaceName].get_or_create<sol::table>();

    // ns.new_usertype<iScenesManager>("iScenesManager", sol::no_constructor,        //
    //                                 "loading_scene", sol::var(kLoadingSceneName), //
    //                                 "create_scene", &iScenesManager::CreateScene  //
    // );
}

bool LuaTimeModule::OnRequire(lua_State *lua, std::string_view name) {
    // sol::stack::push(lua, scenes_manager);
    return false;
}

#if 0
using Component::SubsystemUpdateData;
using TimePoint = Component::SubsystemUpdateData::TimePoint;

static const TimePoint startTimePoint = TimePoint::clock::now();

static int TimeIndex(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "Time::__index";

    SubsystemUpdateData *sud =
        reinterpret_cast<SubsystemUpdateData *>(lua_touserdata(lua, lua_upvalueindex(1)));

    auto what = luaL_checkstring(lua, -1);

    switch (Space::Utils::MakeHash32(what)) {

        /* @ [TimeModule/_] `Time.delta`
    Current time delta
@*/
    case "delta"_Hash32:
        lua_pushnumber(lua, sud->timeDelta);
        return 1;
        /* @ [TimeModule/_] `Time.global`
    Global execution time. This value is value is increased each engine loop
@*/
    case "global"_Hash32:
        lua_pushnumber(lua, sud->globalTime);
        return 1;
        /* @ [TimeModule/_] `Time.scene`
    Current scene execution time. TODO: See Scene.localTime
@*/
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

#endif
} // namespace MoonGlare::LuaModules