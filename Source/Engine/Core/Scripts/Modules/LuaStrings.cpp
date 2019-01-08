#include <pch.h>
#include <nfMoonGlare.h>

#include "../ScriptEngine.h"
#include "StaticModules.h"

#include <Foundation/Resources/StringTables.h>

namespace MoonGlare::Core::Scripts::Modules {

/*@ [StaticModules/LuaStringModule] String module
    This is an extension to lua `string` module.
@*/

/*@ [LuaStringModule/_] `string.get(tableName, entryName)`
    Get string from current language string tables.  
@*/
static int GetString(lua_State* lua) {
    auto *st = reinterpret_cast<Resources::StringTables*>(lua_touserdata(lua, lua_upvalueindex(1)));
    assert(st);

    std::string_view str = luaL_checkstring(lua, -2);
    std::string_view table = luaL_checkstring(lua, -1);

    auto ret = st->GetString(str, table);
    lua_pushlstring(lua, ret.data(), ret.size());

    return 1;
}

void StaticModules::InitStrings(lua_State *lua, World *world) {
    DebugLogf(Debug, "Initializing Strings module");

    Resources::StringTables *st = nullptr;
    world->GetObject(st);
    
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("string")
            .addCClosure("get", &GetString, (void*)st)     
            .addCClosure("Get", &GetString, (void*)st)     //TODO: remove compatibility string.Get method
        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
