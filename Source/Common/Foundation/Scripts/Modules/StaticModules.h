#pragma once

namespace MoonGlare::Scripts::Modules {

/*@ [Scripts/StaticModules] Static modules
    This modules are accessible by global variable
@*/

void InitLuaMath(lua_State *lua);
void InitLuaRandom(lua_State *lua);

using LuaStaticModuleInitFunc = void(*)(lua_State *lua);
static constexpr std::pair<LuaStaticModuleInitFunc, const char*> LuaStaticModulesTable[] = {
    { &InitLuaMath, "luaMathExt" },
    { &InitLuaRandom, "luaRandom" },
};

inline void InstallStaticModules(lua_State *lua) {
    for (auto &elem : LuaStaticModulesTable) {
        elem.first(lua);
    }
}

}
