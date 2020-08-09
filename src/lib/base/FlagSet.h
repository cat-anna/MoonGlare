#pragma once

namespace MoonGlare {

namespace detail {
template<typename E>
using is_scoped_enum = std::integral_constant<
    bool,
    std::is_enum<E>::value && !std::is_convertible<E, int>::value>;
}

template<typename ENUM>
struct FlagSet {
    using BaseType = std::underlying_type_t<ENUM>;

    FlagSet() = default;

    //template<ENUM ... values>
    //constexpr FlagSet() : storage(Mask(values...)) {}
    constexpr FlagSet(FlagSet&& flagset) = default;
    constexpr FlagSet(const FlagSet& flagset) = default;
    constexpr FlagSet(ENUM v) : storage(Mask(v)) {}
                                                        
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

    constexpr void Set(bool set, FlagSet flagSet) {
        auto mask = flagSet.storage;
        if (set)
            storage |= mask;
        else
            storage &= ~mask;
    }
    void Clear() { storage = 0; }

    constexpr FlagSet And(FlagSet other) const { return FlagSet(storage & other.storage); }
    constexpr FlagSet Or(FlagSet other) const { return FlagSet(storage | other.storage); }

    FlagSet& operator =(const FlagSet& value) = default;
    FlagSet& operator =(FlagSet&& value) = default;
    FlagSet& operator =(ENUM value) { storage = Mask(value); return *this; }

    constexpr FlagSet operator &(ENUM value) const { return And(FlagSet(value)); }
    constexpr FlagSet operator &(FlagSet value) const { return And(value); }
    constexpr FlagSet operator |(ENUM value) const { return Or(FlagSet(value)); }
    constexpr FlagSet operator |(FlagSet value) const { return Or(value); }

    //TODO: ops |= &= ! 

    operator bool() const { return storage != 0; }
    explicit operator BaseType() const { return storage; }
private:
    constexpr FlagSet(BaseType v) : storage(v) {}
    BaseType storage = 0;
};

//has interference with eigen lib
//template<typename T, typename = std::enable_if_t<detail::is_scoped_enum<T>::value>>
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
