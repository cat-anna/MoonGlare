#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <ctime>
#include <vector>
#include <Memory/MultiAllocator.h>
#include <Memory/GenerationLinearBuffer.h>
#include <Memory/Handle.h>
#include <cmath>

using namespace Space::Memory;

#include "Common.h"

//-----------------------------------------------------------------------------

template<class ... ARGS>
using StaticMultiAllocator_t = StaticMultiAllocator<1024, ARGS...>;

template<class ... ARGS>
using StaticMultiAllocator512_t = StaticMultiAllocator<512, ARGS...>;

template<class ... ARGS>
using StaticMultiAllocator256_t = StaticMultiAllocator<256, ARGS...>;

//-----------------------------------------------------------------------------

TEST(Memory_GenerationLinearBuffer, test_0) {
	using Handle_t = DoubleHandle32<8>;

	using type_t = GenerationLinearBuffer<
		StaticMultiAllocator_t,
		Handle_t>;

	type_t allocator;
	size_t tempidx;

	Handle_t Handle0 = {};
	Handle0.m_IntegerValue = 0xFADEDEAD; //some garbage
	ASSERT_FALSE(allocator.IsHandleValid(Handle0));

	Handle_t handleA = allocator.Allocate();
	//	std::cout << "Alloc handleA: " << PrintHandle(handleA, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_TRUE(allocator.GetTableIndex(handleA, tempidx));
	ASSERT_EQ(0, tempidx);
	ASSERT_EQ(1, allocator.Allocated());

	Handle_t handleB = allocator.Allocate();
	//	std::cout << "Alloc handleB: " << PrintHandle(handleB, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_TRUE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.GetTableIndex(handleB, tempidx));
	ASSERT_EQ(1, tempidx);
	ASSERT_EQ(2, allocator.Allocated());

	Handle_t handleC = allocator.Allocate();
	//	std::cout << "Alloc handleC: " << PrintHandle(handleC, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_TRUE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.IsHandleValid(handleC));
	ASSERT_TRUE(allocator.GetTableIndex(handleC, tempidx));
	ASSERT_EQ(2, tempidx);
	ASSERT_EQ(3, allocator.Allocated());

//free attempt
	ASSERT_FALSE(allocator.Free(handleB));
	ASSERT_EQ(3, allocator.Allocated());
	ASSERT_TRUE(allocator.GetTableIndex(handleA, tempidx));
	ASSERT_EQ(0, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleB, tempidx));
	ASSERT_EQ(1, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleC, tempidx));
	ASSERT_EQ(2, tempidx);

//allocate 4th
	Handle_t handleD = allocator.Allocate();
	//	std::cout << "Alloc handleC: " << PrintHandle(handleC, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_TRUE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.IsHandleValid(handleC));
	ASSERT_TRUE(allocator.IsHandleValid(handleD));
	ASSERT_TRUE(allocator.GetTableIndex(handleD, tempidx));
	ASSERT_EQ(3, tempidx);
	ASSERT_EQ(4, allocator.Allocated());

//move b to back
	ASSERT_TRUE(allocator.MoveToBack(handleB));
	ASSERT_TRUE(allocator.GetTableIndex(handleA, tempidx));
	ASSERT_EQ(0, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleB, tempidx));
	ASSERT_EQ(3, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleC, tempidx));
	ASSERT_EQ(2, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleD, tempidx));
	ASSERT_EQ(1, tempidx);

//free attempt #2
	ASSERT_TRUE(allocator.Free(handleB));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.GetTableIndex(handleA, tempidx));
	ASSERT_EQ(0, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleC, tempidx));
	ASSERT_EQ(2, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleD, tempidx));
	ASSERT_EQ(1, tempidx);

//allocate 5th
	Handle_t handleE = allocator.Allocate();
	ASSERT_TRUE(allocator.IsHandleValid(handleE));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.GetTableIndex(handleA, tempidx));
	ASSERT_EQ(0, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleC, tempidx));
	ASSERT_EQ(2, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleD, tempidx));
	ASSERT_EQ(1, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleE, tempidx));
	ASSERT_EQ(3, tempidx);

