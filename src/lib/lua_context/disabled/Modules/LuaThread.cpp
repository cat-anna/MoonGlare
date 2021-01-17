#include <pch.h>
#include <nfMoonGlare.h>
#include <Foundation/Scripts/ExecuteCode.h>
#include <Core/Scene/ScenesManager.h>

#include "StaticModules.h"

namespace MoonGlare::Core::Scripts::Modules {

#include <LuaThread.lua.h>

/*@ [StaticModules/LuaThreadModule] Thread module
    This is an extension to lua `coroutine` module. Basic concept remains the same.  
    Pending coroutines are executed each step. Their execution cannot be paused from engine,
    so they must explicitly call `coroutine.yield()` often enough to keep framerate stable.
@*/
void StaticModules::ThreadStep(lua_State *lua, World *world) {
    lua_pushlightuserdata(lua, (void*)LuaThread_lua);
    lua_gettable(lua, LUA_REGISTRYINDEX);

    MoonGlare::Scripts::CallFunction(lua, 0, 0);
}

void StaticModules::InitThread(lua_State *lua, World *world) {      
    if (!MoonGlare::Scripts::ExecuteString(lua, (char*)LuaThread_lua, LuaThread_lua_size, "LuaThread", 1)) {
        throw std::runtime_error("InitThread module execute code failed!");
    }
  //  auto t = lua_type(lua, -1);

    lua_pushlightuserdata(lua, (void*)LuaThread_lua);
    lua_insert(lua, -2);              
    lua_settable(lua, LUA_REGISTRYINDEX);
}

} //namespace MoonGlare::Core::Scripts::Modules
