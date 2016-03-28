#pragma once

namespace Utils {
namespace Memory {

template<class T, size_t SIZE, class LOCKPOLICY = DefaultLockPolicy>
struct StaticIndexQueue : protected LOCKPOLICY {
	using Item = T;
	using LockPolicy = LOCKPOLICY;
	enum {
		Size = SIZE,
	};

	static_assert(std::is_integral<Item>::value, "Item type for StaticIndexQueue must be an integer type");

	StaticIndexQueue(Item First = 0, const char* Name = nullptr): m_Get(0), m_Count(Size) {
		for (Item i = 0; i < (Item)Size; ++i)
			m_Memory[i] = i + First;
	}
	~StaticIndexQueue() { }

	bool empty() const { return m_Count == 0; }
	size_t count() const { return m_Count; }

	bool get(Item &itm) {
		if (empty())
			return false;
		LockPolicy::Guard_t lock(*this);
		itm = m_Memory[m_Get];
		++m_Get;
		--m_Count;
		if (m_Get >= Size)
			m_Get = 0;
		return true;
	}

	bool push(Item itm) {
		LockPolicy::Guard_t lock(*this);
		if (m_Count >= Size)
			return false;
		m_Memory[(m_Get + m_Count) % Size] = itm;
		++m_Count;
		return true;
	}
private:
	size_t m_Get, m_Count;
	T m_Memory[Size];
};

} //namespace Memory
} //namespace Utils
