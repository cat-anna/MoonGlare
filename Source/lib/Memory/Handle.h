#pragma once

namespace MoonGlare::Memory {

template <class VALUETYPE>
struct BaseSingleHandle {
	using ValueType = VALUETYPE;
	ValueType m_Index;
};

template<size_t BITS>
struct IntegerSizeSelector {

	using upto64selector = typename std::conditional<(BITS <= 64ul), uint64_t, void>::type;
	using upto32selector = typename std::conditional<(BITS <= 32ul), uint32_t, upto64selector>::type;
	using upto16selector = typename std::conditional<(BITS <= 16ul), uint16_t, upto32selector>::type;
	using upto8selector  = typename std::conditional<(BITS <=  8ul), uint8_t, upto16selector>::type;
	
	using type = upto8selector;
};

template<size_t GENERATIONBITS, size_t INDEXBITS = 0, size_t TYPEBITS = 0, size_t VALUEBITS = 0>
struct HandleValueTypeSelector {
	using IntValue_t = typename IntegerSizeSelector<GENERATIONBITS + INDEXBITS + TYPEBITS + VALUEBITS>::type;
	using Index_t = IntValue_t;
	using Generation_t = IntValue_t;
	using Type_t = IntValue_t;
	using Value_t = IntValue_t;
	
	static_assert(sizeof(Index_t) * 8 >= INDEXBITS, "Invalid Index_t size!");
	static_assert(sizeof(Generation_t) * 8 >= GENERATIONBITS, "Invalid Generation_t size!");
	static_assert(sizeof(Type_t) * 8 >= TYPEBITS, "Invalid Type_t size!");
	static_assert(sizeof(Value_t) * 8 >= VALUEBITS, "Invalid Value_t size!");
};

//-----------------------------------------------------------------------------

template<size_t GENERATIONBITS, size_t INDEXBITS>
struct AbstractDoubleHandle : public HandleValueTypeSelector<GENERATIONBITS, INDEXBITS> {
	using BaseClass = HandleValueTypeSelector<GENERATIONBITS, INDEXBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	using Index_t = typename BaseClass::Index_t;
	using Generation_t = typename BaseClass::Generation_t;
	
	enum {
		IndexBits = INDEXBITS,
		GenerationBits = GENERATIONBITS,
		BitSize = IndexBits + GenerationBits,
	};

	union {
		struct {
			Index_t m_Index : IndexBits;
			Generation_t m_Generation : GenerationBits;
		};
		IntValue_t m_IntegerValue;
	};

	Index_t GetIndex() const { return m_Index; }
	Generation_t GetGeneration() const { return m_Generation; }
	void SetIndex(Index_t value) { m_Index = value; }
	void SetGeneration(Generation_t value) { m_Generation = value; }
	IntValue_t GetIntValue() const { return m_IntegerValue; }
	void* GetVoidPtr() const { return reinterpret_cast<void*>(m_IntegerValue); }

	void Swap(AbstractDoubleHandle & other) {
		std::swap(m_IntegerValue, other.m_IntegerValue);
	}
};

template<size_t GENERATIONBITS, size_t INDEXBITS>
inline std::ostream& operator << (std::ostream &o, AbstractDoubleHandle<GENERATIONBITS, INDEXBITS> h) {
	char b[256];
	sprintf(b, "(%llu; %llu)", (uint64_t)h.GetIndex(), (uint64_t)h.GetGeneration());
	return (o << b);
}

template<size_t GENERATIONBITS, size_t INDEXBITS = 32 - GENERATIONBITS>
struct DoubleHandle32 : public AbstractDoubleHandle<GENERATIONBITS, INDEXBITS> {
	using BaseClass = AbstractDoubleHandle<GENERATIONBITS, INDEXBITS>;
	using ThisClass = DoubleHandle32<GENERATIONBITS, INDEXBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	
	static_assert(BaseClass::BitSize == 32, "DoubleHandle32 has to be 32 bit long!");
	static ThisClass FromIntValue(IntValue_t value) { ThisClass h; h.m_IntegerValue = value; return h; }
	static ThisClass FromVoidPtr(void* value) { ThisClass h; h.m_IntegerValue = reinterpret_cast<IntValue_t>(value); return h; }

