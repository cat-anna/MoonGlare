#include <pch.h>
#include <nfMoonGlare.h>

#include "../ScriptEngine.h"
#include "StaticModules.h"

#include <Base/Resources/StringTables.h>

namespace MoonGlare::Core::Scripts::Modules {

static int GetString(lua_State* lua) {
    World *w = reinterpret_cast<World*>(lua_touserdata(lua, lua_upvalueindex(1)));

    std::string_view str = luaL_checkstring(lua, -2);
    std::string_view table = luaL_checkstring(lua, -1);

    auto ret = w->GetStringTables()->GetString(str, table);
    lua_pushlstring(lua, ret.data(), ret.size());

    return 1;
}

void StaticModules::InitStrings(lua_State *lua, World *world) {
    DebugLogf(Debug, "Initializing Strings module");
    
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("string")
            .addCClosure("Get", &GetString, (void*)world)

        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
