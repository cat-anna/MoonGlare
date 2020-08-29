#pragma once

#include <boost/align.hpp>

namespace MoonGlare {

template <class T>
using aligned_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

template <class T>
using aligned_array = std::unique_ptr<T[], boost::alignment::aligned_delete>;

template <class T, class... Args>
inline aligned_ptr<T> make_aligned(Args &&... args) {
    auto ptr = boost::alignment::aligned_alloc(boost::alignment::alignment_of<T>::value, sizeof(T));
    if (!ptr) {
        throw std::bad_alloc();
    }
    try {
        auto q = ::new (ptr) T(std::forward<Args>(args)...);
        return aligned_ptr<T>(q);
    } catch (...) {
        boost::alignment::aligned_free(ptr);
        throw;
    }
}

template <class T>
inline aligned_array<T>
make_aligned_array(size_t count, size_t alignment = boost::alignment::alignment_of<T>::value) {
    auto ptr = boost::alignment::aligned_alloc(alignment, sizeof(T) * count);
    if (!ptr) {
        throw std::bad_alloc();
    }
    return aligned_array<T>(reinterpret_cast<T *>(ptr));
}

template <typename T>
inline T Align16(T value) {
    if (value & 0xF)
        value = (value & ~0xF) + 0x10;
    return value;
}

} // namespace MoonGlare
