#include <pch.h>
#include <nfMoonGlare.h>
#include <Core/Engine.h>
#include <Core/Scene/ScenesManager.h>

#include "StaticModules.h"

namespace MoonGlare::Core::Scripts::Modules {

using std::chrono::steady_clock;
using std::chrono::duration;

static const auto startTimePoint = steady_clock::now();
static int TimeIndex(lua_State *lua) {
    World *w = reinterpret_cast<World*>(lua_touserdata(lua, lua_upvalueindex(1)));

    auto what = luaL_checkstring(lua, -1);

    switch (Space::Utils::MakeHash32(what)) {
    case "delta"_Hash32:
        lua_pushnumber(lua, w->GetEngine()->StepData().timeDelta);
        return 1;
    case "global"_Hash32:
        lua_pushnumber(lua, w->GetEngine()->StepData().globalTime);
        return 1;
    case "scene"_Hash32:
        lua_pushnumber(lua, w->GetScenesManager()->GetSceneTime());
        return 1;
    case "fps"_Hash32:
        lua_pushnumber(lua, static_cast<lua_Number>(w->GetEngine()->GetFrameRate()));
        return 1;
    case "perf"_Hash32:
        lua_pushnumber(lua, duration<double>(steady_clock::now() - startTimePoint).count());
        return 1;
    default:
        AddLogf(ScriptRuntime, "Attempt to get unknown field %s", what);
        return 0;
    };
}

void StaticModules::InitTime(lua_State *lua, World *world) {
    void *v = world;
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Time")
            .addCClosure("__index", &TimeIndex, v)
        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
