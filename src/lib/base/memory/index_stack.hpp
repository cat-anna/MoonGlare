#pragma once

#include "pointer_memory_array.hpp"
#include <array>
#include <cassert>
#include <type_traits>

namespace MoonGlare {

template <typename Item_t>
struct alignas(16) IndexStack {
    using Item = Item_t;

    IndexStack() = default;
    IndexStack(const IndexStack &) = default;
    IndexStack &operator=(const IndexStack &) = default;

    IndexStack(Item *array, size_t size) : array_size(size), array(array) {}
    IndexStack(const PointerMemory<Item> &ptr) : array_size(ptr.size()), array(ptr.get()) {}

    void Clear() {
        position = 0;
        for (Item i = 0; i < array_size; ++i)
            array[i] = i;
    }

    bool Empty() const { return position == 0; }
    Item Count() const { return position; }

    bool Get(Item &itm) {
        if (Empty()) {
            return false;
        }
        itm = array[position];
        --position;
        return true;
    }

    bool Push(Item itm) {
        if (position >= array_size) {
            return false;
        }
        array[position] = itm;
        ++position;
        return true;
    }

    void PopulateContent(Item count) {
        while (count > 0 && Push(--count))
            ;
    }

private:
    size_t array_size = 0;
    size_t position = 0;
    Item *array = nullptr;
};

// static_assert((sizeof(IndexStack<uint64_t>) % 16) == 0);

} // namespace MoonGlare
