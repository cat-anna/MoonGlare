#pragma once

namespace Utils {

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


} //namespace Utils

#define FlagBit(FLAG) \
	(1 << static_cast<Flag_t>(FLAG))

#define DefineFlagInternals(MEMBER, FLAGBIT, NAME) \
	using Flag_t = decltype(MEMBER); \
    const Flag_t FlagValue = static_cast<Flag_t>(FLAGBIT);

#define DefineFlagGetter(MEMBER, FLAGBIT, NAME) \
	bool Is##NAME() const { DefineFlagInternals(MEMBER, FLAGBIT, NAME); return ::Utils::TestFlags(MEMBER, FlagValue); }
	
#define DefineFlagSetter(MEMBER, FLAGBIT, NAME) \
	void Set##NAME(bool Value) { DefineFlagInternals(MEMBER, FLAGBIT, NAME); return ::Utils::SetFlags(MEMBER, FlagValue, Value); }

#define DefineFlag(MEMBER, FLAGBIT, NAME) \
	DefineFlagGetter(MEMBER, FLAGBIT, NAME) \
	DefineFlagSetter(MEMBER, FLAGBIT, NAME)
