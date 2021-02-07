#pragma once

#include "component/component_common.hpp"
#include "component_info.hpp"
#include "component_interface.hpp"
#include "system_info.hpp"
#include "system_interface.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <orbit_logger.h>
#include <stdexcept>
#include <vector>

namespace MoonGlare::ECS {

class ECSRegister : public iSystemRegister, public iComponentRegister {
public:
    //iSystemRegister
    std::unique_ptr<iSystem> CreateSystem(SystemId sys_id, const SystemCreateInfo &data,
                                          const nlohmann::json &config_node) const override;
    std::vector<BaseSystemInfo *> GetRegisteredSystemsInfo() const override;
    std::vector<std::unique_ptr<iSystem>> LoadSystemConfiguration(const SystemCreateInfo &data,
                                                                  const nlohmann::json &config_node) const override;

    template <typename T>
    void RegisterSystem() {
        if (auto it = known_systems.find(T::kSystemId); it != known_systems.end()) {
            auto msg = fmt::format("System with id {} already exists (adding '{}'; existing '{}')", T::kSystemId,
                                   T::kSystemName, it->second->GetName());
            AddLog(Error, msg);
            throw std::runtime_error(msg);
        }
        known_systems[T::kSystemId] = std::make_unique<SystemInfo<T>>();
    }

    //iComponentRegister
    std::vector<BaseComponentInfo *> GetRegisteredComponentsInfo() const override;
    std::unordered_map<ComponentId, BaseComponentInfo *> GetRegisteredComponentsMap() const override;
    const ComponentMemoryInfo *GetComponentMemoryInfo() const override;

    template <typename T>
    void RegisterComponent() {
        if (auto it = known_components.find(T::kComponentId); it != known_components.end()) {
            auto msg = fmt::format("Component with id {} already exists (adding '{}'; existing '{}')", T::kComponentId,
                                   T::kComponentName, it->second->GetName());
            AddLog(Error, msg);
            throw std::runtime_error(msg);
        }
        if (T::kComponentId > Component::kMaxComponents) {
            auto msg = fmt::format("Component with id {}:{} has too large id", T::kComponentId, T::kComponentName);
            AddLog(Error, msg);
            throw std::runtime_error(msg);
        }
        known_components[T::kComponentId] = std::make_unique<ComponentInfo<T>>();
        ResetMemoryInfo();
    }

    //Other

    void Dump();

    ECSRegister();
    ~ECSRegister();

private:
    std::unordered_map<SystemId, std::unique_ptr<BaseSystemInfo>> known_systems;
    std::unordered_map<ComponentId, std::unique_ptr<BaseComponentInfo>> known_components;

    mutable std::unique_ptr<ComponentMemoryInfo> component_memory_info;
    void ResetMemoryInfo();
};

} // namespace MoonGlare::ECS
