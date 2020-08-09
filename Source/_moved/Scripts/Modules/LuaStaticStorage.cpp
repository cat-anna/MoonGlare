#include "LuaStaticStorage.h"

#include <Foundation/OS/Path.h>
#include <Foundation/OS/File.h>

#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/ExecuteCode.h>

namespace MoonGlare::Scripts::Modules {

static const char* StorageFileName = "StaticStorage.lua";

LuaStaticStorageModule::LuaStaticStorageModule(lua_State *lua, InterfaceMap *world)  : luaState(lua) {
    world->GetInterface<iRequireModule>()->RegisterRequire("StaticStorage", this);
}

LuaStaticStorageModule::~LuaStaticStorageModule() {
    try {
        SaveStorage();
    }
    catch (...) {
        __debugbreak();
    }
}

//-------------------------------------------------------------------------------------------------

/*@ [RequireModules/StaticStorageModule] StaticStorage module
    This module provides storage which will be preserved between engine executions.
    There is no specific api provided by this module. Returned lua table is freely available. 
    NOTE: Only primitive values can be preserved (boolean, number, string).
@*/

bool LuaStaticStorageModule::OnRequire(lua_State *lua, std::string_view name) {
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
            if (ExecuteString(lua, "return " + data, "StaticStorageData", 1)) {
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

void LuaStaticStorageModule::SaveStorage() {
    lua_State *lua = luaState;

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
