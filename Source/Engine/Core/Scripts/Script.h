/*
 * cScript.h
 *
 *  Created on: 16-01-2014
 *      Author: Paweu
 */
#ifndef CSCRIPT_H_
#define CSCRIPT_H_
namespace Core {
namespace Scripts {

class eLuaPanic: public std::runtime_error {
public:
	eLuaPanic(const string &msg): std::runtime_error(msg) {}
};

class Script : public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(Script, cRootClass)
public:
	Script();
	~Script();
	bool Initialize();
	bool Finalize();

	int LoadCode(const char* Code, unsigned len, const char* ChunkName);

#ifdef _DISABLE_SCRIPT_ENGINE_
	template<class CALLER, class ... Types>
	int RunFunction(const string &Function, CALLER *const caller, Types ... args) { return 0; }
	ApiInitializer GetApiInitializer() {
		return DummyApiInitializer();
	}
#else
	template<class RET, class ... Types>
	RET RunFunction(const char *FuncName, Types ... args) {
		AddLogf(ScriptCall, "Call to: '%s'", FuncName);
		LOCK_MUTEX(m_Mutex);
		try {
			IncrementPerformanceCounter(ExecutionCount);
			luabridge::LuaRef fun = luabridge::getGlobal(m_Lua, FuncName);
			luabridge::LuaRef ret = fun(args...);
			return ret.cast<RET>();
		} catch (const std::exception & e) {
			AddLogf(Error, "Runtime script error! Function '%s', message: '%s'", 
					FuncName, e.what());
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		} catch (...) {
			AddLogf(Error, "Runtime script error! Function '%s' failed with unknown message!", FuncName);
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
	}
	ApiInitializer GetApiInitializer() {
		return luabridge::getGlobalNamespace(m_Lua);
	}
#endif

	/** Returns memory used by lua in kbytes */
	float GetMemoryUsage() const;
	void CollectGarbage();
	void PrintMemoryUsage() const;

	enum {
		sf_Ready			 = 0x0002,
	};
	DefineFlagGetter(m_Flags, sf_Ready, Ready);

	static void RegisterScriptApi(ApiInitializer &api);

	lua_State* GetLuaState() { return m_Lua; }
	std::recursive_mutex& GetMutex() { return m_Mutex; }
protected:
	lua_State *m_Lua;
	mutable std::recursive_mutex m_Mutex;
	unsigned m_Flags;

	DefineFlagSetter(m_Flags, sf_Ready, Ready);

	DeclarePerformanceCounter(ExecutionCount);
	DeclarePerformanceCounter(ExecutionErrors);

	static int Lua_panic(lua_State *L);
	static int Lua_ProcessResult(lua_State *L);
};

} // namespace Scripts
} // namespace Core
#endif // CSCRIPT_H_
