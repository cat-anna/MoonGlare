#pragma once

#include <type_traits>
#include <cassert>
#include <array>
#include <random>

namespace MoonGlare::Memory {

template<class T, size_t SIZE>
struct GenerationBuffer {
    using Item = T;
    static constexpr size_t Size = SIZE;

    static_assert(std::is_integral<Item>::value, "Item type for GenerationBuffer must be an integer type");

    void Zero() { memset(&memory[0], 0, sizeof(memory)); }
    void Fill(T value) { memory.fill(value); }
    template<typename FUNC, typename = std::enable_if_t<!std::is_integral_v<FUNC>> >
    void Fill(FUNC func) { for (auto &item : memory) item = func(); }
    void FillRandom() {
        std::random_device rd;  
        std::mt19937 gen(rd()); 
        Fill(gen);
    }

    T NewGeneration(size_t index) {
        assert(index < Size);
        return ++memory[index];
    }

    T Generation(size_t index) const {
        assert(index < Size);
        return memory[index];
    }
private:
    std::array<T, SIZE> memory;
};

}
