#pragma once

#include "LuaReader.h"

namespace MoonGlare::Scripts {

inline bool CallFunction(lua_State *lua, int args, int rets) {
    assert(lua);

    switch (lua_pcall(lua, args, rets, 0)) {
    case 0:
        return true;
    case LUA_ERRRUN:
        AddLog(Error, "Call error: a runtime error!");
        break;
    case LUA_ERRERR:
        AddLog(Error, "Call error: error while running the error handler function!");
        break;
    case LUA_ERRMEM:
        AddLog(Error, "Call error: Memory allocation failed!");
        break;
    }
    AddLog(Error, "Call error message: " << lua_tostring(lua, -1));
    lua_pop(lua, 1);
    return false;
}

inline bool ExecuteString(lua_State *lua, const char* Code, unsigned len, const char* ChunkName, int rets = 0) {
    assert(lua);
    assert(Code);
    assert(len > 0);

    LuaCStringReader reader(Code, len);
    int result = lua_load(lua, &reader.Reader, &reader, ChunkName);

    switch (result) {
    case 0:
        AddLogf(Debug, "Loaded lua chunk: '%s'", ChunkName ? ChunkName : "?");
        return CallFunction(lua, 0, rets);
    case LUA_ERRSYNTAX:
        AddLogf(Error, "Unable to load script: Syntax Error!\nName:'%s'\nError string: '%s'\ncode: [[%s]]", ChunkName ? ChunkName : "?", lua_tostring(lua, -1), Code);
        break;
    case LUA_ERRMEM:
        AddLog(Error, "Unable to load script: Memory allocation failed!");
        break;
    }

    lua_pop(lua, 1);
    return false;
}

inline bool ExecuteString(lua_State *lua, const std::string &code, const char* ChunkName, int rets = 0) {
    return ExecuteString(lua, code.c_str(), code.size(), ChunkName, rets);
}
inline bool ExecuteString(lua_State *lua, const unsigned char* Code, unsigned len, const char* ChunkName, int rets = 0) {
    return ExecuteString(lua, (const char *)Code, len, ChunkName, rets);
}

} //namespace MoonGlare::Scripts 
