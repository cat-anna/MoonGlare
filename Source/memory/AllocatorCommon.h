#pragma once

#include <type_traits>
#include <memory>

namespace MoonGlare::Memory {

struct FreeDeallocator {
	template<class T>
	void operator()(T *ptr) noexcept {
		free((void*)ptr);
	}
};

struct NullDeallocator {
	template<class T>
	void operator()(T *) noexcept {}
};

//-----------------------------------------------------------------------------

template <class T>
struct AllocatorPODCommon {
	using IsPOD = std::true_type;
	static_assert(std::is_pod<T>::value, "Item type is not POD!");
};

template <class T>
struct AllocatorNOPODCommon {
	using IsPOD = std::false_type;
	static_assert(!std::is_pod<T>::value, "Item type is POD!");
};

template <class T>
using AllocatorCommon = typename std::conditional<std::is_pod<T>::value, AllocatorPODCommon<T>, AllocatorNOPODCommon<T>>::type;

}
