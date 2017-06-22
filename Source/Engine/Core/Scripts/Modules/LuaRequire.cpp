#include <pch.h>
#include <nfMoonGlare.h>

#include "../ScriptEngine.h"

#include "LuaRequire.h"

namespace MoonGlare::Core::Scripts::Modules {

static constexpr char InitRequireCode[] = R"===(

local RequireStorage = { }
local RequireQuerry = __RequireQuerry
__RequireQuerry = nil

function require(what)
    local stored = RequireStorage[what]
    if stored then
        return stored
    end

    local foundcode, value = RequireQuerry(what)
    
    if foundcode == 0 then
        error(value, 2)
    end

    if foundcode == 2 then
        RequireStorage[what] = value
    end

    return value
end

)===";

LuaRequireModule::LuaRequireModule(lua_State *lua, World *world) : world(world) {
    MoonGlareAssert(world);
    MoonGlareAssert(lua);

    lua_pushlightuserdata(lua, this);
    lua_pushcclosure(lua, &lua_RequireQuerry, 1);
    lua_setglobal(lua, "__RequireQuerry");

    if (!world->GetScriptEngine()->ExecuteCode(std::string(InitRequireCode), "LuaRequireModule")) {
        throw std::runtime_error("InitPrintModule execute code failed!");
    }

    lua_pushnil(lua);
    lua_setglobal(lua, "__RequireQuerry");
}

LuaRequireModule::~LuaRequireModule() {
}

void LuaRequireModule::RegisterRequire(const std::string &name, iRequireRequest *iface) {
    if (!iface) {
        AddLog(Debug, "Unregistered lua require: %s", name.c_str());
        scriptRequireMap.erase(name);
        return;
    }
    AddLog(Debug, "Registered lua require: %s", name.c_str());
    scriptRequireMap[name] = iface;
}

int LuaRequireModule::lua_RequireQuerry(lua_State *lua) {
    std::string_view name = luaL_checkstring(lua, -1);

    void *voidThis = lua_touserdata(lua, lua_upvalueindex(1));
    LuaRequireModule *This = reinterpret_cast<LuaRequireModule*>(voidThis);

    if (name[0] == '/') {
        //TODO: request to filesystem
        //lua_pushnumber(lua, 2);
        //return 0;
    }

    auto it = This->scriptRequireMap.find(name.data());
    if (it != This->scriptRequireMap.end()) {
        lua_pushnumber(lua, 1);
        return it->second->OnRequire(lua, name) + 1;
    }

    lua_pushnumber(lua, 0);
    lua_pushfstring(lua, "There is no registered require '%s'", name.data());
    return 2;
}

} //namespace MoonGlare::Core::Scripts::Modules
