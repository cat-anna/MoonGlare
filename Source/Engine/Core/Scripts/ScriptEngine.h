#ifndef ScriptEngineH
#define ScriptEngineH

namespace MoonGlare {
namespace Core {
namespace Scripts {

class cScriptEngine final : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(cScriptEngine, cRootClass);
public:
	cScriptEngine();
	virtual ~cScriptEngine();

	bool Initialize();
	bool Finalize();

	template<class RET, class ... Types>
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
			AddLogf(Error, "Runtime script error! Function '%s', message: '%s'",
					FuncName, e.what());
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
		catch (...) {
			AddLogf(Error, "Runtime script error! Function '%s' failed with unknown message!", FuncName);
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		}
	}

	ApiInitializer GetApiInitializer() {
		return luabridge::getGlobalNamespace(m_Lua);
	}

	int ExecuteCode(const string& code, const char *CodeName = nullptr) {
		return LoadCode(code.c_str(), code.length(), CodeName);
	}
  
	int ExecuteCode(const char *code, unsigned len, const char *CodeName = nullptr) {
		return LoadCode(code, len, CodeName);
	}

	lua_State *GetLua() { return m_Lua; }
	std::recursive_mutex& GetLuaMutex() { return m_Mutex; }

	///script will be on top of lua stack
	bool GetRegisteredScript(const char* name);

	void Step(const MoveConfig & conf);
	
	template<typename T>
	void RegisterLuaSettings(T *t, const char *Name) {
		LOCK_MUTEX_NAMED(GetLuaMutex(), lock);
		GetApiInitializer().beginNamespace("Settings").addPtrVariable(Name, t);
	}

	void CollectGarbage();
	void PrintMemoryInfo();
	void PrintMemoryUsage() const;
	float GetMemoryUsage() const;
protected:
	int RegisterModifyScript(lua_State *lua);
	int RegisterNewScript(lua_State *lua);

	bool ConstructLuaContext();
	bool ReleaseLuaContext();

	DeclarePerformanceCounter(ExecutionCount);
	DeclarePerformanceCounter(ExecutionErrors);
private:
	lua_State *m_Lua = nullptr;
	mutable std::recursive_mutex m_Mutex;

//old
	int LoadCode(const char* Code, unsigned len, const char* ChunkName);
public:
	struct ScriptCode {
		enum class Source {
			File, Code,
		};
		Source Type;
		string Name;
		string Data;
	};

	void LoadAllScripts();
	void RegisterScript(string Name);
	void LoadCode(string code);
	/** Changes code of specified chunk. Does not reload the code. */
	void SetCode(const string& ChunkName, string Code);

	void DumpScripts(std::ostream &out);

	using EnumerateFunc = std::function<void(const ScriptCode &code)>;
	void EnumerateScripts(EnumerateFunc func);

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	std::list<ScriptCode> m_ScriptCodeList;
	int m_CurrentGCStep;
	int m_CurrentGCRiseCounter;
	float m_LastMemUsage;
	void LoadAllScriptsImpl();
};

#ifndef DISABLE_SCRIPT_ENGINE

#define __SCRIPT_FAST_RUN(F, ...)						::MoonGlare::Core::GetScriptEngine()->RunFunction<int>(F.c_str(), __VA_ARGS__);		 		
#define __SCRIPT_FAST_RUN_Ex(RET, F, ...)				::MoonGlare::Core::GetScriptEngine()->RunFunction<RET>(F.c_str(), __VA_ARGS__);	
#define __SCRIPT_GET_HANDLE(NAME)						auto *NAME = GetScriptEvents()

#define	SCRIPT_INVOKE(F, ...) 											\
	do {																\
		__SCRIPT_GET_HANDLE(events);									\
		SCRIPT_CLASS_INSTANCE_VALIDATION();								\
		if (events->F.empty()) 											\
			return 0;													\
		else															\
			return __SCRIPT_FAST_RUN(events->F, this, __VA_ARGS__);		\
	} while(false)														

#define	SCRIPT_INVOKE_NORETURN(F, ...) 									\
	do {																\
		__SCRIPT_GET_HANDLE(events);									\
		SCRIPT_CLASS_INSTANCE_VALIDATION();								\
		if (!events->F.empty()) 										\
			__SCRIPT_FAST_RUN(events->F, this, __VA_ARGS__);			\
	} while(false)	

#define	SCRIPT_INVOKE_RESULT(RESULT, F, ...) 							\
	do {																\
		__SCRIPT_GET_HANDLE(events);									\
		SCRIPT_CLASS_INSTANCE_VALIDATION();								\
		if (!events->F.empty()) 										\
			RESULT = __SCRIPT_FAST_RUN_Ex(decltype(RESULT), events->F, this, __VA_ARGS__);	\
	} while(false)	

#define	SCRIPT_INVOKE_STATIC(F, ...) 									\
	do {																\
		if (F.empty()) 													\
			return 0;													\
		else															\
			return __SCRIPT_FAST_RUN(F, __VA_ARGS__);					\
	} while(false)														

#define	SCRIPT_INVOKE_STATIC_NORETURN(F, ...) 							\
	do {																\
		if (!F.empty()) 												\
			__SCRIPT_FAST_RUN(F, __VA_ARGS__);							\
	} while(false)		

#define	SCRIPT_INVOKE_STATIC_RESULT(RESULT, F, ...) 					\
	do {																\
		if (F.empty()) 													\
			RESULT = 0;													\
		else															\
			RESULT = __SCRIPT_FAST_RUN(F, __VA_ARGS__);					\
	} while(false)		

#else

#define	SCRIPT_INVOKE(...) 						do { return 0; } while(false)
#define	SCRIPT_INVOKE_NORETURN(...)				do { } while(false)
#define	SCRIPT_INVOKE_RESULT(...)				do { } while(false)
#define SCRIPT_INVOKE_STATIC(...)				do { return 0; } while(false)
#define SCRIPT_INVOKE_STATIC_NORETURN(...)		do { } while(false)

#endif

} //namespace Scripts

inline Scripts::cScriptEngine* GetScriptEngine() { return Scripts::cScriptEngine::Instance(); }

} //namespace Core
} //namespace MoonGlare

#endif