    bool operator == (ThisClass other) const { return m_IntegerValue == other.m_IntegerValue; }
    bool operator != (ThisClass other) const { return m_IntegerValue != other.m_IntegerValue; }
};
static_assert(sizeof(DoubleHandle32<10>) == 4);

template<size_t GENERATIONBITS, size_t INDEXBITS = 64 - GENERATIONBITS>
struct DoubleHandle64 : public AbstractDoubleHandle<GENERATIONBITS, INDEXBITS> {
	using BaseClass = AbstractDoubleHandle<GENERATIONBITS, INDEXBITS>;
	using ThisClass = DoubleHandle64<GENERATIONBITS, INDEXBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	
	static_assert(BaseClass::BitSize == 64, "DoubleHandle64 has to be 64 bit long!");
	static_assert(sizeof(ThisClass) == 8, "Invalid handle size!");
	static ThisClass FromIntValue(IntValue_t value) { ThisClass h; h.m_IntegerValue = value; return h; }
	static ThisClass FromVoidPtr(void* value) { ThisClass h; h.m_IntegerValue = reinterpret_cast<IntValue_t>(value); return h; }

	bool operator == (ThisClass other) const { return m_IntegerValue == other.m_IntegerValue; }
    bool operator != (ThisClass other) const { return m_IntegerValue != other.m_IntegerValue; }
};

//-----------------------------------------------------------------------------

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS>
struct AbstractTripleHandle : public HandleValueTypeSelector<GENERATIONBITS, INDEXBITS, TYPEBITS> {
	using BaseClass = HandleValueTypeSelector<GENERATIONBITS, INDEXBITS, TYPEBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	using Index_t = typename BaseClass::Index_t;	
	using Generation_t = typename BaseClass::Generation_t;
	using Type_t = typename BaseClass::Type_t;
	
	enum {
		IndexBits = INDEXBITS,
		GenerationBits = GENERATIONBITS,
		TypeBits = TYPEBITS,
		BitSize = IndexBits + GenerationBits + TypeBits,
	};

	union {
		struct {
			Index_t m_Index : IndexBits;
			Generation_t m_Generation : GenerationBits;
			Type_t m_Type : TypeBits;
		};
		IntValue_t m_IntegerValue;
	};

	Index_t GetIndex() const { return m_Index; }
	Generation_t GetGeneration() const { return m_Generation; }
	Type_t GetType() const { return m_Type; }
	void SetIndex(Index_t value) { m_Index = value; }
	void SetGeneration(Generation_t value) { m_Generation = value; }
	void SetType(Type_t value) { m_Type = value; }
	IntValue_t GetIntValue() const { return m_IntegerValue; }
	void* GetVoidPtr() const { return reinterpret_cast<void*>(m_IntegerValue); }

	void Swap(AbstractTripleHandle & other) {
		std::swap(m_IntegerValue, other.m_IntegerValue);
	}
};

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS>
inline std::ostream& operator << (std::ostream &o, AbstractTripleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS> h) {
	char b[256];
	sprintf(b, "(%llu; %llu; %llu)", (uint64_t)h.GetIndex(), (uint64_t)h.GetGeneration(), (uint64_t)h.GetType());
	return (o << b);
}

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS = 32 - (INDEXBITS + GENERATIONBITS)>
struct TripleHandle32 : public AbstractTripleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS> {
	using BaseClass = AbstractTripleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS>;
	using ThisClass = TripleHandle32<GENERATIONBITS, INDEXBITS, TYPEBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	
	static_assert(BaseClass::BitSize == 32, "TripleHandle32 has to be 32 bit long!");
	static_assert(sizeof(BaseClass) == 4, "Invalid handle size!");
	static ThisClass FromIntValue(IntValue_t value) { ThisClass h; h.m_IntegerValue = value; return h; }
	static ThisClass FromVoidPtr(void* value) { ThisClass h; h.m_IntegerValue = reinterpret_cast<IntValue_t>(value); return h; }

	bool operator == (ThisClass other) const { return m_IntegerValue == other.m_IntegerValue; }
    bool operator != (ThisClass other) const { return m_IntegerValue != other.m_IntegerValue; }
};

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS = 64 - (INDEXBITS + GENERATIONBITS)>
struct TripleHandle64 : public AbstractTripleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS> {
	using BaseClass = AbstractTripleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS>;
	using ThisClass = TripleHandle64<GENERATIONBITS, INDEXBITS, TYPEBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	
	static_assert(BaseClass::BitSize == 64, "TripleHandle64 has to be 64 bit long!");
	static_assert(sizeof(BaseClass) == 8, "Invalid handle size!");
	static ThisClass FromIntValue(IntValue_t value) { ThisClass h; h.m_IntegerValue = value; return h; }
	static ThisClass FromVoidPtr(void* value) { ThisClass h; h.m_IntegerValue = reinterpret_cast<IntValue_t>(value); return h; }

