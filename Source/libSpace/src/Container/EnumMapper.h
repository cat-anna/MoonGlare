#pragma once

#include "../Configuration.h"

#include "../Configuration.h"

namespace Space::Container {

template<typename Enum, typename Item>
struct alignas(std::alignment_of<Item>::value) EnumMapper {
    static_assert(std::is_enum_v<Enum>, "Not an enum type!");

    template<typename ...ARGS>
    EnumMapper(ARGS ... args) : array{std::forward<ARGS>(args)...} { }
    //EnumMapper(std::initializer_list<Item> l) : array(std::move(l)){}

    using Array = std::array<Item, static_cast<size_t>(Enum::MaxValue)>;

    Array* operator->() { return &array; }
    const Array* operator->() const { return &array; }

    Array& operator*() { return array; }
    const Array& operator*() const { return array; }

    Item& operator[](Enum e) { return array[static_cast<size_t>(e)]; }
    const Item& operator[](Enum e) const { return array[static_cast<size_t>(e)]; }
    Item& at(Enum e) { return array[static_cast<size_t>(e)]; }
    const Item& at(Enum e) const { return array[static_cast<size_t>(e)]; }

    size_t size() const { return arraty.size(); }
private:
    Array array;
};

}

