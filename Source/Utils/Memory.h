#pragma once

namespace Utils {
namespace Memory {

//----------------------------------------------------------------
//marking
//----------------------------------------------------------------
//tracking
//----------------------------------------------------------------
//allocators
//----------------------------------------------------------------

struct MemoryManagerBase {
#ifdef DEBUG_MEMORY
	struct MMinfo {
		size_t UsedSize, Size;
		size_t ElementSize;
		const char *Name;
		const char *Type;
	};
	virtual MMinfo GetInfo() const { return MMinfo(); };
#endif
};

//----------------------------------------------------------------

template<size_t SizeValue, class ITEMBASE, class LOCKPOLICY = DefaultLockPolicy>
struct DynamicCyclicBuffer : public MemoryManagerBase {
	using BaseItem = ITEMBASE;
	using LockPolicy = LOCKPOLICY;

	enum {
		Size = SizeValue,
	};

	DynamicCyclicBuffer(const char* Name = nullptr): m_Put(0), m_Get(0) {
		Check<BaseItem>();
	}
	~DynamicCyclicBuffer() {
	}

	struct Entry {
		union {
			struct {
				unsigned __int16 Valid : 1;
				unsigned __int16 Skipped : 1;
				unsigned __int16 Size : 14;
			};
			unsigned __int16 u32value;
		};
	};

	bool empty() const { return !m_Get && m_Put == m_Get; }
	BaseItem* front() {
		if (empty()) 
			return nullptr;
		LockPolicy::Guard_t lock(m_Lock);
		Entry *itm = (Entry*)(m_Memory + m_Get);
		if (itm->Skipped) {
			SkipItem(itm);
			return front();
		}
		if (!itm->Valid)
			return nullptr;
		return (BaseItem*)(itm + 1);
	}

	void pop() {
		if (empty())
			return;
		LockPolicy::Guard_t lock(m_Lock);
		if (Size - m_Get < sizeof(Entry))
			m_Get = 0;
		Entry *itm = (Entry*)(m_Memory + m_Get);
		if (itm->Skipped) {
			SkipItem(itm);
			return pop();
		}
		if (!itm->Valid)
			return;
		m_Get += itm->Size + sizeof(Entry);
		itm->Valid = 0;
		if (m_Get == m_Put)
			m_Get = m_Put = 0;
	}

	template<class T>
	bool push(T &t) {
		Check<T>();
		LockPolicy::Guard_t lock(m_Lock);
		do {
			if (!m_Get && !m_Put)
				break;
			if (m_Get < m_Put) {
				if (Size - m_Put < sizeof(T)) {
					if (m_Get < sizeof(T)) {
						//from the begining there is no space alose
						return false;//allocation failed;
					}
					if (Size - m_Get >= sizeof(Entry)) {
						auto item = (Entry*)(m_Memory + m_Put);
						item->Valid = 0;
						item->Skipped = 1;
						item->Size = Size - m_Get - sizeof(Entry);
					}
					m_Put = 0;
				}
				break;
			} 
			if (m_Get - m_Put < sizeof(T)) {
				return false;//allocation failed;
			}
			break;
		} while (false);
		auto item = (Entry*)(m_Memory + m_Put);
		m_Put += sizeof(Entry) + sizeof(T);
		item->u32value = 0;
		item->Valid = 1;
		item->Size = sizeof(T);
		memcpy(item + 1, &t, sizeof(T));
		return true;
	}
protected:
	void SkipItem(Entry *itm) {
		m_Get += itm->Size + sizeof(Entry);
		if (m_Get >= Size)
			m_Get = 0;
	}
private:
	size_t m_Put, m_Get;
	LockPolicy m_Lock;
	char m_Memory[Size];

	template<class T>
	void Check() { 
		static_assert(std::is_pod<BaseItem>::value, "DynamicCyclicBuffer accepts only POD types"); 
		static_assert(sizeof(T) >= 4, "DynamicCyclicBuffer accepts only types larger than 4 bytes"); 
	}
};

//----------------------------------------------------------------

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
