#pragma once

namespace Utils {

struct VirtualDestructor {
	virtual ~VirtualDestructor() {}
};

//using FlagSet = uint32_t;

template<class BASE, class FLAGS>
bool inline TestFlags(BASE src, FLAGS flags) {
	return (src & flags) == flags;
}

template<class BASE, class FLAGS>
void inline SetFlags(BASE &src, FLAGS flags, bool set) {
	if (set) src |= flags;
	else src &= ~flags;
}

template<class T>
struct BasicFlagSet {
	using ValueType = T;
	using ThisClass = BasicFlagSet<T>;

	BasicFlagSet(): m_Value(0) {}
	BasicFlagSet(ValueType v): m_Value(v) {}

	ValueType operator &(ValueType flags) { return m_Value & flags; }
	ThisClass& operator |=(ValueType flags) { m_Value |= flags; return *this; }
	ThisClass& operator &=(ValueType flags) { m_Value &= flags; return *this; }

	operator ValueType () const { return m_Value; }
	ValueType Raw() const { return m_Value; }
private:
	T m_Value;
};

using FlagSet = BasicFlagSet<uint32_t>;

template<class T>
struct FlagsValueType {
	using ValueType = T;
};

template<>
struct FlagsValueType<FlagSet> {
	using ValueType = FlagSet::ValueType;
};

} //namespace Utils

#define FlagBit(FLAG) \
	(1 << static_cast<Flag_t>(FLAG))

#define DefineFlagInternals(FLAGS, FLAG, NAME) \
	using BaseFlag_t = std::remove_cv<decltype(FLAGS)>::type; \
	using Flag_t = ::Utils::FlagsValueType<BaseFlag_t>::ValueType; \
	const Flag_t NAME = static_cast<Flag_t>(FLAG);

#define DefineFlagGetter(FLAGS, FLAG, NAME) \
	bool Is##NAME() const { \
		DefineFlagInternals(FLAGS, FLAG, FLAGVALUE); \
		return ((FLAGS) & (FLAGVALUE)) == (FLAGVALUE); \
	}
	
#define DefineFlagSetter(FLAGS, FLAG, NAME) \
	void Set##NAME(bool Value) { \
		DefineFlagInternals(FLAGS, FLAG, FLAGVALUE); \
		if (Value) FLAGS |= (FLAGVALUE); \
		else FLAGS &= ~(FLAGVALUE); \
	}

#define DefineFlag(FLAGS, FLAG, NAME) \
	DefineFlagGetter(FLAGS, FLAG, NAME) \
	DefineFlagSetter(FLAGS, FLAG, NAME)

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

