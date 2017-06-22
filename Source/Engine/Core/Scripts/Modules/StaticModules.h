#pragma once

namespace MoonGlare::Core::Scripts::Modules {

struct StaticModules {
    static void InitPrint(lua_State *lua, World *world);
    static void InitMath(lua_State *lua, World *world);
    static void InitRandom(lua_State *lua, World *world);
};

}              
