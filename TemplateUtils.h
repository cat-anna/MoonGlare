#pragma once
#ifndef TEMPLATE_UTILS_H
#define TEMPLATE_UTILS_H

namespace Utils {
namespace Template {

template<class T, class OWNER>
struct ProtectedStaticValue {
protected:
	static T _value;
};
template<class T, class OWNER> T ProtectedStaticValue<T, OWNER>::_value;

template<class T, T value> T StaticReturn() {
	return value;
}

template <class T, T* ptr, T NewValue>
T StaticSet() {
	T backup = *ptr;
	*ptr = NewValue;
	return backup;
}

template <class T, T* ptr>
T DynamicSet(T NewValue) {
	T backup = *ptr;
	*ptr = NewValue;
	return backup;
}

template<class R, class T>
R inline UnionCast(T t) {
	union {
		T t;
		R r;
	} u;
	u.t = t;
	return u.r;
}

using VoidFunc = void(*)();

struct CallAllImpl {
	template < VoidFunc ... Args >
	static void call() {
		icall<0, Args...>();
	}
private:
	template < int f >
	static void icall(void) { }
	template < int f, VoidFunc t, VoidFunc ... Args>
	static void icall() { 
		t();
		icall<0, Args...>();
	}
};

template < VoidFunc ... Args >
struct CallAll {
	static void call() {
		CallAllImpl::call<Args...>();
	}
};

template<class CLASS, class T>
struct PointerFromRef {
	using fun_t = T* (CLASS::*)();

	template<T& (CLASS::*FUNC)()>
	static fun_t get() {
		return reinterpret_cast<fun_t>(&cast<FUNC>);
	}
private:
	template<T& (CLASS::*FUNC)()>
	T* cast() {
		return &(((CLASS*)this)->*FUNC)();
	}
};

template<class CLASS, class RET, RET(CLASS::*FUNC)(const string&)>
struct CastStringParameter {
	using fun_t = RET (CLASS::*)(const char *);

	static fun_t get() {
		return reinterpret_cast<fun_t>(&cast<FUNC>);
	}
private:
	template<RET(CLASS::*FUNC)(const string&)>
	RET cast(const char *ptr) {
		string s(ptr);
		return (((CLASS*)this)->*FUNC)(s);
	}
};

template<class OWNER, class SMART>
struct SmartPointerTweeks {
	using Raw_t = typename SMART::element_type;
	using RawGet_t = Raw_t* (OWNER::*)();
	using RawSet_t = void (OWNER::*)(Raw_t*);

	template<SMART OWNER::*PTR>
	static RawGet_t Get() { return reinterpret_cast<RawGet_t>(&GetImpl<PTR>); }

	template<SMART OWNER::*PTR>
	static RawSet_t Set() { return reinterpret_cast<RawSet_t>(&SetImpl<PTR>); }
	template<void (OWNER::*PTR)(Raw_t*)>
	static RawSet_t Set() { return reinterpret_cast<RawSet_t>(&SetImplFunc<PTR>); }
private:
	template<SMART OWNER::*PTR>
	Raw_t* GetImpl() { return (((OWNER*)this)->*PTR).get(); }

	template<SMART OWNER::*PTR>
	void SetImpl(Raw_t *p) { return (((OWNER*)this)->*PTR).reset(p); }
	template<void (OWNER::*PTR)(Raw_t*)>
	void SetImplFunc(Raw_t *p) { return (((OWNER*)this)->*PTR)(p); }
};

template <class OWNER, class VEC, VEC OWNER::*PTR>
struct HiddenVec3Set {
	using get_t = void(OWNER::*)(float, float, float);
	static get_t get() { return reinterpret_cast<get_t>(&get_impl); }
private:
	void get_impl(float x, float y, float z) {
		(((OWNER*)this)->*PTR) = VEC(x, y, z);
	}
};

template <class OWNER, class TYPE, void(OWNER::*FUNC)(const TYPE&), class ... Params>
struct DynamicArgumentConvert {
	using fun_t = void(OWNER::*)(Params ... args);
	static fun_t get() {
		return reinterpret_cast<fun_t>(&Impl);
	}
private:
	void Impl(Params ... args) {
		TYPE t(std::forward<Params>(args)...);
		(((OWNER*)this)->*FUNC)(t);
	}
};

template<class OWNER, class ENUM, class CONVERT, void(OWNER::*PTR)(ENUM)>
struct StringToEnum {
	using fun_t = void(OWNER::*)(const char*);
	static fun_t get() { return (fun_t)(&safe_impl); }
	static fun_t get_throw() { return reinterpret_cast<fun_t>(&throw_impl); }
private:
	void safe_impl(const char *c) { (((OWNER*)this)->*PTR)(CONVERT::ConvertSafe(c)); }
	void throw_impl(const char *c) { (((OWNER*)this)->*PTR)(CONVERT::ConvertThrow(c)); }
};

template <class OWNER, class RETURN, class ...ARGS>
struct InstancedStaticCall {
	using func_t = RETURN(OWNER::*)(ARGS...);

	template<RETURN(*FUNCTION)(ARGS...)>
	static func_t get() {
		return (func_t)&impl <FUNCTION> ;
	}

	template<RETURN(*FUNCTION)(OWNER*, ARGS...)>
	static func_t callee() {
		return (func_t)&impl_callee <FUNCTION>;
	}
private:
	template <RETURN(*FUNCTION)(ARGS...)>
	RETURN impl(ARGS ... args) {
	return FUNCTION(std::forward<ARGS>(args)...);
	}
	template <RETURN(*FUNCTION)(OWNER*, ARGS...)>
	RETURN impl_callee(ARGS ... args) {
		return FUNCTION(reinterpret_cast<OWNER*>(this), std::forward<ARGS>(args)...);
	}
};

} //namespace Template
} //namespace Utils

#endif // CSCRIPTEVENTS_H_ 
