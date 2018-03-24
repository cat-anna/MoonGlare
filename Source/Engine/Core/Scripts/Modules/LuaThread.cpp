#include <pch.h>
#include <nfMoonGlare.h>
#include <Core/Engine.h>
#include <Core/Scripts/ScriptEngine.h>
#include <Core/Scene/ScenesManager.h>

#include "StaticModules.h"

namespace MoonGlare::Core::Scripts::Modules {

#include <LuaThread.lua.h>

void StaticModules::ThreadStep(lua_State *lua, World *world) {
    lua_pushlightuserdata(lua, (void*)LuaThread_lua);
    lua_gettable(lua, LUA_REGISTRYINDEX);

    if (!world->GetScriptEngine()->Call(lua, 0, 2))
        return;

    bool change = static_cast<bool>(lua_toboolean(lua, -2));
    if (change) {
        int th = static_cast<int>(lua_tonumber(lua, -1));
        world->GetScenesManager()->SetSceneChangeFence(Scene::SceneChangeFence::ScriptThreads, th > 0);
    }
    lua_pop(lua, 2);
}

void StaticModules::InitThread(lua_State *lua, World *world) {
    if (!world->GetScriptEngine()->ExecuteCode((const char *)LuaThread_lua, LuaThread_lua_size, "LuaThread", 1)) {
        throw std::runtime_error("InitThread module execute code failed!");
    }
  //  auto t = lua_type(lua, -1);

    lua_pushlightuserdata(lua, (void*)LuaThread_lua);
    lua_insert(lua, -2);              
    lua_settable(lua, LUA_REGISTRYINDEX);
}

} //namespace MoonGlare::Core::Scripts::Modules
