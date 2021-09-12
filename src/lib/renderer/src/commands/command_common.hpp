#pragma once

#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

namespace detail {

template <typename Allocator>
struct SingleAllocate {
    using HandleType = typename Allocator::HandleType;
    HandleType *out;
    void Execute() const { Allocator::Allocate(1, out); }
};
template <typename Allocator>
struct SingleRelease {
    using HandleType = typename Allocator::HandleType;
    HandleType *out;
    void Execute() const { Allocator::Release(1, out); }
};

template <typename Allocator>
struct BulkAllocate {
    using HandleType = typename Allocator::HandleType;
    HandleType *out;
    GLsizei count;
    void Execute() const { Allocator::Allocate(count, out); }
};

template <typename Allocator>
struct BulkRelease {
    using HandleType = typename Allocator::HandleType;
    HandleType *out;
    GLsizei count;
    void Execute() const { Allocator::Release(count, out); }
};

} // namespace detail

} // namespace MoonGlare::Renderer::Commands
