#include "luainterface.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace MoonGlare::Tools::VfsCli {

static int Lua_print(lua_State *L, std::ostringstream &out) {
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

static int lua_DebugPrint(lua_State *lua) {
    std::ostringstream out;
    out << "[DEBUG][LUA] ";
    Lua_print(lua, out);
    std::cout << out.str() << "\n";
    return 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

Lua::Lua() { sol_lua.open_libraries(); }

//-------------------------------------------------------------------------------------------------

void Lua::RegisterAPI() {
    // luabridge::getGlobalNamespace(m_Lua.get()).addCFunction("debug", &lua_DebugPrint);
}

//-------------------------------------------------------------------------------------------------

bool Lua::LoadLibrary(const char *c) {
    char buf[256];
    sprintf(buf, "require(\"%s\")", c);

    return ExecuteScriptChunk(buf);
}

bool Lua::ExecuteScriptFile(const char *fname) {
    auto L = GetState();
    int status = luaL_dofile(L, fname);
    if (status) {
        luaL_traceback(L, L, "", 1);
        printf("Unable to execute file '%s'\nTrace:\n%s\n\n", fname, lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    lua_settop(L, 0);
    return true;
}

bool Lua::ExecuteScriptChunk(const char *code, const char *name) {
    auto L = GetState();
    int status = luaL_dostring(L, code);
    if (status) {
        if (lua_isnil(L, -1))
            lua_pop(L, 1);
        luaL_traceback(L, L, "", 1);
        printf("Unable to execute chunk '%s'\nTrace:\n%s\n\n", name ? name : code, lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    lua_settop(L, 0);
    return true;
}

bool Lua::ExecuteChunk(const unsigned char *data, size_t len, const char *name) {
    auto L = GetState();
    int status = luaL_loadbuffer(L, (const char *)data, len, name);
    if (status) {
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        printf("Unable to load script %s\n", name);
        return false;
    }
    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status) {
        luaL_traceback(L, L, "", 1);
        printf("Unable to execute '%s'\nTrace:\n%s\n\n", name, lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

//-------------------------------------------------------------------------------------------------

bool Lua::Initialize() {
    RegisterAPI();

    return true;
}

} // namespace MoonGlare::Tools::VfsCli