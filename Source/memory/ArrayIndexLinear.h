#pragma once

#include <array>
#include <cassert>
#include <vector>

#include "ArrayIndexCommon.h"

namespace MoonGlare::Memory {

template <typename ElementIndex_t, ElementIndex_t ElementLimit_v, typename CALLBACK = void>
struct alignas(16) ArrayIndexLinear {
    using ElementIndex = ElementIndex_t;
    using ElementIndexIntType = typename detail::ArrayIndexIndexType<
        typename ElementIndex, typename std::is_enum<typename ElementIndex_t>::type>::IntType;

    static constexpr ElementIndex ElementLimit = ElementLimit_v;
    static constexpr ElementIndex InvalidIndex = ElementIndex(~(ElementIndex(0)));
    static_assert(std::is_integral_v<ElementIndex> || std::is_enum_v<ElementIndex>);

    using ElementIndexVector = std::vector<ElementIndex>;

    static constexpr bool HasCallback = std::is_class_v<CALLBACK>;
    static constexpr bool HasReleaseCallback() {
        if constexpr (HasCallback)
            return detail::has_member_function_ReleaseElement<void (CALLBACK::*)(
                ElementIndex)>::value;
        else
            return false;
    }
    static constexpr bool HasInitCallback() {
        if constexpr (HasCallback)
            return detail::has_member_function_InitElement<void (CALLBACK::*)(ElementIndex)>::value;
        else
            return false;
    }
    static constexpr bool HasSwapCallback() {
        if constexpr (HasCallback)
            return detail::has_member_function_SwapValues<void (CALLBACK::*)(ElementIndex,
                                                                             ElementIndex)>::value;
        else
            return false;
    }
    static constexpr bool HasGetNameCallback() {
        if constexpr (HasCallback)
            return false; // detail::has_member_function_GetElementName<std::string(CALLBACK::*) (ElementIndex)const>::value;
        else
            return false;
    }
    static constexpr bool HasClearCallback() {
        if constexpr (HasCallback)
            return detail::has_member_function_ClearArrays<void (CALLBACK::*)()>::value;
        else
            return false;
    }

    //---------------------------------------------------------

    std::string ElementToString(ElementIndex index) const {
        if constexpr (false) //HasGetNameCallback())
            return reinterpret_cast<const CALLBACK *>(This)->GetElementName(index);
        else
            return std::to_string(index);
    }

    //---------------------------------------------------------

    void Clear() {
        if constexpr (HasClearCallback())
            reinterpret_cast<CALLBACK *>(this)->ClearArrays();
        allocated = (ElementIndex)0;
    }

    ElementIndex Allocated() { return allocated; }
    ElementIndex LastIndex() const { return (ElementIndex)((ElementIndexIntType)allocated - 1); }

    ElementIndex Allocate() {
        auto e = NextIndex();
        if constexpr (HasInitCallback())
            reinterpret_cast<CALLBACK *>(this)->InitElement(e);
        return e;
    }

    void RemoveElement(ElementIndex index) {
        ElementIndexr = LastIndex();
        SwapIndexes(r, i);

        if constexpr (HasReleaseCallback())
            reinterpret_cast<CALLBACK *>(this)->ReleaseElement(r);

        RemoveLast();
    }

    void RemoveAll() {
        if constexpr (HasReleaseCallback()) {
            for (size_t i = 0; i < Allocated(); ++i) {
                auto index = (ElementIndex)i;
                reinterpret_cast<CALLBACK *>(this)->ReleaseElement(index);
            }
        }
        allocated = (ElementIndex)0;
    }

    //---------------------------------------------------------

    template <typename T>
    using Array = typename std::array<T, ElementLimit>;

    //TODO: MoveIndex(ElementIndex src, ElementIndex dst)
    void SwapIndexes(ElementIndex a, ElementIndex b) {
        assert(a != InvalidIndex);
        assert(b != InvalidIndex);

        if constexpr (HasSwapCallback())
            reinterpret_cast<CALLBACK *>(this)->SwapValues(a, b);
    }

private:
    ElementIndex allocated; // = 0;
    char __padding[16 - sizeof(ElementIndex)];

    ElementIndex NextIndex() {
        ElementIndex ei = allocated;
        allocated = (ElementIndex)((ElementIndexIntType)allocated + 1);
        return ei;
    };
    void RemoveLast() { allocated = (ElementIndex)((ElementIndexIntType)allocated - 1); }
};

namespace detail {
struct TestArrayIndexLinear : public ArrayIndexLinear<uint32_t, 16, TestArrayIndexLinear> {};
static_assert(std::is_trivial_v<TestArrayIndexLinear>);
static_assert(std::is_trivially_constructible_v<TestArrayIndexLinear>);
static_assert((sizeof(TestArrayIndexLinear) % 16) == 0);
} // namespace detail

} // namespace MoonGlare::Memory