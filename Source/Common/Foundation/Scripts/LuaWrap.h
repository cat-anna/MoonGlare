#pragma once

#include <lua.hpp>

namespace MoonGlare::Scripts {

template<class T> inline T Lua_to(lua_State *lua, int idx);
template<> inline bool Lua_to<bool>(lua_State *lua, int idx) { return lua_toboolean(lua, idx) != 0; }
template<> inline float Lua_to<float>(lua_State *lua, int idx) { return static_cast<float>(lua_tonumber(lua, idx)); }
template<> inline double Lua_to<double>(lua_State *lua, int idx) { return static_cast<double>(lua_tonumber(lua, idx)); }
template<> inline const char* Lua_to<const char*>(lua_State *lua, int idx) { return lua_tostring(lua, idx); }
template<> inline int Lua_to<int>(lua_State *lua, int idx) { return lua_tointeger(lua, idx); }

inline void Lua_push(lua_State *lua, bool t) { lua_pushboolean(lua, t); }
inline void Lua_push(lua_State *lua, float t) { lua_pushnumber(lua, static_cast<LUA_NUMBER>(t)); }
inline void Lua_push(lua_State *lua, double t) { lua_pushnumber(lua, static_cast<LUA_NUMBER>(t)); }
inline void Lua_push(lua_State *lua, const char* t) { lua_pushstring(lua, t); }
inline void Lua_push(lua_State *lua, int t) { lua_pushinteger(lua, t); }
inline void Lua_push(lua_State *lua, unsigned t) { Lua_push(lua, static_cast<int>(t)); }
inline void Lua_push(lua_State *lua, void* t) { lua_pushlightuserdata(lua, t); }

template <typename T, typename ... ARGS>
inline void Lua_push(lua_State *lua, T&& t, ARGS&& ... args) {
    Lua_push(lua, std::forward<T>(t));
    Lua_push(lua, std::forward<ARGS>(args)...);
}

template<class T> bool inline Lua_is(lua_State *lua, int idx);
template<> inline bool Lua_is<bool>(lua_State *lua, int idx) { return lua_isboolean(lua, idx); }
template<> inline bool Lua_is<float>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }
template<> inline bool Lua_is<double>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }
template<> inline bool Lua_is<const char*>(lua_State *lua, int idx) { return lua_isstring(lua, idx) != 0; }
template<> inline bool Lua_is<int>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }

template<> inline unsigned Lua_to<unsigned>(lua_State *lua, int idx) { return static_cast<int>(Lua_to<int>(lua, idx)); }
template<> inline bool Lua_is<unsigned>(lua_State *lua, int idx) { return lua_isnumber(lua, idx) != 0; }

template<class ... ARGS>
void lua_PushCClosure(lua_State *lua, lua_CFunction cfunc, ARGS&& ...args) {
    Lua_push(lua, std::forward<ARGS>(args)...);
    lua_pushcclosure(lua, cfunc, sizeof...(ARGS));
}

template<typename T>
static std::optional<T> GetTableField(lua_State *lua, int idx, const char *field) {
    lua_getfield(lua, idx, field);
    if (lua_isnil(lua, -1)) {
        lua_pop(lua, 1);
        return std::nullopt;
    } else {
        auto t = luabridge::Stack<T>::get(lua, lua_gettop(lua));
        lua_pop(lua, 1);
        return std::move(t);
    }
}

inline int GetAbsoluteindex(lua_State *lua, int i) {
    return (i > 0 ? i : lua_gettop(lua) + i + 1);
}

}
