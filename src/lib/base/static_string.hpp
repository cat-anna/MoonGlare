#pragma once

#include <array>
#include <cstring>
#include <limits>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <type_traits>

namespace MoonGlare {

//ObjectSize_v is total size of class in bytes
template <size_t ObjectSize_v, typename Char_t = char, typename Length_t = uint8_t>
struct BasicStaticString {
    static_assert(std::numeric_limits<Length_t>::max() > ObjectSize_v);

    using LengthType = Length_t;
    using CharType = Char_t;
    using traits = std::char_traits<CharType>;

    static constexpr size_t Capacity = ((ObjectSize_v - 1) / sizeof(CharType)) - sizeof(LengthType);

    BasicStaticString() = default;
    BasicStaticString(const CharType *str) {
        if (str)
            Set(str, traits::length(str));
        else
            Set("", 0);
    }
    BasicStaticString(const std::basic_string<CharType> &str) { Set(str.c_str(), str.size()); }
    BasicStaticString &operator=(const CharType *str) {
        if (str)
            Set(str, traits::length(str));
        else
            Set("", 0);
        return *this;
    }
    BasicStaticString &operator=(const std::basic_string<CharType> &str) {
        Set(str.c_str(), str.size());
        return *this;
    }

    const CharType *c_str() const { return &table[0]; };
    size_t size() const { return length; }
    bool empty() const { return length == 0; }

    operator std::basic_string<CharType>() const { return std::basic_string<CharType>(c_str(), size()); }

    operator std::basic_string_view<CharType>() const { return std::basic_string_view<CharType>(c_str(), size()); }

private:
    LengthType length;
    std::array<CharType, Capacity + 1> table; //+1 to include trailing \0

    void Set(const CharType *str, size_t strlen) {
        LengthType len = static_cast<LengthType>(std::min(strlen, Capacity));
        traits::copy(&table[0], str, len);
        length = len;
        table[len] = 0;
    }
};

template <typename T, size_t S, typename L>
inline std::basic_string<T> to_string(const BasicStaticString<S, T, L> &input) {
    return static_cast<std::basic_string<T>>(input);
}

template <typename T, size_t S, typename L>
inline void to_json(nlohmann::json &j, const BasicStaticString<S, T, L> &p) {
    j = to_string(p);
}

template <typename T, size_t S, typename L>
inline void from_json(const nlohmann::json &j, BasicStaticString<S, T, L> &p) {
    p = j.get<std::basic_string<T>>();
}

using ShortString = BasicStaticString<256, char>;
using ShortWString = BasicStaticString<256, wchar_t>;

// static_assert(std::is_trivially_constructible_v<ShortString>);
// static_assert(std::is_trivial_v<ShortString>);

// static_assert(ShortString::Capacity == 256 - 2);
// static_assert(ShortWString::Capacity == 256 / sizeof(wchar_t) - 2);

// static_assert(sizeof(ShortString) == 256);
// static_assert(sizeof(ShortWString) == 256);

} // namespace MoonGlare