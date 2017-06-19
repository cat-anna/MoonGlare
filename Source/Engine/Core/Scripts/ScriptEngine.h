#ifndef ScriptEngineH
#define ScriptEngineH

namespace MoonGlare {
namespace Core {
namespace Scripts {

class ScriptEngine final : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(ScriptEngine, cRootClass);
public:
	ScriptEngine(World *world);
	virtual ~ScriptEngine();

	bool Initialize();
	bool Finalize();

	void Step(const MoveConfig & conf);

	template<class RET = void, class ... Types>
	RET RunChildFunction(const char *Location, const char *FuncName, Types ... args) {
		AddLogf(ScriptCall, "Call to: '%s'", FuncName);
		LOCK_MUTEX(m_Mutex);
		try {
			IncrementPerformanceCounter(ExecutionCount);
			luabridge::LuaRef fun = luabridge::getGlobal(m_Lua, Location)[FuncName];
			luabridge::LuaRef ret = fun(args...);
			return ret.cast<RET>();
		}
		catch (const std::exception & e) {
			AddLogf(Error, "Runtime script error! Function '%s.%s', message: '%s'",	Location, FuncName, e.what());
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
		catch (...) {
			AddLogf(Error, "Runtime script error! Function '%s.%s' failed with unknown message!", Location, FuncName);
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
	}

	template<class RET = void, class ... Types>
	RET RunFunction(const char *FuncName, Types ... args) {
		AddLogf(ScriptCall, "Call to: '%s'", FuncName);
		LOCK_MUTEX(m_Mutex);
		try {
			IncrementPerformanceCounter(ExecutionCount);
			luabridge::LuaRef fun = luabridge::getGlobal(m_Lua, FuncName);
			luabridge::LuaRef ret = fun(args...);
			return ret.cast<RET>();
		}
		catch (const std::exception & e) {
			AddLogf(Error, "Runtime script error! Function '%s', message: '%s'", FuncName, e.what());
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
		catch (...) {
			AddLogf(Error, "Runtime script error! Function '%s' failed with unknown message!", FuncName);
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
	}

	ApiInitializer GetApiInitializer() { return luabridge::getGlobalNamespace(m_Lua); }

	bool ExecuteCode(const char *code, unsigned len, const char *CodeName = nullptr);
	bool ExecuteCode(const string& code, const char *CodeName = nullptr) { return ExecuteCode(code.c_str(), code.length(), CodeName); }

	lua_State *GetLua() { return m_Lua; }
	std::recursive_mutex& GetLuaMutex() { return m_Mutex; }

	//script will be on top of lua stack, on fail stack remain unchanged
	bool GetRegisteredScript(const char* name);
	//result will be on top of lua stack, on fail stack remain unchanged
	bool GetComponentEntryMT(ComponentID cid);
	
	template<typename T>
	void RegisterLuaSettings(T *t, const char *Name) {
		LOCK_MUTEX_NAMED(GetLuaMutex(), lock);
		GetApiInitializer().beginNamespace("Settings").addPtrVariable(Name, t);
	}

	void CollectGarbage();
	void PrintMemoryUsage() const;
	float GetMemoryUsage() const;

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	int RegisterModifyScript(lua_State *lua);
	int RegisterNewScript(lua_State *lua);

	bool ConstructLuaContext();
	bool ReleaseLuaContext();

	void GetScriptTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetScriptTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}
	void GetComponentMTTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetComponentMTTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}
	void *GetScriptTableIndex() { return this; }
	void *GetComponentMTTableIndex() { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 1); }

	DeclarePerformanceCounter(ExecutionCount);
	DeclarePerformanceCounter(ExecutionErrors);
private:
	lua_State *m_Lua = nullptr;
	World *m_world;
	mutable std::recursive_mutex m_Mutex;

//old:
protected:
	int m_CurrentGCStep;
	int m_CurrentGCRiseCounter;
	float m_LastMemUsage;
	void LoadAllScriptsImpl();
};

} //namespace Scripts

inline Scripts::ScriptEngine* GetScriptEngine() { return Scripts::ScriptEngine::Instance(); }

} //namespace Core
} //namespace MoonGlare

#endif
