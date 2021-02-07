#include <gtest/gtest.h>
#include <Memory/MemoryAllocation.h>

using namespace Space::Memory;

//-----------------------------------------------------------------------------

template<class T>
void RawMemoryStackAllocator_NoResize(T &mem) {

	ASSERT_EQ(mem.GetStorage().ElementCount(), mem.Reserve());

	ASSERT_NE(nullptr, mem.template Allocate<int>());
	ASSERT_EQ(nullptr, mem.template Allocate<int>(1024));
	ASSERT_NE(nullptr, mem.template Allocate<int>());
	ASSERT_NE(nullptr, mem.template Allocate<int>(6));

	mem.Clear();
	ASSERT_EQ(mem.GetStorage().ElementCount(), mem.Reserve());
}

//-----------------------------------------------------------------------------

TEST(Memory_RawMemoryStackAllocator, DynamicExternalPointerAllocator) {
	using mem_t = RawMemoryStackAllocator<DynamicExternalPointerAllocator>;

	mem_t mem;
	char memory[4096];

	mem.GetStorage().SetMemory(memory, sizeof(memory));

	RawMemoryStackAllocator_NoResize(mem);
}

template<class T>
using Allocator_t = StaticAllocator<T, 1024>;

TEST(Memory_RawMemoryStackAllocator, StaticAllocator) {

	using mem_t = RawMemoryStackAllocator<Allocator_t>;

	mem_t mem;

	RawMemoryStackAllocator_NoResize(mem);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

