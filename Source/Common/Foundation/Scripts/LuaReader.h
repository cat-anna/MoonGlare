#pragma once

#include <lua.hpp>
#include <string>

namespace MoonGlare::Scripts {

struct LuaStringReader {
    LuaStringReader(const std::string *string) : m_String(string), m_Finished(false) {}
    static const char * Reader(lua_State *L, void *data, size_t *size) {
        return ((LuaStringReader*)data)->ReaderImpl(L, size);
    }
private:
    const char * ReaderImpl(lua_State *L, size_t *size) {
        if (m_Finished) {
            *size = 0;
            return nullptr;
        }
        m_Finished = true;
        *size = m_String->length();
        return m_String->c_str();
    }
    bool m_Finished;
    const std::string *m_String;
};

struct LuaCStringReader {
    LuaCStringReader(const char *string, size_t length) : m_String(string), m_Length(length), m_Finished(false) {}
    static const char * Reader(lua_State *L, void *data, size_t *size) {
        return ((LuaCStringReader*)data)->ReaderImpl(L, size);
    }
private:
    const char * ReaderImpl(lua_State *L, size_t *size) {
        if (m_Finished) {
            *size = 0;
            return nullptr;
        }
        m_Finished = true;
        *size = m_Length;
        return m_String;
    }
    bool m_Finished;
    size_t m_Length;
    const char *m_String;
};

}
