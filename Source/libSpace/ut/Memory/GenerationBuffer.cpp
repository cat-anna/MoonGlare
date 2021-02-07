#include <gtest/gtest.h>
#include <Memory/MemoryAllocation.h>
#include <Memory/GenerationBuffer.h>
#include <Memory/Handle.h>

using namespace Space::Memory;

//-----------------------------------------------------------------------------


TEST(Memory_BaseGenerationBuffer, StaticAllocator) {
	using Allocator_t = StaticAllocator<uint16_t, 1024>;
	using mem_t = BaseGenerationBuffer<Allocator_t>;
	mem_t mem;

	mem.GetStorage().MemZero();

	ASSERT_EQ(mem.GetStorage().ElementCount(), mem.IndexCount());

	ASSERT_EQ(1, mem.Next(1));
	ASSERT_NE(1, mem.Next(1));
	auto item = mem.Next(2);
	ASSERT_TRUE(mem.Test(2, item));
	mem.Next(2);
	ASSERT_FALSE(mem.Test(2, item));
}

TEST(Memory_GenerationBuffer, StaticAllocator_DoubleHandle32) {
	using Allocator_t = StaticAllocator<uint32_t, 1024>;
	using mem_t = GenerationBuffer<Allocator_t, DoubleHandle32<2>>;
	mem_t mem;

	mem.GetStorage().MemZero();

	ASSERT_EQ(mem.GetStorage().ElementCount(), mem.IndexCount());

	ASSERT_EQ(1u, mem.Next(1));
	ASSERT_NE(1u, mem.Next(1));
	auto h = mem.NextHandle(2);
	ASSERT_TRUE(mem.TestHandle(h));
	mem.Next(2);
	ASSERT_FALSE(mem.TestHandle(h));
}

TEST(Memory_GenerationBuffer, StaticAllocator_TripleHandle32) {
	using Allocator_t = StaticAllocator<uint32_t, 1024>;
	using mem_t = GenerationBuffer<Allocator_t, TripleHandle32<2, 2>>;
	mem_t mem;

	mem.GetStorage().MemZero();

	ASSERT_EQ(mem.GetStorage().ElementCount(), mem.IndexCount());

	ASSERT_EQ(1u, mem.Next(1));
	ASSERT_NE(1u, mem.Next(1));
	auto h = mem.NextHandle(2);
	ASSERT_TRUE(mem.TestHandle(h));
	mem.Next(2);
	ASSERT_FALSE(mem.TestHandle(h));
}
//-----------------------------------------------------------------------------

TEST(Memory_BaseGenerationBuffer, BitClamp) {
	using Allocator_t = StaticAllocator<uint32_t, 1024>;
	using mem_t = BaseGenerationBuffer<Allocator_t, 2>;
	mem_t mem;
	mem.GetStorage().MemZero();

	size_t index = 2;
	auto item = mem.Next(index);
	ASSERT_TRUE(mem.Test(index, item));
	mem.Next(index);
	ASSERT_FALSE(mem.Test(index, item));
	mem.Next(index);
	ASSERT_FALSE(mem.Test(index, item));
	mem.Next(index);
	ASSERT_FALSE(mem.Test(index, item));
	mem.Next(index);
	ASSERT_TRUE(mem.Test(index, item));
}

//-----------------------------------------------------------------------------

TEST(Memory_GenerationBuffer, BitClamp_DoubleHandle32) {
	using Allocator_t = StaticAllocator<uint32_t, 1024>;
	using mem_t = GenerationBuffer<Allocator_t, DoubleHandle32<2>>;
	mem_t mem;
	mem.GetStorage().MemZero();

	size_t index = 2;
	auto h = mem.NextHandle(index);
	ASSERT_TRUE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_FALSE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_FALSE(mem.TestHandle(h));
	mem.Next(index); 
	ASSERT_FALSE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_TRUE(mem.TestHandle(h));
}

TEST(Memory_GenerationBuffer, BitClamp_TripleHandle32) {
	using Allocator_t = StaticAllocator<uint32_t, 1024>;
	using mem_t = GenerationBuffer<Allocator_t, TripleHandle32<2, 2>>;
	mem_t mem;
	mem.GetStorage().MemZero();

	size_t index = 2;
	auto h = mem.NextHandle(index);
	ASSERT_TRUE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_FALSE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_FALSE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_FALSE(mem.TestHandle(h));
	mem.Next(index);
	ASSERT_TRUE(mem.TestHandle(h));
}
