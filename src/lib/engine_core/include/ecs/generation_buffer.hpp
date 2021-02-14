#pragma once

#include "pointer_memory_array.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <random>
#include <type_traits>

namespace MoonGlare::ECS {

template <typename Index_t, typename Generation_t = Index_t>
struct alignas(16) GenerationBuffer {
    using Index = Index_t;
    using Generation = Generation_t;
    static_assert(std::is_unsigned_v<Generation>, "Item type must be unsigned integral!");

    GenerationBuffer() = default;
    GenerationBuffer(const GenerationBuffer &) = default;
    GenerationBuffer &operator=(const GenerationBuffer &) = default;

    GenerationBuffer(Generation *memory, Index size) : array_size(size), generation_array(memory) {}
    GenerationBuffer(const PointerMemory<Generation> &ptr) : array_size(ptr.size()), generation_array(ptr.get()) {}

    size_t Size() const { return array_size; }

    void Zero() { memset(generation_array, 0, Size() * sizeof(Generation)); }
    void Randomize() {
        std::random_device rd;
        std::mt19937_64 mt(rd());
        std::generate(generation_array, generation_array + array_size,
                      [&mt]() { return static_cast<Generation_t>(mt()); });
    }

    Generation Get(Index index) { return (generation_array[index]); }

    Generation Next(Index index) {
        assert(index < Size());
        return (++generation_array[index]);
    }

    bool Test(Index index, Generation generation) const {
        return index < array_size && generation_array[index] == generation;
    }

private:
    size_t array_size = 0;
    Generation *generation_array = nullptr;
};

static_assert((sizeof(GenerationBuffer<uint64_t, uint64_t>) % 16) == 0);

} // namespace MoonGlare::ECS
