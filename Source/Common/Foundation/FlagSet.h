
#pragma once

namespace MoonGlare {

template<typename ENUM>
struct FlagSet {
    using BaseType = std::underlying_type_t<ENUM>;

    FlagSet() = default;

    template<ENUM ... values>
    constexpr FlagSet() : storage(Mask(values...)) {}

    template<typename ... ARGS>
    constexpr FlagSet(ARGS&& ... args) : storage(Mask(std::forward<ARGS>(args)...)) { }

    template<typename ... ARGS>
    constexpr static BaseType Mask(ARGS&& ... args) {
        return ( ... | (1 << static_cast<BaseType>(args)));
    }

    template<typename ... ARGS>
    constexpr bool Test(ARGS&& ... args) const {
        return (storage & Mask(std::forward<ARGS>(args)...)) != 0
    }   

    constexpr bool Test(FlagSet flagset) const {
        return (storage & flagset.storage) != 0
    }

    template<typename ... ARGS>
    constexpr bool Set(bool set, ARGS&& ... args) const {
        auto mask = Mask(std::forward<ARGS>(args)...);
        if (set)
            storage | = mask;
        else
            storage &= ~mask;
    }

    void Clear() { storage = 0; }

    constexpr FlagSet And(FlagSet other) const {
        FlagSet r;
        r.storage = storage & other.storage;
        return r;
    }

    constexpr FlagSet Or(FlagSet other) const {
        FlagSet r;
        r.storage = storage | other.storage;
        return r;
    }

    FlagSet& operator =(const FlagSet& value) = default;
    FlagSet& operator =(FlagSet&& value) = default;
    FlagSet& operator =(ENUM value) { storage = Mask(value); return *this; }

    constexpr FlagSet operator &(ENUM value) const { return And(FlagSet(value)); }
    constexpr FlagSet operator &(FlagSet value) const { return And(value); }
    constexpr FlagSet operator |(ENUM value) const { return Or(FlagSet(value)); }
    constexpr FlagSet operator |(FlagSet value) const { return Or(value); }

    operator bool() const { return storage != 0; }
    explicit operator BaseType() const { return storage; }
private:
    BaseType storage = 0;
};


//has interference with eigen lib
//template<typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
//constexpr MoonGlare::FlagSet<T> operator | (T a, T b) {
//    return MoonGlare::FlagSet<T>(a, b);
//}

namespace detail {
    enum class FlagSetTest : int {
        None = 0,
        A = 1, B = 2, C = 4,
    };

    static constexpr int tst = sizeof(FlagSet<FlagSetTest>);
    
    static_assert(sizeof(FlagSet<FlagSetTest>) == sizeof(FlagSetTest));

    //constexpr static FlagSet<FlagSetTest> constexprtest = FlagSetTest::A | FlagSetTest::B;
}

}
