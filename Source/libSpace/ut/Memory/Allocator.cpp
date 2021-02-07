#include <gtest/gtest.h>
#include <Memory/Allocator.h>

using namespace Space::Memory;

TEST(Memory_StaticAllocator, pod) {
	enum {
		SIZE = 5,
	};

	using mem_t = StaticAllocator<int, SIZE>;
	ASSERT_TRUE(mem_t::IsPOD::value);

	mem_t memory;
	ASSERT_FALSE(memory.Realloc(5));

	ASSERT_EQ(SIZE, memory.ElementCount());
	ASSERT_EQ(SIZE * sizeof(mem_t::Item_t), memory.ByteSize());

	ASSERT_ANY_THROW(memory[SIZE] = 5);
}

TEST(Memory_StaticAllocator, nopod) {
	enum {
		SIZE = 5,
	};

	using mem_t = StaticAllocator<std::string, SIZE>;
	ASSERT_EQ(false, mem_t::IsPOD::value);

	mem_t memory;
	ASSERT_EQ(false, memory.Realloc(5));

	ASSERT_EQ(SIZE, memory.ElementCount());
	ASSERT_EQ(SIZE * sizeof(mem_t::Item_t), memory.ByteSize());

	ASSERT_ANY_THROW(memory[SIZE].capacity());
}

//-------------------------------------------------------------------------------------------------

TEST(Memory_StaticPointerAllocator, pod) {
	enum {
		SIZE = 5,
	};

	using mem_t = StaticPointerAllocator<int, SIZE>;
	mem_t memory;

	ASSERT_EQ(true, mem_t::IsPOD::value);

	ASSERT_EQ(false, memory.Realloc(5));

	ASSERT_EQ(SIZE, memory.ElementCount());
	ASSERT_EQ(SIZE * sizeof(mem_t::Item_t), memory.ByteSize());
}

TEST(Memory_StaticPointerAllocator, nopod) {
	enum {
		SIZE = 5,
	};

	using mem_t = StaticPointerAllocator<std::string, SIZE>;
	mem_t memory;

	ASSERT_EQ(false, mem_t::IsPOD::value);

	ASSERT_EQ(false, memory.Realloc(5));

	ASSERT_EQ(SIZE, memory.ElementCount());
	ASSERT_EQ(SIZE * sizeof(mem_t::Item_t), memory.ByteSize());
}

TEST(Memory_StaticPointerAllocator, pod_swap) {
	enum {
		SIZE = 5,
	};

	using mem_t = StaticPointerAllocator<int, SIZE>;

	mem_t memory1;
	mem_t memory2;

	memory1.MemZero();
	memory1[2] = 1;
	memory2.MemZero();
	memory2[1] = 1;

	memory1.swap(memory2);

	ASSERT_EQ(1, memory1[1]);
	ASSERT_EQ(0, memory1[2]);

	ASSERT_EQ(1, memory2[2]);
	ASSERT_EQ(0, memory2[1]);
}

TEST(Memory_StaticPointerAllocator, nopod_swap) {
	enum {
		SIZE = 5,
	};

	using mem_t = StaticPointerAllocator<std::string, SIZE>;

	mem_t memory1;
	mem_t memory2;

	memory1.Fill("x");
	memory1[2] = "2";
	memory2.Fill("x");
	memory2[1] = "1";

	memory1.swap(memory2);

	ASSERT_EQ("1", memory1[1]);
	ASSERT_EQ("x", memory1[2]);

	ASSERT_EQ("2", memory2[2]);
	ASSERT_EQ("x", memory2[1]);
}

//-------------------------------------------------------------------------------------------------

TEST(Memory_DynamicPointerAllocator, pod) {
	using mem_t = DynamicPointerAllocator<int>;
	ASSERT_EQ(true, mem_t::IsPOD::value);

	mem_t memory;

	ASSERT_EQ(true, memory.Realloc(5));
	ASSERT_EQ(5, memory.ElementCount());

	ASSERT_NO_THROW(memory[2] = 5);

	ASSERT_EQ(true, memory.Realloc(70));
	ASSERT_EQ(70, memory.ElementCount());

	ASSERT_EQ(5, memory[2]);

	ASSERT_EQ(true, mem_t::IsPOD::value);

	ASSERT_EQ(true, memory.Realloc(7));
	ASSERT_EQ(7, memory.ElementCount());

	ASSERT_NO_THROW(memory[6] = 5);
	ASSERT_ANY_THROW(memory[7] = 5);

	ASSERT_EQ(true, memory.Realloc(5));
	ASSERT_EQ(5, memory.ElementCount());

	ASSERT_NO_THROW(memory[4] = 5);
	ASSERT_ANY_THROW(memory[5] = 5);
}

TEST(Memory_DynamicPointerAllocator, nopod) {
	using mem_t = DynamicPointerAllocator<std::string>;
	ASSERT_EQ(false, mem_t::IsPOD::value);

	mem_t memory;

	ASSERT_EQ(true, memory.Realloc(5));
	ASSERT_EQ(5, memory.ElementCount());

	ASSERT_EQ(true, memory.Realloc(7));
	ASSERT_EQ(7, memory.ElementCount());


	ASSERT_NO_THROW(memory[6] = "1");
	ASSERT_ANY_THROW(memory[7] = "2");

	ASSERT_EQ(true, memory.Realloc(5));
	ASSERT_EQ(5, memory.ElementCount());

	ASSERT_NO_THROW(memory[4] = 5);
	ASSERT_ANY_THROW(memory[5] = 5);
}

TEST(Memory_DynamicPointerAllocator, pod_swap) {
	using mem_t = DynamicPointerAllocator<int>;

	mem_t memory1;
	mem_t memory2;

	ASSERT_EQ(true, memory1.Realloc(5));
	ASSERT_EQ(5, memory1.ElementCount());
	ASSERT_EQ(true, memory2.Realloc(5));
	ASSERT_EQ(5, memory2.ElementCount());

	memory1.MemZero();
	memory1[2] = 1;
	memory2.MemZero();
	memory2[1] = 1;

	memory1.swap(memory2);

	ASSERT_EQ(1, memory1[1]);
	ASSERT_EQ(0, memory1[2]);

	ASSERT_EQ(1, memory2[2]);
	ASSERT_EQ(0, memory2[1]);
}

TEST(Memory_DynamicPointerAllocator, nopod_swap) {
	using mem_t = DynamicPointerAllocator<std::string>;

	mem_t memory1;
	mem_t memory2;

	ASSERT_EQ(true, memory1.Realloc(5));
	ASSERT_EQ(5, memory1.ElementCount());
	ASSERT_EQ(true, memory2.Realloc(5));
	ASSERT_EQ(5, memory2.ElementCount());

	memory1.Fill("x");
	memory1[2] = "2";
	memory2.Fill("x");
	memory2[1] = "1";

	memory1.swap(memory2);

	ASSERT_EQ("1", memory1[1]);
	ASSERT_EQ("x", memory1[2]);

	ASSERT_EQ("2", memory2[2]);
	ASSERT_EQ("x", memory2[1]);
}

//-------------------------------------------------------------------------------------------------
