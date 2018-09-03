#pragma once

#include <boost/align.hpp>

namespace MoonGlare::Memory {

template<class T>
using aligned_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

template<class T, class... Args>
inline aligned_ptr<T> make_aligned(Args&&... args) {
    auto ptr = boost::alignment::aligned_alloc(boost::alignment::alignment_of<T>::value, sizeof(T));
    if (!ptr) {
        throw std::bad_alloc();
    }
    try {
        auto q = ::new(ptr) T(std::forward<Args>(args)...);
        return aligned_ptr<T>(q);
    }
    catch (...) {
        boost::alignment::aligned_free(ptr);
        throw;
    }
}

}
