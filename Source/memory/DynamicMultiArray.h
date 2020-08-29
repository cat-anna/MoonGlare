#pragma once

namespace MoonGlare::Memory {

template<size_t ElementCount, class PointerTuple>
struct PointerTupleHelper {
	static void Init(char *Memory, PointerTuple &Tuple, size_t ArraySize) { InitElement<0>(Memory, Tuple, ArraySize); }
	static void MemCopy(PointerTuple &Source, PointerTuple &Dest, size_t SrcSize, size_t DestSize) { ElementCopy<0>(Source, Dest, SrcSize, DestSize); }
private:
	template<size_t Current>
	static void InitElement(char *Memory, PointerTuple &Tuple, size_t ArraySize) {
		using ElementPointer = std::tuple_element<Current, PointerTuple>::type;
		using RawElement = std::remove_pointer<ElementPointer>::type;
		
		std::get<Current>(Tuple) = reinterpret_cast<ElementPointer>(Memory);
		Memory += ArraySize * sizeof(RawElement);
		InitElement<Current + 1>(Memory, Tuple, ArraySize);
	}

	template<>
	static void InitElement<ElementCount>(char *Memory, PointerTuple &Tuple, size_t ArraySize) {}

	template<size_t Current>
	static void ElementCopy(PointerTuple &Source, PointerTuple &Dest, size_t SrcSize, size_t DestSize) {
		using ElementPointer = std::tuple_element<Current, PointerTuple>::type;
		using RawElement = std::remove_pointer<ElementPointer>::type;

		memcpy(std::get<Current>(Source), std::get<Current>(Dest), (SrcSize < DestSize ? SrcSize : DestSize) * sizeof(RawElement));
		if (SrcSize < DestSize) {
			//clear remaining part of memory
			memset(((char*)std::get<Current>(Dest)) + SrcSize * sizeof(RawElement), 0, ( DestSize - SrcSize) * sizeof(RawElement));
		}
		ElementCopy<Current + 1>(Source, Dest, SrcSize, DestSize);
	}

	template<>
	static void ElementCopy<ElementCount>(PointerTuple &Source, PointerTuple &Dest, size_t SrcSize, size_t DestSize) {}
};

/** Memory is zeroed automatically */
template <class ...ARGS>
struct DynamicMultiArray {

	using index_t = size_t;
	using ThisClass = DynamicMultiArray<ARGS...>;

	template< class T > using add_pointer_t = typename std::add_pointer<T>::type;
	template< class T > using add_reference_t = typename std::add_lvalue_reference<T>::type;

	using ElementTuple = std::tuple<ARGS...>;
	using ReferenceTuple = std::tuple<add_reference_t<ARGS>...>;
	using PointerTuple = std::tuple<add_pointer_t<ARGS>...>;

	enum {
		ArraysPerElement = sizeof...(ARGS),
		ElementSize = sizeof(ElementTuple),
	};

	using TupleHelper = PointerTupleHelper<ArraysPerElement, PointerTuple>;

	template<size_t index, class TUPLE>
	using TupleElement = typename std::tuple_element<index, TUPLE>::type;

	template<size_t index>
	using ElementType = TupleElement<index, ElementTuple>;

	DynamicMultiArray() : m_Size(0), m_Memory(nullptr) { }
	DynamicMultiArray(size_t InitialCapacity) : m_Size(0), m_Memory(nullptr) {
		SetSize(InitialCapacity);
	}

	~DynamicMultiArray() { }
	DynamicMultiArray(const ThisClass&) = delete;
	DynamicMultiArray(ThisClass&&) = delete;
	DynamicMultiArray& operator=(const ThisClass&) = delete;

	size_t GetSize() const { return m_Size; }
	void SetSize(size_t NewSize) {
		char *NewMemory = new char[NewSize * ElementSize];
		if (m_Memory) {//TODO: handle NewSize == 0 properly!s
			PointerTuple NewArrays;
			TupleHelper::Init(NewMemory, NewArrays, NewSize);
			TupleHelper::MemCopy(m_Arrays, NewArrays, m_Size * ElementSize, NewSize);
			m_Arrays = NewArrays;
		} else {
			memset(NewMemory, 0, NewSize * ElementSize);
			TupleHelper::Init(NewMemory, m_Arrays, NewSize);
		}
		m_Memory.reset(NewMemory);
		m_Size = NewSize;
	}

	template <size_t index>
	typename TupleElement<index, ReferenceTuple> Get(size_t itemid) {
		return std::get<index>(m_Arrays)[itemid];
	}
private:
	size_t m_Size;
	PointerTuple m_Arrays;
	std::unique_ptr<char[]> m_Memory;
};

} //namespace Memory
