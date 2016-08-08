/*
 * cScript.cpp
 *
 *  Created on: 16-01-2014
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/Console.h>
#include "GlobalContext.h"

namespace Core {
namespace Scripts {

#define lua_registerF(V, N, Lua) lua_pushcfunction(Lua, V);  lua_setglobal(Lua, N)
#define RegisterLuaFunction(A, B, C) lua_registerF(A, B, C)

#if 0
const char LUA_DEFAULT_SCRIPT_FUNCTION[] = "StringScriptLoader";
const char LUA_DEFAULT_SCRIPT_CODE[] = "\
function StringScriptLoader(code, name)\n\
	local f;\n\
	local msg;\n\
	f, msg = load(code, name);\n\
	if f == nil then\n\
		Log.Error(1, \"Script compile error! Chunk name: \'\", name, \"\', Mesage: \'\", msg, \"\'\");\n\
		return 1;\n\
	end\n\
	return ProcessResult(name, pcall(f));\n\
end";
#endif // 0

static void LuaToLog(lua_State *L, std::ostream &lg, int parambegin = 1, bool Separate = false) {
	//	void* ptr;
	//	cObject *obj = 0;
	for (int i = parambegin, n = lua_gettop(L); i <= n; i++) {
		auto type = lua_type(L, i);
		switch (type) {
		case LUA_TNUMBER: {
			lua_Number v = lua_tonumber(L, i);
			lg << v;
			break;
		}
		case LUA_TSTRING: {
			const char *s = lua_tostring(L, i);
			lg << s;
			break;
		}
		case LUA_TBOOLEAN:
			lg << std::boolalpha << (lua_toboolean(L, i) != 0);
			break;
		case LUA_TUSERDATA: {
			unsigned *ptr = (unsigned*)lua_touserdata(L, i);
			try {
				cRootClass *obj = dynamic_cast<cRootClass*>((Space::RTTI::RTTIObject*)(void*)*(ptr + 1));
				if (!obj)
					throw false;
				lg << "[" << obj->Info() << "]";
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

static int Lua_debug(lua_State *L) {
#ifdef DEBUG_SCRIPTAPI
	std::ostringstream oss;
	LuaToLog(L, oss);
	AddLog(Debug, oss.str());
#endif
	return 0;
}

void LogScriptApi(ApiInitializer &api) {
	api
		.addCFunction("Error", Lua_error)
		.addCFunction("Warning", Lua_warning)
		.addCFunction("Hint", Lua_hint)
		.addCFunction("Console", Lua_print)
		.addCFunction("Debug", Lua_debug)
		;
}

RegisterApiNonClass(LogScriptApi, &LogScriptApi, "Log");

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(Script)
RegisterApiDerivedClass(Script, &Script::RegisterScriptApi);

Script::Script(): BaseClass(), m_Lua(0), m_Flags(0) {
	SetPerformanceCounterOwner(ExecutionErrors);
	SetPerformanceCounterOwner(ExecutionCount);
}

Script::~Script() {
}

void Script::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cScript")
#ifdef DEBUG
		.addFunction("CollectGarbage", &ThisClass::CollectGarbage)
		.addFunction("PrintMemoryUsage", &ThisClass::PrintMemoryUsage)
#endif
	.endClass()
	;
}

bool Script::Initialize() {
	LOCK_MUTEX(m_Mutex);
	if (IsReady()) return true;

	m_Lua = luaL_newstate();
	luaopen_base(m_Lua);
	luaopen_math(m_Lua);
	luaopen_bit(m_Lua);
	luaopen_string(m_Lua);
	luaopen_table(m_Lua);
#ifdef DEBUG
	luaopen_debug(m_Lua);
#endif

	lua_atpanic(m_Lua, Script::Lua_panic);
	//RegisterLuaFunction(Script::Lua_ProcessResult, "ProcessResult", m_Lua);
	RegisterLuaFunction(Lua_print, "print", m_Lua);

	lua_newtable(m_Lua);
	lua_setglobal(m_Lua, "global");

#ifndef _USE_API_GENERATOR_
#ifdef DEBUG
	luabridge::setHideMetatables(false);
#else
	luabridge::setHideMetatables(true);
#endif
#endif

	ApiInit::Initialize(this);

	GlobalContext::Instance()->Install(m_Lua);
	
#if 0
	int ret = luaL_loadstring(m_Lua, LUA_DEFAULT_SCRIPT_CODE);
	switch (ret) {
	case LUA_ERRSYNTAX:
		AddLog(Error, "Default script has syntax error!");
		break;
	case LUA_ERRMEM:
		AddLog(Error, "Memory error while initializing ScriptEngine");
		break;
	}

	if (ret != 0) {
		lua_close(m_Lua);
		m_Lua = 0;
		AddLog(Error, "Initializing Script failed. Error code: 0x" << std::hex << ret);
		return false;
	}

	lua_pcall(m_Lua, 0, 0, 0);
#endif // 0

	lua_gc(m_Lua, LUA_GCCOLLECT, 0);
	lua_gc(m_Lua, LUA_GCSTOP, 0);
	PrintMemoryUsage();

	SetReady(true);
	return true;
}

bool Script::Finalize() {
	LOCK_MUTEX(m_Mutex);
	if (!IsReady())
		return true;
	PrintMemoryUsage();
	lua_close(m_Lua);
	m_Lua = 0;
	SetReady(false);
	return true;
}

int Script::LoadCode(const char* Code, unsigned len, const char* ChunkName) {
	LOCK_MUTEX(m_Mutex);
	if (!IsReady()) {
		AddLog(Error, "Unable to load code. Script is not initialized.");
		return -1;
	}

	Utils::Scripts::LuaCStringReader reader(Code, len);

	int result = lua_load(m_Lua, &reader.Reader, &reader, ChunkName);

	switch (result) {
	case 0:
		if (lua_pcall(m_Lua, 0, 0, 0) != 0) {
			AddLog(Error, "Lua error: " << lua_tostring(m_Lua, -1));
			//throw std::runtime_error();
			return -1;
		}
		break;
	case LUA_ERRSYNTAX:
		AddLogf(Error, "Unable to load script: Syntax Error!\nName:'%s'\nError string: '%s'\ncode: [[%s]]", ChunkName, lua_tostring(m_Lua, -1), Code );
		break;
	case LUA_ERRMEM:
		AddLog(Error, "Unable to load script: Memory allocation failed!");
		break;
	}

#if 0
	lua_getglobal(m_Lua, LUA_DEFAULT_SCRIPT_FUNCTION);
	lua_pushlstring(m_Lua, Code, len);
	if (ChunkName)
		lua_pushstring(m_Lua, ChunkName);
	else
		lua_pushnil(m_Lua);
	//lua_pushboolean(m_Lua, PrintResult);

	if (lua_pcall(m_Lua, 2, 1, 0) != 0) {
		AddLog(Error, "Lua error: " << lua_tostring(m_Lua, -1));
		//throw std::runtime_error();
		return -1;
	}

	int r = (int)lua_tonumber(m_Lua, -1);
	lua_pop(m_Lua, 1);
#endif // 0

	return result;
}

//-------------------------------------------------------------------------------------

void Script::CollectGarbage() {
	LOCK_MUTEX(m_Mutex);
#ifdef DEBUG
	float prev = GetMemoryUsage();
	lua_gc(m_Lua, LUA_GCCOLLECT, 0);
	float next = GetMemoryUsage();
	AddLogf(Debug, "Finished lua garbage collection. %.2f -> %.2f kb (released %.2f kb)", prev, next, prev - next);
#else
	lua_gc(m_Lua, LUA_GCCOLLECT, 0);
#endif
}

float Script::GetMemoryUsage() const {
	LOCK_MUTEX(m_Mutex);
	return (float)lua_gc(m_Lua, LUA_GCCOUNT, 0) + (float)lua_gc(m_Lua, LUA_GCCOUNTB, 0) / 1024.0f;
}

void Script::PrintMemoryUsage() const {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	AddLogf(Performance, "Lua memory usage: %.2fkb ", GetMemoryUsage());
#endif
}

//-------------------------------------------------------------------------------------

void traceback(lua_State *L) {
#ifdef DEBUG
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (lua_isnil(L, -1)) {
		return;
	}
	lua_getfield(L, -1, "traceback");
	if (lua_isnil(L, -1)) {
		return;
	}
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);
	lua_call(L, 2, 1);

	const char *cs = lua_tostring(L, 1);
	AddLogf(Error, "Lua callstack:\n%s", cs);
	lua_pop(L, 2);
#endif
}

int Script::Lua_panic(lua_State *L) {
	const char *m = lua_tostring(L, 1);
	AddLogf(Error, "Lua panic! message: %s", m);
	traceback(L);
	if (!m)
		throw eLuaPanic("NO MESSAGE");
	throw eLuaPanic(m);
}

#if 0
int Script::Lua_ProcessResult(lua_State *L) {
	int n = lua_gettop(L);
	if (n < 2) {
		AddLog(Error, "Attempt to call ProcessResult without enough paramerters!");
		lua_pushinteger(L, 2);
		return 1;
	}
	int doprint = lua_toboolean(L, 1);
	int callsucces = lua_toboolean(L, 2);

	if (!callsucces) {
		AddLog(Error, "Script runtime error! Message: '" << lua_tostring(L, 3) << "'");
		lua_pushinteger(L, 3);
		return 1;
	}

	if (doprint && n > 2) {
		std::stringstream ss;
		if (n > 3)
			ss << "Returned values: ";
		else
			ss << "Returned value: ";
		LuaToLog(L, ss, 3, true);
		AddLog(Hint, ss.str());
	}
	lua_pushinteger(L, lua_tointeger(L, 3));
	// lua_pushinteger(L, 4);
	return 1;
}
#endif // 0

} // namespace Scripts
} // namespace Core
