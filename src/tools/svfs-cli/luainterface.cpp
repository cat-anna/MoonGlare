#include "luainterface.h"
#include <iostream>
#include <memory>
#include <orbit_logger.h>
#include <sstream>
#include <string_view>

namespace MoonGlare::Tools::VfsCli {

static int LuaPrinToStream(lua_State *L, std::ostringstream &out) {
    for (int i = 1, n = lua_gettop(L); i <= n; i++) {
        switch (lua_type(L, i)) {
        case LUA_TNUMBER:
            out << lua_tonumber(L, i);
            break;
        case LUA_TSTRING: {
            const char *s = lua_tostring(L, i);
            out << (s ? s : "{NULL}");
            break;
        }
        case LUA_TBOOLEAN:
            out << ((lua_toboolean(L, i) != 0) ? "true" : "false");
            break;
        case LUA_TUSERDATA:
            out << "[USERDATA]";
            break;
        case LUA_TNIL:
            out << "[NULL]";
            break;
        case LUA_TFUNCTION:
            out << "[Function@" << lua_tocfunction(L, i) << "]";
            break;
        case LUA_TTABLE:
            out << "[TABLE]";
            break;
        default:
            out << "[?=" << lua_type(L, i) << "]";
            break;
        }
        out << " ";
    }
    return 0;
}

static int lua_Print(lua_State *lua) {
    std::ostringstream out;
    LuaPrinToStream(lua, out);
    std::cout << out.str() << std::endl;
    return 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

Lua::Lua(const InitEnv &env) {
    sol_lua.open_libraries();
    sol_lua["print"] = lua_Print;
    auto config = sol_lua["config"].get_or_create<sol::table>();
    config["verbose"] = env.verbose;
}

//-------------------------------------------------------------------------------------------------

void Lua::RegisterAPI() {
}

//-------------------------------------------------------------------------------------------------

bool Lua::LoadLibrary(const char *c) {
    char buf[256];
    sprintf_s(buf, "require(\"%s\")", c);
    return ExecuteScriptChunk(buf);
}

bool Lua::ExecuteScriptFile(const char *fname) {
    sol_lua.script_file(fname);
    return true;
}

bool Lua::ExecuteScriptChunk(const char *code, const char *name) {
    sol_lua.script(std::string_view(code), name);
    return true;
}

bool Lua::ExecuteChunk(const unsigned char *data, size_t len, const char *name) {
    sol_lua.script(std::string_view(reinterpret_cast<const char *>(data), len), name);
    return true;
}

//-------------------------------------------------------------------------------------------------

bool Lua::Initialize(SharedModuleManager module_manager) {
    sol_lua["module_manager"] = module_manager;
    return true;
}

} // namespace MoonGlare::Tools::VfsCli
