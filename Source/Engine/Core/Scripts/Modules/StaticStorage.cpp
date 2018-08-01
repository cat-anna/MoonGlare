#include <pch.h>
#include <nfMoonGlare.h>

#include "../../Engine.h"
#include "../ScriptEngine.h"

#include "StaticStorage.h"

#include <Foundation/Scripts/ErrorHandling.h>

#include <Foundation/OS/Path.h>
#include <Foundation/OS/File.h>

namespace MoonGlare::Core::Scripts::Modules {
using namespace MoonGlare::Scripts;

static const char* StorageFileName = "StaticStorage.lua";

StaticStorageModule::StaticStorageModule(lua_State *lua, World *world) {
    scriptEngine = world->GetScriptEngine();
    scriptEngine->QuerryModule<iRequireModule>()->RegisterRequire("StaticStorage", this);
}

StaticStorageModule::~StaticStorageModule() {
    try {
        SaveStorage();
    }
    catch (...) {
        __debugbreak();
    }
}

//-------------------------------------------------------------------------------------------------

bool StaticStorageModule::OnRequire(lua_State *lua, std::string_view name) {
    lua_pushlightuserdata(lua, this);
    lua_gettable(lua, LUA_REGISTRYINDEX);

    //stack: storage/nil

    bool reset = false;
    if (lua_isnil(lua, -1)) {       
        reset = true;
        //stack: nil
        lua_pop(lua, 1);                   //stack: 

        std::string data;
        if (OS::GetFileContent(OS::GetSettingsFilePath(StorageFileName), data) ) {
            if (scriptEngine->ExecuteCode("return " + data, "StaticStorageData", 1)) {
                lua_pushlightuserdata(lua, this);
                lua_pushvalue(lua, -2);
                lua_settable(lua, LUA_REGISTRYINDEX);
                reset = false;
            }
            else {
                AddLogf(Error, "Failed to load static storage");
            }
        }
    }

    if (reset) {
        lua_createtable(lua, 0, 0);
        lua_pushlightuserdata(lua, this);
        lua_pushvalue(lua, -2);

        lua_settable(lua, LUA_REGISTRYINDEX);
    }
    //stack: storage

    return true;
}

void StaticStorageModule::SaveStorage() {
    lua_State *lua = scriptEngine->GetLua();
    int luatop = lua_gettop(lua);
    lua_pushcclosure(lua, LuaErrorHandler, 0);                    //stack: errH
    int errf = lua_gettop(lua);

    lua_pushlightuserdata(lua, this);
    lua_gettable(lua, LUA_REGISTRYINDEX);

    if (lua_isnil(lua, -1)) {
        lua_settop(lua, luatop);
        return;
    }

    //stack: errH storage

    lua_getglobal(lua, "table");               //stack: storage table
    lua_getfield(lua, -1, "serialize");        //stack: storage table serialize

    lua_pushvalue(lua, -3);                    //stack: storage table serialize storage

    if (!LuaSafeCall(lua, 1, 1, "table.serialize", errf)) {
        AddLogf(Error, "Failure during static storage serialization");
    }
    else {
        size_t len = 0;
        const char *text = lua_tolstring(lua, -1, &len);
        std::ofstream out(OS::GetSettingsFilePath(StorageFileName), std::ios::out | std::ios::binary);
        out.write(text, len);
        out.close();
    }

    lua_settop(lua, luatop);
}

} //namespace MoonGlare::Core::Scripts::Modules
