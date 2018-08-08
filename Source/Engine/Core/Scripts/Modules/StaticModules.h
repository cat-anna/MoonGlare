#pragma once

namespace MoonGlare::Core::Scripts::Modules {

struct StaticModules {
    static void InitPrint(lua_State *lua, World *world);
    static void InitApplication(lua_State *lua, World *world);
    static void InitTime(lua_State *lua, World *world);
    static void InitThread(lua_State *lua, World *world);
    static void InitStrings(lua_State *lua, World *world);

    static void ThreadStep(lua_State *lua, World *world);
};

}              

