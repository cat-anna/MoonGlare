#pragma once

#include <Foundation/Scripts/LuaStackOverflowAssert.h>

namespace MoonGlare {
namespace Core {
namespace Scripts {

using namespace MoonGlare::Scripts;

class ScriptEngine;
class Script;

class ApiInit {
public:
	static void Initialize(ScriptEngine *s);

	static void RegisterApi(void(*func)(ApiInitializer&), const Space::RTTI::TypeInfo *Class, const Space::RTTI::TypeInfo *BaseClass, const char *where = 0);

	struct ApiRegister {
		template <void(*func)(ApiInitializer&)> 
		struct Function { Function(const char* where = "api") { RegisterApi(func, 0, 0, where); } };

		template <class T, void(*func)(ApiInitializer&)>
		struct Base { Base(const char* where = "api") { RegisterApi(func, T::GetStaticTypeInfo(), nullptr, where); } };

		template <class T, class B, void(*func)(ApiInitializer&)>
		struct Derived { Derived(const char* where = "api") { RegisterApi(func, T::GetStaticTypeInfo(), B::GetStaticTypeInfo(), where); } };
	};
private:
};

#ifndef _DISABLE_SCRIPT_ENGINE_
//detect configuartion private defs
//but only when script api is enabeled

#ifdef _USE_API_GENERATOR_
#define PRIV_SCRIPT_API_GEN
#else
#define PRIV_SCRIPT_API_REGULAR
#endif

#endif

#if !defined(PRIV_SCRIPT_API_REGULAR) && !defined(PRIV_SCRIPT_API_GEN)
#define PRIV_SCRIPT_API_DISABLED
#endif

//----------------------------------------ENABLED-COMMON----------------------------------------------
#if defined(PRIV_SCRIPT_API_REGULAR) || defined(PRIV_SCRIPT_API_GEN)

#define RegisterApiBaseClass(CLASS, FUNCTION)\
	::MoonGlare::Core::Scripts::ApiInit::ApiRegister::Base<CLASS, FUNCTION> ApiReg##CLASS

#define RegisterApiDerivedClass(CLASS, FUNCTION)\
	::MoonGlare::Core::Scripts::ApiInit::ApiRegister::Derived<CLASS, CLASS::BaseClass, FUNCTION> ApiReg##CLASS

#define RegisterApiNonClass(NAME, FUNCTION, ...)\
	::MoonGlare::Core::Scripts::ApiInit::ApiRegister::Function<FUNCTION> ApiReg##NAME{__VA_ARGS__}

#ifdef DEBUG
#define RegisterDebugApi(NAME, FUNCTION, ...)		RegisterApiNonClass(NAME, FUNCTION, __VA_ARGS__)	
#else
#define RegisterDebugApi(...)
#endif

#define RegisterApiInstance(CLASS, GETTER, NAME)\
	template <class T, T*(*func)()>\
	struct CLASS##InstanceReg { \
		CLASS##InstanceReg() {  \
			::MoonGlare::Core::Scripts::ApiInit::RegisterApi(&DoReg, 0, 0, "Inst"); \
		} \
	private:\
		static void DoReg(::ApiInitializer &api) {\
			auto Name = (NAME ? NAME : T::GetStaticTypeInfo()->GetName());\
			api.addProperty<T*, T*>(Name, func, nullptr);\
		}\
	};\
	CLASS##InstanceReg<CLASS, GETTER> ApiInstReg##CLASS{};

#define RegisterApiDynamicInstance(CLASS, GETTER, NAME)\
	RegisterApiInstance(CLASS, GETTER, NAME)

#if 0
	template <class T, T*(*func)()>\
	struct CLASS##InstanceReg { \
		CLASS##InstanceReg() { Core::Scripts::ApiInit::RegisterApi(&DoReg, 0, 0, "Inst"); } \
	private:\
		static int get(lua_State *s) {\
			return func()->PushExactClass(s);\
		}\
		static void DoReg(ApiInitializer &api) {\
			auto Name = (NAME ? NAME : T::GetStaticTypeInfo()->GetName());\
			api.addCFunction(Name, &get);\
		}\
	};\
	CLASS##InstanceReg<CLASS, GETTER> ApiInstReg##CLASS;
#endif
#endif
//-------------------------------------------REGULAR--------------------------------------------------
#ifdef PRIV_SCRIPT_API_REGULAR

#ifdef DEBUG
#define SCRIPT_CLASS_INSTANCE_VALIDATION() \
	do { \
		if(this->GetDynamicTypeInfo() != ThisClass::GetStaticTypeInfo()) \
			AddLogOncef(Script, "Script instance valiation failed!!  caller:%s  handled in class: %s", \
				 GetDynamicTypeInfo()->GetName(), ThisClass::GetStaticTypeInfo()->GetName());\
	 } while(false)
#else
#define SCRIPT_CLASS_INSTANCE_VALIDATION() do { /* NOP */ } while(false)
#endif

#define DECLARE_EXCACT_SCRIPT_CLASS_GETTER() \
	public: virtual int PushExactClass(lua_State *lua) { \
		SCRIPT_CLASS_INSTANCE_VALIDATION(); \
		luabridge::push(lua, this); \
		return 1; \
	}

#define _NOT_FOR_API_GENERATOR(...) __VA_ARGS__

#endif
//-------------------------------------------API-GEN--------------------------------------------------
#ifdef PRIV_SCRIPT_API_GEN

#define DECLARE_EXCACT_SCRIPT_CLASS_GETTER() \
	public: virtual int PushExactClass(lua_State *lua) { return 0; }

#define _NOT_FOR_API_GENERATOR(...) 

#endif
//-------------------------------------------DISABLED-------------------------------------------------
#ifdef PRIV_SCRIPT_API_DISABLED

#define RegisterApiBaseClass(...)
#define RegisterApiDerivedClass(...)
#define RegisterApiNonClass(...)
#define RegisterDebugApi(...)
#define DECLARE_EXCACT_SCRIPT_CLASS_GETTER(...) 

#endif
//---------------------------------------------END---------------------------------------------------

//remove private defs
#undef PRIV_SCRIPT_API_REGULAR
#undef PRIV_SCRIPT_API_GEN
#undef PRIV_SCRIPT_API_DISABLED

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

template <class T>
inline void PublishSelfLuaTable(lua_State *lua, const char *name, T *OwnerPtr, int selfLuaIndex) {
#if DEBUG
	LuaStackOverflowAssert check(lua);
	lua_pushvalue(lua, selfLuaIndex);
	char buf[64];
	sprintf_s(buf, "%p_%s", OwnerPtr, name);
	lua_setglobal(lua, buf);
	AddLogf(Debug, "Adding global registry mapping: %s by %p(%s)", buf, OwnerPtr, typeid(*OwnerPtr).name());
#endif
}

template <class T>
inline void HideSelfLuaTable(lua_State *lua, const char *name, T *OwnerPtr) {
#if DEBUG
	LuaStackOverflowAssert check(lua);
	lua_pushnil(lua);
	char buf[64];
	sprintf_s(buf, "%p_%s", OwnerPtr, name);
	lua_setglobal(lua, buf);
	AddLogf(Debug, "Deleting mapped global: %s by %p(%s)", buf, OwnerPtr, typeid(*OwnerPtr).name());
#endif
}

} //namespace Scripts
} //namespace Core
} //namespace MoonGlare 
