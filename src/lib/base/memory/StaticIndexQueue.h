#pragma once

#include <cassert>
#include <type_traits>

namespace MoonGlare::Memory {

template<class T, T SIZE>
struct StaticIndexQueue  {
    using Item = T;
    static constexpr Item Size = SIZE;

    static_assert(std::is_integral<Item>::value, "Item type for StaticIndexQueue must be an integer type");

    void Clear() {
        position = 0;
        count = Size;
        for (Item i = 0; i < (Item)Size; ++i)
            memory[i] = i;

        for (Item i = 0; i < (Item)Size / 2; ++i)
            std::swap(memory[i], memory[Size - i - 1]);

    }

    bool Empty() const { return count == 0; }
    Item Count() const { return count; }

    bool Allocate(Item &itm) {
        if (Empty())
            return false;
        itm = memory[position];
        ++position;
        --count;
        if (position >= Size)
            position = 0;
        return true;
    }

    bool Release(Item itm) {
        if (count >= Size)
            return false;
        memory[(position + count) % Size] = itm;
        ++count;
        return true;
    }
private:
    size_t position, count;
    std::array<Item, Size> memory;
};

}
