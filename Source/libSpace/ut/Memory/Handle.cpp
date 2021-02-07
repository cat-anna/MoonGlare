#include <type_traits>
#include <gtest/gtest.h>
#include <Memory/Handle.h>

using namespace Space::Memory;

//-----------------------------------------------------------------------------

TEST(Memory_HandleTypeDetection, DoubleHandle32) {
	using Handle4  = DoubleHandle32<4>;
	using Handle12 = DoubleHandle32<12>;
	using Handle16 = DoubleHandle32<16>;
	using Handle24 = DoubleHandle32<24>;
	using Handle28 = DoubleHandle32<28>;

	static_assert(std::is_same<Handle4::Generation_t, uint8_t>::value, "not an uint8_t type");
	static_assert(std::is_same<Handle4::Index_t, uint32_t>::value, "not an uint32_t type");

	static_assert(std::is_same<Handle12::Generation_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle12::Index_t, uint32_t>::value, "not an uint32_t type");

	static_assert(std::is_same<Handle16::Generation_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle16::Index_t, uint16_t>::value, "not an uint16_t type");

	static_assert(std::is_same<Handle24::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle24::Index_t, uint8_t>::value, "not an uint8_t type");

	static_assert(std::is_same<Handle28::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle28::Index_t, uint8_t>::value, "not an uint8_t type");

}

TEST(Memory_HandleTypeDetection, DoubleHandle64) {
	using Handle4  = DoubleHandle64<4>;
	using Handle12 = DoubleHandle64<12>;
	using Handle16 = DoubleHandle64<16>;
	using Handle24 = DoubleHandle64<24>;
	using Handle28 = DoubleHandle64<28>;
	using Handle32 = DoubleHandle64<32>;
	using Handle36 = DoubleHandle64<36>;
	using Handle48 = DoubleHandle64<48>;

	static_assert(std::is_same<Handle4::Generation_t, uint8_t>::value, "not an uint8_t type");
	static_assert(std::is_same<Handle4::Index_t, uint64_t>::value, "not an uint64_t type");

	static_assert(std::is_same<Handle12::Generation_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle12::Index_t, uint64_t>::value, "not an uint64_t type");

	static_assert(std::is_same<Handle16::Generation_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle16::Index_t, uint64_t>::value, "not an uint64_t type");

	static_assert(std::is_same<Handle24::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle24::Index_t, uint64_t>::value, "not an uint64_t type");

	static_assert(std::is_same<Handle28::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle28::Index_t, uint64_t>::value, "not an uint64_t type");

	static_assert(std::is_same<Handle32::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle32::Index_t, uint32_t>::value, "not an uint32_t type");

	static_assert(std::is_same<Handle36::Generation_t, uint64_t>::value, "not an uint64_t type");
	static_assert(std::is_same<Handle36::Index_t, uint32_t>::value, "not an uint32_t type");

	static_assert(std::is_same<Handle48::Generation_t, uint64_t>::value, "not an uint64_t type");
	static_assert(std::is_same<Handle48::Index_t, uint16_t>::value, "not an uint16_t type");
}

//-----------------------------------------------------------------------------

TEST(Memory_HandleTypeDetection, TripleHandle32) {
	using Handle4 = TripleHandle32<4, 16>;
	using Handle12 = TripleHandle32<12, 12>;
	using Handle16 = TripleHandle32<16, 4>;
	using Handle24 = TripleHandle32<24, 6>;
	using Handle28 = TripleHandle32<28, 2>;

	static_assert(std::is_same<Handle4::Generation_t, uint8_t>::value, "not an uint8_t type");
	static_assert(std::is_same<Handle4::Index_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle4::Type_t, uint16_t>::value, "not an uint16_t type");

	static_assert(std::is_same<Handle12::Generation_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle12::Index_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle12::Type_t, uint8_t>::value, "not an uint8_t type");

	static_assert(std::is_same<Handle16::Generation_t, uint16_t>::value, "not an uint16_t type");
	static_assert(std::is_same<Handle16::Index_t, uint8_t>::value, "not an uint8_t type");
	static_assert(std::is_same<Handle16::Type_t, uint16_t>::value, "not an uint16_t type");

	static_assert(std::is_same<Handle24::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle24::Index_t, uint8_t>::value, "not an uint8_t type");
	static_assert(std::is_same<Handle24::Type_t, uint8_t>::value, "not an uint8_t type");

	static_assert(std::is_same<Handle28::Generation_t, uint32_t>::value, "not an uint32_t type");
	static_assert(std::is_same<Handle28::Index_t, uint8_t>::value, "not an uint8_t type");
	static_assert(std::is_same<Handle28::Type_t, uint8_t>::value, "not an uint8_t type");

}

//TODO the same for TripleHandle64