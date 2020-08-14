#pragma once

extern "C" {
#define LUA_COMPAT_5_1
#if __has_include(<luajit/lua.h>)
#include <luajit/lauxlib.h>
#include <luajit/lua.h>
#include <luajit/lualib.h>
#else
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#endif
}
// #include <LuaBridge/LuaBridge.h>

#ifndef LUA_OK
#define LUA_OK 0
#endif

#if !defined(lua_writestringerror)
#define lua_writestringerror(s, p) (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

#if !defined(lua_writeline)
#define lua_writeline() (lua_writestring("\n", 1), fflush(stdout))
#endif

#if !defined(lua_writestring)
#define lua_writestring(s, l) fwrite((s), sizeof(char), (l), stdout)
#endif
