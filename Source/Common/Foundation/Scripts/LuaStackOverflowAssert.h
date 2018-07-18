#pragma once

#include <lua.hpp>
#include <string>

namespace MoonGlare::Scripts {

struct LuaStackOverflowAssert {
    LuaStackOverflowAssert(lua_State *lua) {
        m_lua = lua;
        m_InitStack = lua_gettop(lua);
        m_CheckStack = m_InitStack;
        m_RetArgs = 0;
    }
    ~LuaStackOverflowAssert() {
        Test();
    }
    void Test() {
        m_CheckStack = lua_gettop(m_lua);
        if (m_CheckStack != (m_InitStack + m_RetArgs))
            AddLogf(Error, "Lua stack [over/under]flow! Expected:%d Actual:%d", m_CheckStack, m_InitStack);
    }
    int ReturnArgs(int argc) {
        m_RetArgs = argc;
        return argc;
    }
private:
    lua_State * m_lua;
    int m_InitStack;
    int m_CheckStack;
    int m_RetArgs;
};

}