//move a to back
	ASSERT_TRUE(allocator.MoveToBack(handleA));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.GetTableIndex(handleA, tempidx));
	ASSERT_EQ(3, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleC, tempidx));
	ASSERT_EQ(2, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleD, tempidx));
	ASSERT_EQ(1, tempidx);
	ASSERT_TRUE(allocator.GetTableIndex(handleE, tempidx));
	ASSERT_EQ(0, tempidx);

	ASSERT_TRUE(allocator.Free(handleA));
	ASSERT_TRUE(allocator.Free(handleC));
	ASSERT_TRUE(allocator.Free(handleD));
	ASSERT_TRUE(allocator.Free(handleE));

	ASSERT_EQ(0, allocator.Allocated());
#if 0
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.IsHandleValid(handleC));
	ASSERT_EQ(2, allocator.GetRawIndex(handleC));
	ASSERT_EQ(2, allocator.Allocated());
	ASSERT_EQ(2, handleC.GetIndex());

	Handle_t handleD = allocator.Allocate();
	//	std::cout << "Alloc handleD: " << PrintHandle(handleD, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.IsHandleValid(handleC));
	ASSERT_TRUE(allocator.IsHandleValid(handleD));
	ASSERT_EQ(3, allocator.GetRawIndex(handleD));
	ASSERT_EQ(3, allocator.Allocated());

	ASSERT_EQ(2, handleC.GetIndex());
	ASSERT_NE(handleD.GetIndex(), handleC.GetIndex());

	ASSERT_NE(handleD.GetIndex(), handleB.GetIndex());
	ASSERT_EQ(1, handleD.GetGeneration());

	Handle_t handleE = allocator.Allocate();
	//	std::cout << "Alloc handleE: " << PrintHandle(handleE, &allocator) << "\n";
	//	std::cout << "handleA: " << PrintHandle(handleA, &allocator) << "\n";
	//	std::cout << "handleB: " << PrintHandle(handleB, &allocator) << "\n";
	//	std::cout << "handleC: " << PrintHandle(handleC, &allocator) << "\n";
	//	std::cout << "handleD: " << PrintHandle(handleD, &allocator) << "\n";
	//	std::cout << "handleE: " << PrintHandle(handleE, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.IsHandleValid(handleC));
	ASSERT_TRUE(allocator.IsHandleValid(handleD));
	ASSERT_TRUE(allocator.IsHandleValid(handleE));
	ASSERT_EQ(4, handleE.GetIndex());
	ASSERT_EQ(4, allocator.GetRawIndex(handleE));

	allocator.Free(handleC);
	allocator.Free(handleD);

	//std::cout << "handleA: " << PrintHandle(handleA, &allocator) << "\n";
	//std::cout << "handleB: " << PrintHandle(handleB, &allocator) << "\n";
	//std::cout << "handleC: " << PrintHandle(handleC, &allocator) << "\n";
	//std::cout << "handleD: " << PrintHandle(handleD, &allocator) << "\n";
	//std::cout << "handleE: " << PrintHandle(handleE, &allocator) << "\n";

	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_FALSE(allocator.IsHandleValid(handleB));
	ASSERT_FALSE(allocator.IsHandleValid(handleC));
	ASSERT_FALSE(allocator.IsHandleValid(handleD));
	ASSERT_TRUE(allocator.IsHandleValid(handleE));
#endif
}
#if 0
TEST(Memory_GenerationLinearBuffer, generation_overflow_attempt) {
	using Handle_t = DoubleHandle32<8>;

	using type_t = GenerationLinearBuffer<
		StaticMultiAllocator256_t,
		Handle_t>;

	type_t allocator;

	std::vector<Handle_t> InvalidHandles;
	std::list<std::pair<int, Handle_t>> ValidHandles;

	for (size_t i = 0; i < 10 * 1000; ++i) {
		for (auto it : InvalidHandles)
			ASSERT_FALSE(allocator.IsHandleValid(it));
		for (auto &it : ValidHandles) {
			ASSERT_TRUE(allocator.IsHandleValid(it.second));
		}
		{
			auto pair = std::make_pair(i, allocator.Allocate());
			ValidHandles.emplace_back(pair);
		}
		{
			auto it = ValidHandles.begin();
			auto pair = *it;
			ValidHandles.erase(it);
			allocator.Free(pair.second);
			InvalidHandles.push_back(pair.second);
		}
	}
}
#endif
