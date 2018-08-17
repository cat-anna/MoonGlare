#pragma once

#include <type_traits>
#include <cassert>
#include <array>

namespace MoonGlare::Memory {

template<class T, T SIZE>
struct StaticIndexStack {
    using Item = T;
    static constexpr T Size = SIZE;

    static_assert(std::is_integral<Item>::value, "Item type for StaticIndexQueue must be an integer type");

    void Clear() {
        position = 0;
        for (Item i = 0; i < (Item)Size; ++i)
            memory[i] = i;
    }

    bool empty() const { return position == 0; }
    T count() const { return position; }

    bool get(Item &itm) {
        if (empty())
            return false;
        itm = memory[position];
        --position;
        return true;
    }

    bool push(Item itm) {
        if (position >= Size)
            return false;
        memory[position] = itm;
        ++position;
        return true;
    }
private:
    T position;
    std::array<T, Size> memory;
};

static_assert(std::is_trivial_v<StaticIndexStack<uint32_t, 32>>);

}
