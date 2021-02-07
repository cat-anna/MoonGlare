#pragma once

namespace LuaWrap {

struct LuaDeleter {
	void operator() (lua_State *lua) {
		lua_close(lua);
	}
};

}
