#pragma once

namespace Utils {
namespace Handle {

using HandleUint = unsigned __int32;

template <class VALUETYPE>
struct BaseSingleHandle {
	using ValueType = VALUETYPE;
	ValueType m_Value;
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS = 32 - GENERATIONBITS>
struct BaseDoubleHandle32 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		BitsSize = GenerationBits + ValueBits,
	};

	static_assert(BitsSize == 32, "BaseDoubleHandle32 has to be 32bits long!");

	union {
		struct {
			ValueType m_Value : ValueBits;
			ValueType m_Generation : GenerationBits;
		};
		ValueType m_IntegerValue;
	};
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS = 64 - GENERATIONBITS>
struct BaseDoubleHandle64 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		BitsSize = GenerationBits + ValueBits,
	};

	static_assert(BitsSize == 64, "BaseDoubleHandle32 has to be 64bits long!");

	union {
		struct {
			ValueType m_Value : ValueBits;
			ValueType m_Generation : GenerationBits;
		};
		ValueType m_IntegerValue;
	};
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS, VALUETYPE TYPEBITS = 32 - VALUEBITS - GENERATIONBITS>
struct BaseTripleHandle32 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		TypeBits = TYPEBITS,
		BitsSize = GenerationBits + ValueBits + TypeBits,
	};

	static_assert(BitsSize == 32, "BaseTripleHandle32 has to be 32bits long!");

	union {
		struct {
			ValueType m_Value : ValueBits;
			ValueType m_Type : TypeBits;
			ValueType m_Generation : GenerationBits;
		};
		ValueType m_IntegerValue;
	};
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS, VALUETYPE TYPEBITS = 64 - VALUEBITS - GENERATIONBITS>
struct BaseTripleHandle64 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		TypeBits = TYPEBITS,
		BitsSize = GenerationBits + ValueBits + TypeBits,
	};

	static_assert(BitsSize == 64, "BaseTripleHandle64 has to be 64bits long!");

	union {
		struct {
			ValueType m_Value : ValueBits;
			ValueType m_Type : TypeBits;
			ValueType m_Generation : GenerationBits;
		};
		ValueType m_IntegerValue;
	};
};

} //namespace Handle
} //namespace Utils
