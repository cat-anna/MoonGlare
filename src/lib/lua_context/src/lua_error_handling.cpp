#include "lua_error_handling.hpp"
#include "lua_context/lua_stack_overflow_assert.hpp"

namespace MoonGlare::Lua {

int LuaErrorHandler(lua_State *L) {
    const char *cs = lua_tostring(L, -1);
    //AddLogf(ScriptRuntime, : %s", cs);

    lua_getglobal(L, "debug");
    if (lua_isnil(L, -1)) {
        return 0;
    }
    lua_getfield(L, -1, "traceback");
    if (lua_isnil(L, -1)) {
        return 0;
    }
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);

    cs = lua_tostring(L, -1);
    AddLogf(ScriptRuntime, "Lua Error : %s", cs);

    lua_pop(L, 2);
    return 0;
}

int LuaTraceback(lua_State *lua) {
    LuaStackOverflowAssert check(lua);

    lua_getglobal(lua, "debug");
    if (lua_isnil(lua, -1)) {
        return 0;
    }
    lua_getfield(lua, -1, "traceback");
    if (lua_isnil(lua, -1)) {
        return 0;
    }
    lua_pushvalue(lua, -3);
    lua_call(lua, 1, 1);

    const char *cs = lua_tostring(lua, -1);
    AddLogf(ScriptRuntime, "%s", cs);
    lua_pop(lua, 3);
    return check.ReturnArgs(-1);
}

int LuaPanicHandler(lua_State *L) {
    const char *m = lua_tostring(L, 1);
    AddLogf(ScriptRuntime, "Lua panic! message: %s", m);
    LuaTraceback(L);
    if (!m) {
        throw LuaPanic("NO MESSAGE");
    }
    throw LuaPanic(m);
}

} // namespace MoonGlare::Lua
