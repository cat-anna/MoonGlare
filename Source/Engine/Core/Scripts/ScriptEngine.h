#ifndef ScriptEngineH
#define ScriptEngineH

namespace Core {
namespace Scripts {

class cScriptEngine final : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(cScriptEngine, cRootClass);
public:
	cScriptEngine();
	virtual ~cScriptEngine();

	template<class RET, class ... Types>
	RET RunFunction(const char *FuncName, Types ... args) {
		return m_Script->RunFunction<RET>(FuncName, std::forward<Types>(args)...);
	}

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
			ScriptsLoaded = 0x02,
		};
	};

	DefineFlagGetter(m_Flags, Flags::Ready, Ready);
	DefineFlagGetter(m_Flags, Flags::ScriptsLoaded, ScriptsLoaded);

	void Step(const MoveConfig &config);

	void CollectGarbage();
	void PrintMemoryInfo();

	bool Initialize();
	bool Finalize();

	bool CreateScript(const std::string& Class, Entity Owner);

	void LoadAllScripts();
	void RegisterScript(string Name);
	void LoadCode(string code);
	/** Changes code of specified chunk. Does not reload the code. */
	void SetCode(const string& ChunkName, string Code);

	void DefferExecution(string fname, int parameter);

	void DumpScripts(std::ostream &out);

	using EnumerateFunc = std::function<void(const ScriptCode &code)>;
	void EnumerateScripts(EnumerateFunc func);
	
	static void RegisterScriptApi(ApiInitializer &api);
protected:
	unsigned m_Flags;
	SharedScript m_Script;
	std::list<ScriptCode> m_ScriptCodeList;

	std::recursive_mutex m_Mutex;


	DefineFlagSetter(m_Flags, Flags::Ready, Ready);
	DefineFlagSetter(m_Flags, Flags::ScriptsLoaded, ScriptsLoaded);
private:
	int m_CurrentGCStep;
	int m_CurrentGCRiseCounter;
	float m_LastMemUsage;
	void LoadAllScriptsImpl();
};

#ifndef DISABLE_SCRIPT_ENGINE
//::Core::Scripts::ScriptProxy::RunFunction<int>(F.c_str(), __VA_ARGS__);

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
} //namespace Core

#endif
