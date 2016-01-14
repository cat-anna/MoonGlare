#pragma once

namespace Utils {
namespace Handle {

using HandleUint = unsigned __int32;

template <class VALUETYPE>
struct BaseSingleHandle {
	using ValueType = VALUETYPE;
	ValueType m_Value;
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS>
struct BaseDoubleHandle32 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		BitsSize = GenerationBits + ValueBits,
	};

	static_assert(BitsSize == 32, "BaseDoubleHandle32 has to be 32bits long!");

	ValueType m_Value : ValueBits;
	ValueType m_Generation : GenerationBits;
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS>
struct BaseDoubleHandle64 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		BitsSize = GenerationBits + ValueBits,
	};

	static_assert(BitsSize == 64, "BaseDoubleHandle32 has to be 64bits long!");

	ValueType m_Value : ValueBits;
	ValueType m_Generation : GenerationBits;
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS, VALUETYPE TYPEBITS>
struct BaseTripleHandle32 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		TypeBits = TYPEBITS
		BitsSize = GenerationBits + ValueBits + TypeBits,
	};

	static_assert(BitsSize == 32, "BaseTripleHandle32 has to be 32bits long!");

	ValueType m_Value : ValueBits;
	ValueType m_Type : TypeBits;
	ValueType m_Generation : GenerationBits;
};

template<class VALUETYPE, VALUETYPE GENERATIONBITS, VALUETYPE VALUEBITS, VALUETYPE TYPEBITS>
struct BaseTripleHandle64 {
	using ValueType = VALUETYPE;
	enum {
		GenerationBits = GENERATIONBITS,
		ValueBits = VALUEBITS,
		TypeBits = TYPEBITS
		BitsSize = GenerationBits + ValueBits + TypeBits,
	};

	static_assert(BitsSize == 64, "BaseTripleHandle64 has to be 64bits long!");

	ValueType m_Value : ValueBits;
	ValueType m_Type : TypeBits;
	ValueType m_Generation : GenerationBits;
};
} //namespace Handle
} //namespace Utils