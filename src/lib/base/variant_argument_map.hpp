#pragma once

#include <any>
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <fmt/format.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace MoonGlare {

namespace detail {

template <typename T>
struct CastTo {
    template <typename A>
    static T from(const A &a) {
        throw std::runtime_error(fmt::format("Invalid cast attempt from {} to {}", typeid(A).name(), typeid(T).name()));
    }
};

} // namespace detail

struct VariantArgumentMap {
    //TODO: casting between types

    using VariantType = std::variant<nullptr_t, int64_t, double, bool, std::string, std::any>;
    using MapType = std::unordered_map<std::string, VariantType>;

    void set(const std::string &key, nullptr_t) { container[key] = nullptr; }
    void set(const std::string &key, int64_t value) { container[key] = value; }
    void set(const std::string &key, bool value) { container[key] = value; }
    void set(const std::string &key, double value) { container[key] = value; }
    void set(const std::string &key, std::string value) { container[key] = std::move(value); }
    void set(const std::string &key, std::any value) { container[key] = std::move(value); }

    template <typename WantedType>
    void get_to(WantedType &output, const std::string &key) const {
        output = get<WantedType>(key);
    }

    template <typename WantedType>
    void get_to(WantedType &output, const std::string &key, const WantedType &default_value) const {
        output = get<WantedType>(key, default_value);
    }

    template <typename WantedType>
    WantedType get(const std::string &key) const {
        return get<WantedType>(key, std::nullopt);
    }

    template <typename WantedType>
    WantedType get(const std::string &key, const WantedType &default_value) const {
        return get<WantedType>(key, std::optional(default_value));
    }

    template <typename WantedType>
    WantedType get(const std::string &key, const std::optional<WantedType> &default_value) const {
        auto it = container.find(key);
        if (it == container.end()) {
            if (default_value.has_value()) {
                return *default_value;
            } else {
                throw std::runtime_error("Key '" + key + "' not found");
            }
        }
        return CastVariant<WantedType>(it->second);
    }

    template <typename WantedType>
    static WantedType CastVariant(const VariantType &variant) {
        return std::visit(
            [](auto &&arg) -> WantedType {
                using StoredType = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<StoredType, WantedType>) {
                    return arg;
                } else if constexpr (std::is_same_v<nullptr_t, StoredType>) {
                    return {};
                } else if constexpr (std::is_same_v<std::any, StoredType>) {
                    return std::any_cast<WantedType>(arg);
                } else if constexpr (std::is_fundamental_v<StoredType> && std::is_fundamental_v<WantedType>) {
                    return static_cast<WantedType>(arg);
                } else if constexpr (std::is_fundamental_v<StoredType> && std::is_same_v<WantedType, std::string>) {
                    return boost::lexical_cast<std::string>(arg);
                } else if constexpr (std::is_fundamental_v<WantedType> && std::is_same_v<StoredType, std::string>) {
                    return boost::lexical_cast<WantedType>(arg);
                } else {
                    return detail::CastTo<WantedType>::from(arg);
                }
            },
            variant);
    }

    MapType container;
};

} // namespace MoonGlare
