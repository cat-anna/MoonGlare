#ifndef ScriptEngineH
#define ScriptEngineH
namespace Core {
namespace Scripts {

class cScriptEngine : public cRootClass {
	GABI_DECLARE_CLASS_SINGLETON(cScriptEngine, cRootClass);
public:
	cScriptEngine();
	virtual ~cScriptEngine();


	struct ScriptCode {
		enum class Source {
			File, Code,
		};

		Source Type;
		string Name;
		string Data;
	};

	struct Flags {
		enum {
			Ready	= 0x01,
		};
	};

	DefineFlagGetter(m_Flags, Flags::Ready, Ready);

	void CollectGarbage();
	void PrintMemoryInfo();

	bool Initialize();
	bool Finalize();
	bool InitializeScriptProxy(ScriptProxy &proxy, UniqueScript& ptr);
	bool FinalizeScriptProxy(ScriptProxy &proxy, UniqueScript& ptr);
	void KillAllScripts();

	void LoadAllScripts();
	void RegisterScript(string Name);
	void BroadcastCode(string code);
	/** Changes code of specified chunk. Does not reload the code. */
	void SetCode(const string& ChunkName, string Code);

	void DefferExecution(string fname, int parameter);

	void DumpScripts(std::ostream &out);

	using EnumerateFunc = std::function<void(const ScriptCode &code)>;
	void EnumerateScripts(EnumerateFunc func);
	
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	unsigned m_Flags;

	std::list<ScriptCode> m_ScriptCodeList;

	std::recursive_mutex m_Mutex;
	std::list<std::pair<Script*, ScriptProxy*>> m_ScriptList;

	bool ConstructScript(UniqueScript &ptr);
	bool DestroyScript(UniqueScript &ptr);

	DefineFlagSetter(m_Flags, Flags::Ready, Ready);
private:
	DeclarePerformanceCounter(ScriptInstancesConstructed);
	DeclarePerformanceCounter(ScriptInstancesDestroyed);

	void LoadAllScriptsImpl();
};

#ifdef _USE_API_GENERATOR_

#define SCRIPT_INVOKE(F, ...)	 						API_GEN_MAKE_DECL(F, this, this, __VA_ARGS__) 	return 0;
#define SCRIPT_INVOKE_NORETURN(F, ...)					API_GEN_MAKE_DECL(F, this, this, __VA_ARGS__)
#define SCRIPT_INVOKE_RESULT(RESULT, F, ...)			API_GEN_MAKE_DECL(F, this, this, __VA_ARGS__)
#define SCRIPT_INVOKE_STATIC(F, this, ...)	 			API_GEN_MAKE_DECL(F, this, this,  __VA_ARGS__) 	return 0;
#define SCRIPT_INVOKE_STATIC_NORETURN(F, this, ...)		API_GEN_MAKE_DECL(F, this, this,  __VA_ARGS__)

#elif !defined(DISABLE_SCRIPT_ENGINE)

#define __SCRIPT_FAST_RUN(F, ...)						::Core::Scripts::ScriptProxy::RunFunction<int>(F.c_str(), __VA_ARGS__);
#define __SCRIPT_FAST_RUN_Ex(RET, F, ...)				::Core::Scripts::ScriptProxy::RunFunction<RET>(F.c_str(), __VA_ARGS__);
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

#define INVOKE_FUNCTION_noparam(...) 			do { return 0; } while(false)
#define INVOKE_FUNCTION_noparam_noret(...)		do { } while(false)
#define INVOKE_FUNCTION(...) 					do { return 0; } while(false)
#define INVOKE_FUNCTION_NORET(...)				do { } while(false)	

} //namespace Scripts
} //namespace Core
#endif
