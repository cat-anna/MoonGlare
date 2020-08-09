#include "LuaLinter.h"
#include <lua.hpp>
#include <regex>

LuaLinter::LuaLinter() {}

LuaLinter::~LuaLinter() {}

CompilationResult LuaLinter::Compile(const std::string &code) {
    lua_State *lua = luaL_newstate();
    CompilationResult result;

    int r = luaL_loadstring(lua, code.c_str());

    switch (r) {
    case LUA_ERRSYNTAX: {
        result.status = CompilationResult::Status::Failed;
        // const char *c =
        lua_tostring(lua, -1);
        std::string errorstr = lua_tostring(lua, -1);
        std::regex pieces_regex(R"==(\[(.+)\]\:(\d+)\:\ (.+))==", std::regex::icase);
        std::smatch pieces_match;
        if (std::regex_match(errorstr, pieces_match, pieces_regex)) {
            auto line = std::strtol(pieces_match[2].str().c_str(), nullptr, 10);
            result.lineInfo[line] = pieces_match[3];
        }
        break;
    }
    case 0:
        result.status = CompilationResult::Status::Success;
        break;
    case LUA_ERRMEM:
    default:
        result.status = CompilationResult::Status::Error;
        break;
    }

    lua_close(lua);
    return result;
}
