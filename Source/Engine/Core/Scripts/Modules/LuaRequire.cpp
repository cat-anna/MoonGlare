#include <pch.h>
#include <nfMoonGlare.h>

#include "../ScriptEngine.h"

#include "LuaRequire.h"

#include <fmt/format.h>

namespace MoonGlare::Core::Scripts::Modules {

using ResultStoreMode = LuaRequireModule::ResultStoreMode;

LuaRequireModule::LuaRequireModule(lua_State *lua, World *world) : world(world) {
    DebugLogf(Debug, "Initializing Require module");


    MoonGlareAssert(world);
    MoonGlareAssert(lua);

    lua_newtable(lua); //table for cache

    lua_pushlightuserdata(lua, this);
    lua_pushvalue(lua, -2);
    lua_settable(lua, LUA_REGISTRYINDEX);					

    lua_pushlightuserdata(lua, this);
    lua_insert(lua, -2);
    lua_pushcclosure(lua, &lua_Require, 2);
    lua_setglobal(lua, "require");
}

LuaRequireModule::~LuaRequireModule() {
}

void LuaRequireModule::RegisterRequire(const std::string &name, iRequireRequest *iface) {
    if (!iface) {
        AddLogf(Debug, "Unregistered lua require: %s", name.c_str());
        scriptRequireMap.erase(name);
        return;
    }
    AddLogf(Debug, "Registered lua require: %s", name.c_str());
    scriptRequireMap[name] = iface;
}

bool LuaRequireModule::Querry(lua_State *lua, std::string_view name) {
    lua_pushlightuserdata(lua, this);
    lua_gettable(lua, LUA_REGISTRYINDEX);           
    int tableidx = lua_gettop(lua);

    bool succ = ProcessRequire(lua, name, tableidx);
    if (succ) {
        lua_insert(lua, -2);
    }

    lua_pop(lua, 1);
    return succ;
}         

bool LuaRequireModule::ProcessRequire(lua_State *lua, std::string_view name, int cachetableloc) {

    lua_getfield(lua, cachetableloc, name.data());
    if (!lua_isnil(lua, -1)) {
        AddLog(Performance, fmt::format("Got require '{}' from cache", name.data()));
        return 1;
    }
    else {
        lua_pop(lua, 1);
    }

    static constexpr std::array<decltype(&LuaRequireModule::HandleScriptSearch), 3> funcs = {
        &LuaRequireModule::HandleFileRequest,
        &LuaRequireModule::HandleModuleRequest,
        &LuaRequireModule::HandleScriptSearch,
    };

    for (auto item : funcs) {
        auto mode = (this->*item)(lua, name);

        switch (mode) {
        case ResultStoreMode::NoResult:
            continue;
        case ResultStoreMode::Store: {
            lua_pushvalue(lua, -1);
            lua_setfield(lua, cachetableloc, name.data());
        }
        //[[fallthrough]]
        case ResultStoreMode::DontStore:
            return true;

        default:
            LogInvalidEnum(mode);
            break;
        }
    }

    AddLog(Warning, fmt::format("Cannot find require '{}'", name.data()));
    return false;
}

ResultStoreMode LuaRequireModule::TryLoadFileScript(lua_State *lua, const std::string &uri) {
    StarVFS::ByteTable bt;
    if (!GetFileSystem()->OpenFile(bt, uri)) {
        AddLogf(Warning, "Cannot open file %s", uri.c_str());
        return ResultStoreMode::NoResult;
    }

    if (!world->GetScriptEngine()->ExecuteCode(bt.c_str(), bt.byte_size(), uri.c_str(), 1)) {
        AddLogf(Error, "Script execution failed: %s", uri.c_str());
        return ResultStoreMode::NoResult;
    }

    return ResultStoreMode::Store;
}

ResultStoreMode LuaRequireModule::HandleFileRequest(lua_State *lua, std::string_view name) {
    if (name[0] != '/') {
        return ResultStoreMode::NoResult;
    }
    //TODO: loading compiled scripts
    std::string uri = "file://" + std::string(name) + ".lua";
    return TryLoadFileScript(lua, uri);
}

ResultStoreMode LuaRequireModule::HandleScriptSearch(lua_State *lua, std::string_view name) {
    //TODO: improve script search
    std::string uri = "file:///Scripts/" + std::string(name) + ".lua";
    return TryLoadFileScript(lua, uri);
}

ResultStoreMode LuaRequireModule::HandleModuleRequest(lua_State *lua, std::string_view name) {
    auto it = scriptRequireMap.find(name.data());
    if (it != scriptRequireMap.end()) {
        if(it->second->OnRequire(lua, name))
            return ResultStoreMode::DontStore;
    }
    return ResultStoreMode::NoResult;
}

int LuaRequireModule::lua_Require(lua_State *lua) {
    void *voidThis = lua_touserdata(lua, lua_upvalueindex(1));
    LuaRequireModule *This = reinterpret_cast<LuaRequireModule*>(voidThis);

    std::string_view name = luaL_checkstring(lua, -1);
    if (This->ProcessRequire(lua, name, lua_upvalueindex(2)))
        return 1;

    throw eLuaPanic(fmt::format("Cannot find require '{}'", lua_tostring(lua, -1)));
}

} //namespace MoonGlare::Core::Scripts::Modules
