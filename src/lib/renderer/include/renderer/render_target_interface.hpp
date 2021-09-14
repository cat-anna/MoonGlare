#pragma once

#include "math/vector.hpp"
#include "types.hpp"

namespace MoonGlare::Renderer {

class iRenderTarget {
public:
    virtual ~iRenderTarget() = default;

    virtual void Reset() = 0;
    virtual void ReleaseResources(CommandQueue *command_queue) = 0;

    virtual uint8_t *AllocateMemory(size_t byte_count, bool zero = false) = 0;

    virtual math::ivec2 BufferSize() const = 0;

    // virtual void SetCamera(void *camera) = 0;

    template <typename T>
    T *AllocateElements(size_t count, bool zero = false) {
        return reinterpret_cast<T *>(AllocateMemory(count * sizeof(T), zero));
    }
};

} // namespace MoonGlare::Renderer
