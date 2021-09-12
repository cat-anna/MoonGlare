#pragma once

#include "math/vector.hpp"
#include "types.hpp"

namespace MoonGlare::Renderer {

class iFrameBuffer {
public:
    virtual ~iFrameBuffer() = default;
    virtual void Reset() = 0;
    virtual uint8_t *AllocateMemory(size_t byte_count, bool zero = false) = 0;

    template <typename T>
    T *AllocateElements(size_t count, bool zero = false) {
        return reinterpret_cast<T *>(AllocateMemory(count * sizeof(T), zero));
    }
};

} // namespace MoonGlare::Renderer