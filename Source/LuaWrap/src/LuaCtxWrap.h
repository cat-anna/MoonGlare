#pragma once
#ifndef LuaCtxWrap
#define LuaCtxWrap

namespace LuaWrap {

struct LuaCtx {
    LuaCtx(lua_State *lua): m_Lua(lua) { }
    lua_State* GetLua() const { return m_Lua; }

    using CFunc = lua_CFunction;

    void Push(bool t) { lua_pushboolean(GetLua(), t); }
    void Push(float t) { lua_pushnumber(GetLua(), static_cast<LUA_NUMBER>(t)); }
    void Push(double t) { lua_pushnumber(GetLua(), static_cast<LUA_NUMBER>(t)); }
    void Push(const char* t) { lua_pushstring(GetLua(), t); }
    void Push(int t) { lua_pushinteger(GetLua(), t); }
    void Push(unsigned t) { Lua_push(GetLua(), static_cast<LUA_NUMBER>(t)); }
    void Push(void* t) { lua_pushlightuserdata(GetLua(), t); }

    template <typename T, typename ...ARGS>
    void Push(T&& t, ARGS&& ...args) {
    	Push(std::forward<T>(t));
    	Push(std::forward<ARGS>(args)...);
    }
    template<typename ... ARGS>
    void PushCClosure(CFunc cfunc, ARGS&& ...args) {
    	Push(std::forward<ARGS>(args)...);
    	lua_pushcclosure(GetLua(), cfunc, sizeof...(ARGS));
    }

    template<typename KEY, typename ...ARGS>
    void SetCClosure(KEY && key, int tidx, CFunc cfunc, ARGS&& ...args) {
    	PushCClosure(cfunc, std::forward<ARGS>(args)...);
        TableSet(tidx, std::forward<KEY>(key));
    }

    int Top() { return lua_gettop(GetLua()); }
    int TypeAt(int index) { return lua_type(GetLua(), index; }

    void TableGet(int index) { lua_gettable(GetLua(), index); }
    void TableGet(int index, const char *Name) { lua_getfield(GetLua(), index, Name); }
    template<typename T>
    void TableGet(int index, T && t) {
        Push(std::forward<T>(t));
        TableGet(index);
    }

    void TableSet(int index) { lua_settable(GetLua(), index); }
    void TableSet(int index, const char *Name) { lua_setfield(GetLua(), index, Name); }
    template<typename T>
    void TableSet(int index, T && t) {
        Push(std::forward<T>(t));
        TableSet(index);
    }

    void DumpStack() {
    	for (int i = 1, top = Top(L); i <= top; i++)
    		printf("%s  ", lua_typename(GetLua(), TypeAt(i)));
    	printf("\n");  /* end the listing */
    }

    template<typename T>
    void GetRegistry(T && t) {
        Push(std::forward<T>(t));
        TableGet(LUA_REGISTRYINDEX);
    }
private:
    lua_State *m_Lua;
};

}

#endif /* end of include guard: LuaCtxWrap */
