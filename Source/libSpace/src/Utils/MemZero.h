#pragma once

#include "../Configuration.h"

namespace Space {

template<class T, size_t S>
inline void MemZero(std::array<T, S> &arr) {
	static_assert(std::is_pod<T>::value, "T must be pod type!");
	memset(&arr, 0, S * sizeof(T));
}

} //namespace Space
