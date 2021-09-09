#pragma once

#include <array>
#include <fmt/format.h>
#include <math/vector.hpp>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>

namespace Eigen {

template <typename T, size_t S>
void to_json(nlohmann::json &j, const Eigen::Matrix<T, S, 1> &p) {
    const std::array<T, S> *a = reinterpret_cast<const std::array<T, S> *>(&p);
    j = *a;
}

template <typename T, size_t S>
void from_json(const nlohmann::json &j, Eigen::Matrix<T, S, 1> &p) {
    std::array<T, S> *a = reinterpret_cast<std::array<T, S> *>(&p);
    if (j.is_number()) {
        a->fill(j.get<T>());
    } else {
        *a = j.get<std::array<T, S>>();
    }
}

//----------------------------------------------------------------------------------

void to_json(nlohmann::json &j, const Quaternionf &p) {
    auto *a = reinterpret_cast<const std::array<float, 4> *>(&p);
    j = *a;
}

void from_json(const nlohmann::json &j, Quaternionf &p) {
    auto *a = reinterpret_cast<std::array<float, 4> *>(&p);
    if (j.is_number()) {
        a->fill(j.get<float>());
    } else {
        *a = j.get<std::array<float, 4>>();
    }
}

} // namespace Eigen