	bool operator == (ThisClass other) { return m_IntegerValue == other.m_IntegerValue; }
    bool operator != (ThisClass other) { return m_IntegerValue != other.m_IntegerValue; }
};

//-----------------------------------------------------------------------------

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS, size_t VALUEBITS>
struct AbstractQuadrupleHandle : public HandleValueTypeSelector<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS> {
	using BaseClass = HandleValueTypeSelector<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;
	using Index_t = typename BaseClass::Index_t;
	using Generation_t = typename BaseClass::Generation_t;
	using Type_t = typename BaseClass::Type_t;
	using Value_t = typename BaseClass::Value_t;

	enum {
		IndexBits = INDEXBITS,
		GenerationBits = GENERATIONBITS,
		TypeBits = TYPEBITS,
		ValueBits = VALUEBITS,
		BitSize = IndexBits + GenerationBits + TypeBits + ValueBits,
	};

	union {
		struct {
			Index_t m_Index : IndexBits;
			Generation_t m_Generation : GenerationBits;
			Type_t m_Type : TypeBits;
			Value_t m_Value : ValueBits;
		};
		IntValue_t m_IntegerValue;
	};

	Index_t GetIndex() const { return m_Index; }
	void SetIndex(Index_t value) { m_Index = value; }
	Generation_t GetGeneration() const { return m_Generation; }
	void SetGeneration(Generation_t value) { m_Generation = value; }
	Type_t GetType() const { return m_Type; }
	void SetType(Type_t value) { m_Type = value; }
	Value_t GetValue() const { return m_Value; }
	void SetValue(Value_t value) { m_Value = value; }
	IntValue_t GetIntValue() const { return m_IntegerValue; }
	void* GetVoidPtr() const { return reinterpret_cast<void*>(m_IntegerValue); }

	void Swap(AbstractQuadrupleHandle & other) {
		std::swap(m_IntegerValue, other.m_IntegerValue);
	}
};

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS, size_t VALUEBITS>
inline std::ostream& operator << (std::ostream &o, AbstractQuadrupleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS> h) {
	char b[256];
	sprintf(b, "(%llu; %llu; %llu; %llu)", (uint64_t)h.GetIndex(), (uint64_t)h.GetGeneration(), (uint64_t)h.GetType(), (uint64_t)h.GetValue());
	return (o << b);
}

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS, size_t VALUEBITS = 32 - (INDEXBITS + GENERATIONBITS + TYPEBITS)>
struct QuadrupleHandle32 : public AbstractQuadrupleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS> {
	using ThisClass = QuadrupleHandle32<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS>;
	using BaseClass = AbstractQuadrupleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;

	static_assert(BaseClass::BitSize == 32, "QuadrupleHandle32 has to be 32 bit long!");
	static_assert(sizeof(BaseClass) == 4, "Invalid handle size!");
	static ThisClass FromIntValue(IntValue_t value) { ThisClass h; h.m_IntegerValue = value; return h; }
	static ThisClass FromVoidPtr(void* value) { ThisClass h; h.m_IntegerValue = reinterpret_cast<IntValue_t>(value); return h; }

	bool operator == (ThisClass other) const { return m_IntegerValue == other.m_IntegerValue; }
    bool operator != (ThisClass other) const { return m_IntegerValue != other.m_IntegerValue; }
};

template<size_t GENERATIONBITS, size_t INDEXBITS, size_t TYPEBITS, size_t VALUEBITS = 64 - (INDEXBITS + GENERATIONBITS + TYPEBITS)>
struct QuadrupleHandle64 : public AbstractQuadrupleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS> {
	using ThisClass = QuadrupleHandle64<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS>;
	using BaseClass = AbstractQuadrupleHandle<GENERATIONBITS, INDEXBITS, TYPEBITS, VALUEBITS>;
	using IntValue_t = typename BaseClass::IntValue_t;

	static_assert(BaseClass::BitSize == 64, "QuadrupleHandle64 has to be 64 bit long!");
	static_assert(sizeof(BaseClass) == 8, "Invalid handle size!");
	static ThisClass FromIntValue(IntValue_t value) { ThisClass h; h.m_IntegerValue = value; return h; }
	static ThisClass FromVoidPtr(void* value) { ThisClass h; h.m_IntegerValue = reinterpret_cast<IntValue_t>(value); return h; }

	bool operator == (ThisClass other) const { return m_IntegerValue == other.m_IntegerValue; }
    bool operator != (ThisClass other) const { return m_IntegerValue != other.m_IntegerValue; }
};

}
