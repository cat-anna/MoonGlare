#pragma once

namespace MoonGlare::Memory {

template<class ELEMENT, size_t SIZE, typename IndexType = uint32_t>
struct StaticAllocationPool final {
	using Index_t = IndexType;
	using ArrayType = std::array<ELEMENT, SIZE>;
	using value_type = ELEMENT;

	StaticAllocationPool(): m_Allocated(0), m_Valid(0) { }

	IndexType Allocated() const { return m_Allocated; }
	IndexType ValidElements() const { return m_Valid; }
	constexpr IndexType Capacity() const { return m_Array.size(); }
	IndexType RemainStorage() const { return Capacity() - ValidElements(); }
	bool HasStorage() const { return RemainStorage() > 0; }
	void ClearAllocation() { m_Allocated = 0; }
	void MemZero() { memset(this, 0, sizeof(*this)); }
	bool Empty() const { return m_Allocated == 0; }

	template<typename T>
	void fill(T && t) { return m_Array.fill(std::forward<T>(t)); }

	bool PushAndAlloc(value_type &pushitem, value_type &allocitem) {
		if (m_Valid >= Capacity())
			return false;
		auto pushidx = m_Valid++;
		auto allocidx = m_Allocated++;
		m_Array[pushidx] = pushitem;
		allocitem = m_Array[allocidx];
		return true;
	}

	bool Allocate(value_type & item) {
		if (m_Allocated >= m_Valid)
			return false;
		auto next = m_Allocated++;
		//got index;
		item = m_Array[next];
		return true;
	}
protected:
	Index_t m_Allocated;
	Index_t m_Valid;
	ArrayType m_Array;
};

}
