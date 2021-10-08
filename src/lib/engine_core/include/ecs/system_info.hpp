#pragma once

#include "system_interface.hpp"
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>

namespace MoonGlare::ECS {

namespace detail {

template <typename T, typename Tuple>
struct HasFlag;

template <typename T, typename... Us>
struct HasFlag<T, std::tuple<Us...>> : public std::disjunction<std::is_same<T, Us>...> {};

} // namespace detail

class BaseSystemInfo {
public:
    struct SystemDetails {
        bool has_config;
        bool stepable;
        SystemOrder order;
    };

    virtual SystemDetails GetDetails() const = 0;
    virtual SystemId GetId() const = 0;
    virtual SystemOrder GetOrder() const = 0;
    virtual const std::string &GetName() const = 0;
    virtual const std::type_info &GetTypeInfo() const = 0;
    virtual std::unique_ptr<iSystem> MakeInstance(const SystemCreateInfo &data,
                                                  const nlohmann::json &config_node) const = 0;
};

inline std::string to_string(const BaseSystemInfo::SystemDetails &sd) {
    return fmt::format("has_config={},stepable={},order={}", sd.has_config, sd.stepable, sd.order);
}

template <class T>
struct SystemInfo : public BaseSystemInfo {
    SystemDetails GetDetails() const override { return details; }
    SystemId GetId() const override { return T::kSystemId; }
    SystemOrder GetOrder() const override { return T::kOrder; };
    const std::string &GetName() const override { return name; }
    const std::type_info &GetTypeInfo() const override { return typeid(T); }

    std::unique_ptr<iSystem> MakeInstance(const SystemCreateInfo &data,
                                          const nlohmann::json &config_node) const override {
        return std::make_unique<T>(data, config_node.get<T::SystemConfiguration>());
    }

private:
    std::string name = T::kSystemName;
    const SystemDetails details{
        .has_config = !std::is_same_v<T::SystemConfiguration, BaseSystemConfig>,
        .stepable = T::kStepable,
        .order = T::kOrder,
    };
};

} // namespace MoonGlare::ECS
