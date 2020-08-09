#pragma once

#include "../Configuration.h"

namespace MoonGlare::Memory {

template <class TYPE>
struct ItemSize {
	using Item_t = typename TYPE::Item_t;
	constexpr static size_t ByteSize() { return sizeof(Item_t); }
	constexpr static size_t BitSize() { return ByteSize() * 8; }
};

//TODO: Support for LockPolicy
template <class ALLOCATOR, size_t BITSIZE = ItemSize<ALLOCATOR>::BitSize() >
struct BaseGenerationBuffer {
	using Item_t = typename ALLOCATOR::Item_t;
	using Storage_t = ALLOCATOR;
	static_assert(std::is_integral<Item_t>::value, "Item type must be unsigned integral!");
	static_assert(std::is_unsigned<Item_t>::value, "Item type must be unsigned integral!");

	enum Enum : Item_t {
		ItemBitSize = BITSIZE,
		ElementByteSize = sizeof(Item_t),
		ElementBitSize = ElementByteSize * 8,
		ItemBitMask = (Item_t(~0)) >> (ElementBitSize - ItemBitSize),
	};

	template<typename ... ARGS>
	BaseGenerationBuffer(ARGS ... args) : m_Storage(std::forward<ARGS>(args)...) {}
	~BaseGenerationBuffer() {}
	//not supported yet:
	//copy ctor
	//move ctor
	//copy operator
	//move operator
	//swap function ?

	size_t IndexCount() const { return m_Storage.ElementCount(); }

	void Clear() { m_Storage.MemZero(); }

	Item_t Next(size_t Index) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(Index < IndexCount());
		return (++m_Storage[Index]) & ItemBitMask;
	}

	bool Test(size_t Index, Item_t Generation) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(Index < IndexCount());
		return (m_Storage[Index] & ItemBitMask) == Generation;
	}

	Storage_t& GetStorage() { return m_Storage; }
	const Storage_t& GetStorage() const { return m_Storage; }
protected:
	Storage_t m_Storage;
};

template <class ALLOCATOR, class HANDLETYPE>
struct GenerationBuffer : public BaseGenerationBuffer<ALLOCATOR, HANDLETYPE::GenerationBits> {
	using Handle_t = HANDLETYPE;
	using BaseClass = BaseGenerationBuffer<ALLOCATOR, HANDLETYPE::GenerationBits>;
	using Item_t = typename BaseClass::Item_t;
	
	template<typename ... ARGS>
	GenerationBuffer(ARGS ... args) : BaseClass(std::forward<ARGS>(args)...) {}

	Handle_t NextHandle(size_t Index) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(Index < this->IndexCount());
		Handle_t h;
		h.SetIndex(Index);
		h.SetGeneration(this->Next(Index));
		return h;
	}

	bool TestHandle(Handle_t handle) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(handle.GetIndex() < this->IndexCount());
		return this->Test(static_cast<size_t>(handle.GetIndex()), static_cast<Item_t>(handle.GetGeneration())); 
	}

};

}



