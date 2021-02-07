#pragma once

/*

libSpace wrapper for Constexpr Murmur hash 3 implementation

*/

#include "../Configuration.h"
#include <cstdint>

namespace Space {
namespace Utils {
namespace Detail {

#include "ConstexprMurmur3.h"

}

static const uint32_t Murmur3_32_Seed = 0x1657d5fa;

constexpr inline uint32_t Murmur3_32(const char *const key, const size_t length, const uint32_t seed) noexcept {
	return Detail::_Murmur3Loop_32(key, length, seed);
}

constexpr inline uint32_t Murmur3_32(const char *const str, const uint32_t seed) noexcept {
	return Detail::_Murmur3Loop_32(str, Detail::_StringLength(str), seed);
}

constexpr inline uint32_t MakeHash32(const char *const key, const size_t length) noexcept {
	return Murmur3_32(key, length, Murmur3_32_Seed);
}
constexpr inline uint32_t MakeHash32(const char *const str) noexcept {
	return Murmur3_32(str, Murmur3_32_Seed);
}

namespace HashLiterals {

constexpr uint32_t operator "" _Hash32(const char* str) { return MakeHash32(str); }
constexpr uint32_t operator "" _Hash32(const char* str, size_t size) { return MakeHash32(str, size); }

constexpr uint32_t operator "" _Murmur32(const char* str) { return Murmur3_32(str, Murmur3_32_Seed); }
constexpr uint32_t operator "" _Murmur32(const char* str, size_t size) { return Murmur3_32(str, size, Murmur3_32_Seed); }

}

}
}
