#pragma once

#include "LuaPanic.h"
#include "LuaWrap.h"

namespace MoonGlare::Scripts {

int LuaErrorHandler(lua_State *L);

// Wants single string parameter, pops it from stack 
int LuaTraceback(lua_State *L);

int LuaPanicHandler(lua_State *L);

inline bool LuaSafeCall(lua_State *lua, int args, int rets, const char *CaleeName, int errf = 0) {
	try {
		AddLogf(ScriptCall, "Call to %s", CaleeName);
		return lua_pcall(lua, args, rets, errf) == 0;
	}
	catch (LuaPanic &err) {
		AddLogf(Error, "Failure during call to %s message: %s", CaleeName, err.what());
		return false;
	}
}

} //namespace MoonGlare::Scripts 
