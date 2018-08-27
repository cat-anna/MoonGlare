#pragma once

#include <array>
#include <type_traits>
#include <cstring>
#include <string_view>
#include <string>

namespace MoonGlare {

template <size_t Capacity_v, typename Char_t = char, typename Length_t = uint8_t>
struct BasicStaticString {
    static constexpr size_t Capacity = Capacity_v - 1;
    using CharType = Char_t;
    using LengthType = Length_t;
    using traits = std::char_traits<CharType>;

    BasicStaticString() = default;
    BasicStaticString(const CharType *str) { Set(str, traits::length(str)); }
    BasicStaticString(const std::basic_string<CharType> &str) { Set(str.c_str(), str.size()); }
    BasicStaticString& operator=(const CharType *str) {
        Set(str, traits::length(str));
        return *this;
    }                                                   
    BasicStaticString& operator=(const std::basic_string<CharType> &str) {
        Set(str.c_str(), str.size());
        return *this;
    }

    const CharType* c_str() const { return &table[0]; };
    size_t size() const { return length; }

    operator std::basic_string<CharType>() const {
        return std::basic_string<CharType>(c_str(), size());
    }
private:
    LengthType length;
    std::array<CharType, Capacity + 1> table;

    void Set(const CharType *str, size_t strlen) {
        LengthType len = (LengthType)std::min(strlen, Capacity);
        traits::copy(&table[0], str, len);
        length = len;
        table[len] = 0;
    }
};

using ShortString = BasicStaticString<255, char>;

namespace detail {

static_assert(std::is_trivially_constructible_v<ShortString>);
static_assert(std::is_trivial_v<ShortString>);

}
}