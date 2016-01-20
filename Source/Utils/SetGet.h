#pragma once

namespace Utils {

using FlagSet = unsigned __int32;

} //namespace Utils

#define DefineFlagInternals(FLAGS, FLAG, NAME) using Flag_t = std::remove_cv<decltype(FLAGS)>::type; const Flag_t NAME = static_cast<Flag_t>(FLAG);
#define DefineFlagGetter(FLAGS, FLAG, NAME) bool Is##NAME() const { DefineFlagInternals(FLAGS, FLAG, F); return TestFlags(FLAGS, F); }
#define DefineFlagSetter(FLAGS, FLAG, NAME) void Set##NAME(bool Value) { DefineFlagInternals(FLAGS, FLAG, F); SetFlags(FLAGS, F, Value); }
#define DefineFlag(FLAGS, FLAG, NAME) DefineFlagGetter(FLAGS, FLAG, NAME) DefineFlagSetter(FLAGS, FLAG, NAME)
#define FlagBit(FLAG) (1 << static_cast<Flag_t>(FLAG))

#define DefineRefGetterConst(NAME, TYPE) const TYPE& Get##NAME() const { return m_##NAME; }
#define DefineRefGetter(NAME, TYPE) TYPE& Get##NAME() { return m_##NAME; }
#define DefineRefGetterAll(NAME, TYPE) DefineRefGetterConst(NAME, TYPE)  DefineRefGetter(NAME, TYPE) 

#define DefineDirectGetterConst(NAME, TYPE) const TYPE Get##NAME() const { return m_##NAME; }
#define DefineDirectGetter(NAME, TYPE) TYPE Get##NAME() { return m_##NAME; }
#define DefineDirectGetterAll(NAME, TYPE) DefineDirectGetterConst(NAME, TYPE)  DefineDirectGetter(NAME, TYPE) 

#define DefineDirectSetGet(NAME, TYPE) TYPE Get##NAME() const { return m_##NAME; } void Set##NAME(TYPE t) { m_##NAME = t; }
#define DefineSetGetByRef(NAME, TYPE) DefineRefGetterAll(NAME, TYPE) void Set##NAME(const TYPE &v) { m_##NAME = v; }
#define DefineRefSetGet(NAME, TYPE) DefineRefGetterAll(NAME, TYPE) void Set##NAME(const TYPE &v) { m_##NAME = v; }

//deprecated 
#define DefineREADAcces(NAME, TYPE) const TYPE &Get##NAME() const { return m_##NAME; }
#define DefineRWAcces(NAME, TYPE) TYPE &Get##NAME() { return m_##NAME; } DefineREADAcces(NAME, TYPE)
#define DefineWRITEAcces(NAME, TYPE) const TYPE &Set##NAME(const TYPE &val) { m_##NAME = val; return val; }
#define DefineFULLAcces(NAME, TYPE)	DefineRWAcces(NAME, TYPE) DefineWRITEAcces(NAME, TYPE)

#define DefineREADAccesPTR(NAME, TYPE) const TYPE* Get##NAME() const { return m_##NAME; } TYPE* Get##NAME() { return m_##NAME; }
#define DefineWRITEAccesPTR(NAME, TYPE) TYPE* Set##NAME(TYPE *val) { m_##NAME = val; return val; }
#define DefineFULLAccesPTR(NAME, TYPE)	DefineREADAccesPTR(NAME, TYPE) DefineWRITEAccesPTR(NAME, TYPE)

bool inline TestFlags(unsigned src, unsigned flags) {
	return (src & flags) == flags;
}

void inline SetFlags(unsigned &src, unsigned flags, bool set) {
	if (set) src |= flags;
	else src &= ~flags;
}

void inline SetFlags(volatile unsigned &src, unsigned flags, bool set) {
	if (set) src |= flags;
	else src &= ~flags;
}

template<class BASE>
bool inline TestFlags(BASE src, BASE flags) {
	static_assert(std::is_integral<BASE>::value, "BASE type must be an integral type!");
	return (src & flags) == flags;
}

template<class BASE>
void inline SetFlags(BASE &src, BASE flags, bool set) {
	static_assert(std::is_integral<BASE>::value, "BASE type must be an integral type!");
	if (set) src |= flags;
	else src &= ~flags;
}

