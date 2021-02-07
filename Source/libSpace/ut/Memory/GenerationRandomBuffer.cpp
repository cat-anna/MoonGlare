#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <ctime>
#include <vector>
#include <Memory/MultiAllocator.h>
#include <Memory/GenerationRandomBuffer.h>
#include <Memory/Handle.h>

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

TEST(Memory_GenerationRandomBuffer, test_0) {
	using Handle_t = DoubleHandle32<4>;

	using type_t = GenerationRandomBuffer<
						StaticMultiAllocator_t, 
						Handle_t>;

	type_t allocator;

	Handle_t Handle0 = {};
	Handle0.m_IntegerValue = 0xFADEDEAD; //some garbage
	ASSERT_FALSE(allocator.IsHandleValid(Handle0));

	Handle_t handleA = allocator.Allocate();
//	std::cout << "Alloc handleA: " << PrintHandle(handleA, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_EQ(0, allocator.GetRawIndex(handleA));
	ASSERT_EQ(1, allocator.Allocated());

	Handle_t handleB = allocator.Allocate();
//	std::cout << "Alloc handleB: " << PrintHandle(handleB, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_TRUE(allocator.IsHandleValid(handleB));
	ASSERT_EQ(1, allocator.GetRawIndex(handleB));
	ASSERT_EQ(2, allocator.Allocated());

	Handle_t handleC = allocator.Allocate();
//	std::cout << "Alloc handleC: " << PrintHandle(handleC, &allocator) << "\n";
	ASSERT_TRUE(allocator.IsHandleValid(handleA));
	ASSERT_TRUE(allocator.IsHandleValid(handleB));
	ASSERT_TRUE(allocator.IsHandleValid(handleC));
	ASSERT_EQ(2, allocator.GetRawIndex(handleC));
	ASSERT_EQ(3, allocator.Allocated());

	allocator.Free(handleB);
//	std::cout << "FREE B\n";
//	std::cout << "handleA: " << PrintHandle(handleA, &allocator) << "\n";
//	std::cout << "handleB: " << PrintHandle(handleB, &allocator) << "\n";
//	std::cout << "handleC: " << PrintHandle(handleC, &allocator) << "\n";
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
}

//-----------------------------------------------------------------------------

TEST(Memory_GenerationRandomBuffer, random) {
	using Handle_t = DoubleHandle32<8>;

	unsigned seed = time(NULL);
	srand(seed);
	std::cout << "SEED: " << seed << "\n" << std::flush;

	struct data {
		int a;
		float b;
	};

	using type_t = GenerationRandomBuffer<
					StaticMultiAllocator256_t,
					Handle_t >;

	type_t allocator;
	
#if 0

	std::vector<Handle_t> InvalidHandles;
	std::list<std::pair<int, Handle_t>> ValidHandles;

	auto ReleaseRandom = [&] {
		auto it = ValidHandles.begin();
		std::advance(it, rand() % ValidHandles.size());
		auto pair = *it;
		ValidHandles.erase(it);
//			std::cout << "free (" << i << ") " << pair.first << ":" << PrintHandle(pair.second, &allocator) << " val:" << allocator.GetChannelValue<0>(pair.second).a << "\n";
//			std::cout << "allocated " << allocator.Allocated() << "\n";
		allocator.Free(pair.second);
		InvalidHandles.push_back(pair.second);
	};

	auto AllocHandle = [&](int i) {
		auto pair = std::make_pair(i, allocator.Allocate());
		ValidHandles.emplace_back(pair);
		allocator.GetChannelValue<0>(pair.second).a = i;
//			std::cout << "alloc (" << i << ") " << pair.first << ":" << PrintHandle(pair.second, &allocator) << " val:" << allocator.GetChannelValue<0>(pair.second).a << "\n";
	};

	auto AssertInvalid = [&] {
		for (auto it : InvalidHandles)
			ASSERT_FALSE(allocator.IsHandleValid(it));
	};
	auto AssertValid = [&] {
		for (auto &it : ValidHandles) {
			ASSERT_TRUE(allocator.IsHandleValid(it.second));
//			std::cout << "valid (" << i << ") " << it.first << ":" << PrintHandle(it.second, &allocator) << " val:" << allocator.GetChannelValue<0>(it.second).a  << "\n";
			ASSERT_EQ(it.first, allocator.GetChannelValue<0>(it.second).a);
		}
	};

	for (size_t i = 0; i < 10*1000; ++i) {
		//std::cout << "----------------------------------\n";

		AssertInvalid();
		AssertValid();

		switch (ValidHandles.empty() ? 0 : (rand() % 5)){
		case 0:
		case 1:
		case 2:
			if (allocator.Reserve() > 0) {
				AllocHandle(i);
				break;
			}
		case 3:
		case 4:
		default:
			ReleaseRandom();
			break;
		}
	}

	while (!ValidHandles.empty()) {
		AssertInvalid();
		AssertValid();
		ReleaseRandom();
	}

	AssertInvalid();
	AssertValid();

	ASSERT_EQ(0, allocator.Allocated());
	
#endif
}

//-----------------------------------------------------------------------------

TEST(Memory_GenerationRandomBuffer, generation_overflow_attempt) {
	using Handle_t = DoubleHandle32<8>;

	using type_t = GenerationRandomBuffer<
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
