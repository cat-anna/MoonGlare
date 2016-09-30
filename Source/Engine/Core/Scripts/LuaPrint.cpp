#include <pch.h>
#include <nfMoonGlare.h>

#include <Core/Console.h>

static void LuaToLog(lua_State *L, std::ostream &lg, int parambegin = 1, bool Separate = false) {
	//	void* ptr;
	//	cObject *obj = 0;
	for (int i = parambegin, n = lua_gettop(L); i <= n; i++) {
		auto type = lua_type(L, i);
		switch (type) {
		case LUA_TNUMBER:
		{
			lua_Number v = lua_tonumber(L, i);
			lg << v;
			break;
		}
		case LUA_TSTRING:
		{
			const char *s = lua_tostring(L, i);
			lg << s;
			break;
		}
		case LUA_TBOOLEAN:
			lg << std::boolalpha << (lua_toboolean(L, i) != 0);
			break;
		case LUA_TUSERDATA:
		{
			unsigned *ptr = (unsigned*)lua_touserdata(L, i);
			try {
				cRootClass *obj = dynamic_cast<cRootClass*>((Space::RTTI::RTTIObject*)(void*)*(ptr + 1));
				if (!obj)
					throw false;
				lg << "[" << typeid(*obj).name() << "]";
			}
			catch (...) {
				lg << "[err: unknown pointer type!]";
			}
			// ostr << "{USR: " << ptr << " -> " << (void*)*(ptr+1) << "}";
			break;
		}
#ifdef DEBUG
		case LUA_TNIL:
			lg << "[NULL]";
			break;
		case LUA_TFUNCTION:
			lg << "[Function@" << lua_tocfunction(L, i) << "]";
			break;
		case LUA_TTABLE:
			lg << "[TABLE]";
			break;
#endif
		default:
#ifdef DEBUG
			lg << "[?=" << type << "]";
#endif
			break;
		}
		if (Separate && i < n)
			lg << ", ";
	}
}

static int Lua_print(lua_State *L) {
	std::ostringstream out;
	for (int i = 1, n = lua_gettop(L); i <= n; i++) {
		switch (lua_type(L, i)) {
		case LUA_TNUMBER:
			out << lua_tonumber(L, i);
			break;
		case LUA_TSTRING:
		{
			const char *s = lua_tostring(L, i);
			out << (s ? s : "{NULL}");
			break;
		}
		case LUA_TBOOLEAN:
			out << std::boolalpha << (lua_toboolean(L, i) != 0);
			break;
#ifdef DEBUG
		case LUA_TUSERDATA:
		{
			int top = lua_gettop(L);
			lua_pushvalue(L, i);
			lua_getglobal(L, "tostring");
			lua_insert(L, -2);
			if (lua_pcall(L, 1, 1, 0) == 0) {
				out << lua_tostring(L, -1);
			} else
				out << "[PTR]";
			lua_settop(L, top);
			break;
		}
		case LUA_TNIL:
			out << "[NULL]";
			break;
		case LUA_TFUNCTION:
			out << "[Function@" << lua_tocfunction(L, i) << "]";
			break;
		case LUA_TTABLE:
			out << "[TABLE]";
			break;
#endif
		default:
#ifdef DEBUG
			out << "[?=" << lua_type(L, i) << "]";
#endif
			break;
		}
	}
	string str = out.str();
	if (ConsoleExists())
		GetConsole()->AsyncLine(str);
	AddLog(Info, str);
	return 0;
}

static int Lua_error(lua_State *L) {
	std::ostringstream oss;
	LuaToLog(L, oss);
	AddLog(Error, oss.str());
	return 0;
}

static int Lua_warning(lua_State *L) {
	std::ostringstream oss;
	LuaToLog(L, oss);
	AddLog(Warning, oss.str());
	return 0;
}

static int Lua_hint(lua_State *L) {
	std::ostringstream oss;
	LuaToLog(L, oss);
	AddLog(Hint, oss.str());
	return 0;
}

#ifdef DEBUG_SCRIPTAPI
static int Lua_debug(lua_State *L) {
	std::ostringstream oss;
	LuaToLog(L, oss);
	AddLog(Debug, oss.str());
	return 0;
}
#endif

void LogScriptApi(ApiInitializer &api) {
	api
		.addCFunction("print", Lua_print)
		.beginNamespace("Log")
			.addCFunction("Error", Lua_error)
			.addCFunction("Warning", Lua_warning)
			.addCFunction("Hint", Lua_hint)
			.addCFunction("Console", Lua_print)
#ifdef DEBUG_SCRIPTAPI
			.addCFunction("Debug", Lua_debug)
#endif
		.endNamespace()
		;
}

RegisterApiNonClass(LogScriptApi, &LogScriptApi, nullptr);