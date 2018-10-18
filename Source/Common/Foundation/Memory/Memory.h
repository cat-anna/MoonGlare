#pragma once

namespace Utils {
namespace Memory {

template<class CHAR, size_t SIZE, class LOCKPOLICY = DefaultLockPolicy >
struct StaticStringPool {
    using Char_t = CHAR;
	using LockPolicy = LOCKPOLICY;
    enum {
        Size = SIZE,
    };
    
    size_t RemainSize() const { return Size - m_UsedSize; }
    size_t RemainBytes() const { return RemainSize() * sizeof(Char_t); }
    size_t UsedSize() const { return m_UsedSize; }
    size_t UsedBytes() const { return UsedSize() * sizeof(Char_t); }
    void Clear() { m_UsedSize = 0; }
    
    StaticStringPool(): m_UsedSize(0) { }
    
    /** allocs space for len chars*/
    Char_t *Allocate(size_t len) {
		++len;
		Char_t *ptr;
		{
			LockPolicy::Guard_t lock(m_Lock);
			if(len > RemainSize())
				return nullptr;
			ptr = m_Pool + m_UsedSize;
			m_UsedSize += len;
		}
		ptr[len] = static_cast<Char_t>(0);
		ptr[len - 1] = static_cast<Char_t>(0);
        return ptr;
    }
    
    Char_t *Allocate(const Char_t* str) {
        size_t len = 0;
        if(!str) 
            return Allocate((size_t)0);
        while(str[len])
            ++len;
        return Allocate(str, len);
    }
     
    Char_t *Allocate(const Char_t* str, size_t len) {
        auto *ptr = Allocate(len);
        if(ptr) 
            memcpy(ptr, str, sizeof(Char_t) * len);        
        return ptr;
    }   
private:
	LockPolicy m_Lock;
    size_t m_UsedSize;
    Char_t m_Pool[Size];
};

//----------------------------------------------------------------

template<class T, size_t SIZE, class LOCKPOLICY = DefaultLockPolicy >
struct StaticMemoryPool {
	static_assert(std::is_pod<T>::value, "StaticMemoryPool accepts only POD types"); 

	using Item = T;
	using LockPolicy = LOCKPOLICY;
	enum {
		Size = SIZE,
	};

	template<class Iter_T>
	struct BaseIterator {
		using Item = Iter_T;
		using ThisClass = BaseIterator<Iter_T>;
		ThisClass(Item *ptr): m_Ptr(ptr) { }
		Item& operator*() { return *m_Ptr; }
		std::add_const_t<Item>& operator*() const { return *m_Ptr; }
		ThisClass operator++(int) { return ThisClass(m_Ptr + 1); }
		ThisClass& operator++() { ++m_Ptr; return *this; }
		ThisClass operator--(int) { return ThisClass(m_Ptr - 1); }
		ThisClass& operator--() { ++m_Ptr; return *this; }
		bool operator!=(const ThisClass& other) const { return m_Ptr != other.m_Ptr; }
		bool operator>(const ThisClass& other) const { return m_Ptr > other.m_Ptr; }
		bool operator<(const ThisClass& other) const { return m_Ptr < other.m_Ptr; }
	private:
		Item *m_Ptr;
	};

	using iterator = BaseIterator<T>;
	using const_iterator = BaseIterator<const T>;

	StaticMemoryPool(): m_UsedSize(0) { }
    
    size_t RemainSize() const { return Size - m_UsedSize; }
    size_t RemainBytes() const { return RemainSize() * sizeof(Char_t); }
    size_t UsedSize() const { return m_UsedSize; }
    size_t UsedBytes() const { return UsedSize() * sizeof(Char_t); }
    void Clear() { m_UsedSize = 0; }
	bool Empty() const { return m_UsedSize == 0; }

	/** Allocate single element */
	T* Allocate() { return Allocate(1); }
	/** Allocate single element and memset to zero */
	T* AllocateAndZero() { return AllocateAndZero(1); }
	/** Allocate multilple elements  */
	T* Allocate(size_t len) {
		if (RemainSize() < len)
			return nullptr;
		LockPolicy::Guard_t lock(m_Lock);
		T* ptr = m_Memory + m_UsedSize;
		m_UsedSize += len;
		return ptr;
	}
	/** Allocate multilple elements and memset to zero */
	T* AllocateAndZero(size_t len) {
		auto ptr = Allocate(len);
		if (ptr)
			memset(ptr, 0, sizeof(T) * len);
		return ptr;
	}

	iterator begin() { return iterator(m_Memory); }
	const_iterator begin() const { return const_iterator(m_Memory); }
	iterator end() { return iterator(m_Memory + m_UsedSize); }
	const_iterator end() const { return const_iterator(m_Memory + m_UsedSize); }
private:
	LockPolicy m_Lock;
    size_t m_UsedSize;
	T m_Memory[Size];
};

}
}
