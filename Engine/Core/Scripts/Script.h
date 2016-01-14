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
	GABI_DECLARE_CLASS_NOCREATOR(Script, cRootClass)
public:
	Script();
	~Script();
	bool Initialize();
	bool Finalize();

	int LoadCode(const char* Code, unsigned len, const char* ChunkName);

#ifdef _USE_API_GENERATOR_
	template<class CALLER, class ... Types>
	int RunFunction(const string &Function, CALLER *const caller, Types ... args) { return 0; }
	ApiInitializer GetApiInitializer(){
		return ApiDefAutoGen::Namespace::Begin();
	}
#elif defined(_DISABLE_SCRIPT_ENGINE_)
	template<class CALLER, class ... Types>
	int RunFunction(const string &Function, CALLER *const caller, Types ... args) { return 0; }
	ApiInitializer GetApiInitializer() {
		return DummyApiInitializer();
	}
#else
	template<class RET, class ... Types>
	RET RunFunction(const char *FuncName, Types ... args) {
		try {
			//AddLogf(Debug, "Script owner: %s, call to: '%s'", GetOwnerProxy()->GetThreadSignature(), FuncName);
			IncrementPerformanceCounter(ExecutionCount);
			luabridge::LuaRef fun = luabridge::getGlobal(m_Lua, FuncName);
			luabridge::LuaRef ret = fun(args...);
			return ret.cast<RET>();
		} catch (const std::exception & e) {
			AddLogf(Error, "Runtime script error! Function '%s', Script owner: %s, message: '%s'", 
					FuncName, GetOwnerProxy()->GetThreadSignature(), e.what());
			IncrementPerformanceCounter(ExecutionErrors);
			return RET(0);
		} catch (...) {
			AddLogf(Error, "Runtime script error! Function '%s' failed with unknown message!", 
					FuncName, GetOwnerProxy()->GetThreadSignature());
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
		sf_HasCode			 = 0x0001,
		sf_Ready			 = 0x0002,
	};
	DefineFlagGetter(m_Flags, sf_Ready, Ready);

	DefineDirectSetGet(OwnerProxy, ScriptProxy*)

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	lua_State *m_Lua;
	unsigned m_Flags;
	ScriptProxy *m_OwnerProxy;

	DefineFlagSetter(m_Flags, sf_Ready, Ready);

	DeclarePerformanceCounter(ExecutionCount);
	DeclarePerformanceCounter(ExecutionErrors);

	static int Lua_panic(lua_State *L);
	static int Lua_ProcessResult(lua_State *L);
};

} // namespace Scripts
} // namespace Core
#endif // CSCRIPT_H_
