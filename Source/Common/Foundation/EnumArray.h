#pragma once

#include <array>

namespace MoonGlare {

template<typename ENUM, typename ITEM, size_t CAPACITY>
struct EnumArray : public std::array<ITEM, CAPACITY> {
    using EnumType = ENUM;
    using ItemType = ITEM;
    static constexpr size_t capacity = CAPACITY;

    using BaseArray = std::array<ITEM, CAPACITY>;

    ItemType & operator[](EnumType e) { return BaseArray::operator[](static_cast<size_t>(e)); }
    const ItemType & operator[](EnumType e) const { return BaseArray::operator[](static_cast<size_t>(e)); }
};

}
