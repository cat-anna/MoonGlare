#pragma once

#include <array>
#include <attribute_map_builder.hpp>
#include <fmt/format.h>
#include <math/vector.hpp>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <typeinfo>

namespace Eigen {

template <typename T, size_t S>
auto GetTypeInfo(Eigen::Matrix<T, S, 1> *) {
    auto t = MoonGlare::AttributeMapBuilder<Eigen::Matrix<T, S, 1>>::Start(
        fmt::format("Eigen::Matrix<{}, {}, 1>", typeid(T).name(), S));
    using O = Eigen::Matrix<T, S, 1>;

    union {
        uint32_t raw_ptr;
        T O::*ptr;
    } u;
    static_assert(sizeof(u.raw_ptr) == sizeof(u.ptr));

    static const std::string field_names[] = {"x", "y", "z", "w"};
    for (uint32_t i = 0; i < S; ++i) {
        u.raw_ptr = i * sizeof(T);
        t->AddField(field_names[i], u.ptr);
    }

    return t;
}

//----------------------------------------------------------------------------------

auto GetTypeInfo(Quaternionf *) {
    auto t = MoonGlare::AttributeMapBuilder<Quaternionf>::Start("Quaternionf");

    union {
        uint32_t raw_ptr;
        float Quaternionf::*ptr;
    } u;
    static_assert(sizeof(u.raw_ptr) == sizeof(u.ptr));

    static const std::string field_names[] = {"x", "y", "z", "w"};
    for (uint32_t i = 0; i < 4; ++i) {
        u.raw_ptr = i * sizeof(float);
        t->AddField(field_names[i], u.ptr);
    }

    return t;
}

} // namespace Eigen
