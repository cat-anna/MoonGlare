#pragma once

#if 0

#include "CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands {

namespace detail {

template<typename T>
struct MemoryStore {
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(std::is_trivially_constructible_v<T>);

    T data;
    T *dest;

    void Run() {
        memcpy(dest, &data, sizeof(T));
    }
};
}

template<typename T>
using MemoryStore = RunnableCommandTemplate<detail::MemoryStore<T>>;

} //namespace MoonGlare::Renderer::Commands

#endif
