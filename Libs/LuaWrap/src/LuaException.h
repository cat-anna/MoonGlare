#pragma once

namespace LuaWrap {

class eLuaPanic : public std::runtime_error {
public:
	eLuaPanic(const std::string &msg) : std::runtime_error(msg) {}

	static int ThrowPanicHandler(lua_State *L) {
		const char *m = lua_tostring(L, 1);
		const char *cs = nullptr;
		lua_getfield(L, LUA_GLOBALSINDEX, "debug");
		if (!lua_isnil(L, -1)) {
			lua_getfield(L, -1, "traceback");
			if (!lua_isnil(L, -1)) {
				lua_pushvalue(L, 1);
				lua_pushinteger(L, 2);
				lua_call(L, 2, 1);
				cs = lua_tostring(L, 1);
			}
		}
		std::string msg = "Lua panic! ";
		if (m) {
			msg += m;
			msg += "; ";
		}
		if (cs) {
			msg += "Stack trace: ";
			msg += cs;
		} else {
			msg += "Failed to get stack trace";
		}
		lua_pop(L, 2);
		throw eLuaPanic(std::move(msg));
	}
};

}
