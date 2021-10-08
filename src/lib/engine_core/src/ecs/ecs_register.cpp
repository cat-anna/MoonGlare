#include "ecs/ecs_register.hpp"
#include "build_configuration.hpp"
#include "ecs/component_array.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::ECS {

//----------------------------------------------------------------------------------

ECSRegister::ECSRegister() {
}

ECSRegister::~ECSRegister() {
}

//----------------------------------------------------------------------------------
//iSystemRegister

std::unique_ptr<iSystem> ECSRegister::CreateSystem(SystemId sys_id, const SystemCreateInfo &data,
                                                   const nlohmann::json &config_node) const {
    auto system_it = known_systems.find(sys_id);
    if (system_it == known_systems.end()) {
        AddLog(Error, fmt::format("Unknown system with id {}", sys_id));
        return nullptr;
    }

    return system_it->second->MakeInstance(data, config_node);
}

std::vector<BaseSystemInfo *> ECSRegister::GetRegisteredSystemsInfo() const {
    std::vector<BaseSystemInfo *> r;
    for (auto &[id, uptr] : known_systems) {
        r.emplace_back(uptr.get());
    }
    std::sort(r.begin(), r.end(), [](auto a, auto b) { return a->GetId() < b->GetId(); });
    return r;
}

std::vector<std::unique_ptr<iSystem>>
ECSRegister::LoadSystemConfiguration(const SystemCreateInfo &data,
                                     const nlohmann::json &config_node) const {
    std::vector<std::unique_ptr<iSystem>> r;
    r.reserve(kMaxSystems);
    for (auto &entry : config_node) {
        //TODO: share it with editor!
        auto id = entry.at("id").get<SystemId>();
        auto system_config_node = entry.at("configuration");
        auto ptr = CreateSystem(id, data, system_config_node);

        if (!ptr) {
            AddLog(Error, fmt::format("Failed to create system with id {}", id));
            continue;
        }

        auto system_order = ptr->GetSystemInfo().order;
        auto insert_point = std::find_if(r.begin(), r.end(), [&](const auto &a) {
            return a->GetSystemInfo().order > system_order;
        });

        r.insert(insert_point, std::move(ptr));
    }
    r.shrink_to_fit();

    if constexpr (kDebugDumpEnabled) {
        AddLog(Debug, "Created systems:");
        int index = 0;
        for (const auto &item : r) {
            const auto id = item->GetSystemInfo().id;
            const auto &system = known_systems.at(id);
            AddLog(Debug, fmt::format("{:02}. {:02x}:{:20} [{}] ", index, system->GetId(),
                                      system->GetName(), to_string(system->GetDetails())));
            ++index;
        }
    }

    return r;
}

//----------------------------------------------------------------------------------

const BaseComponentInfo *ECSRegister::GetComponentsInfo(ComponentId c_id) const {
    auto it = known_components.find(c_id);
    if (it == known_components.end()) {
        AddLog(Error, fmt::format("Unknown component with id {}", c_id));
        throw std::runtime_error("Unknown component");
    }
    return it->second.get();
}

std::vector<BaseComponentInfo *> ECSRegister::GetRegisteredComponentsInfo() const {
    std::vector<BaseComponentInfo *> r;
    for (auto &[id, uptr] : known_components) {
        r.emplace_back(uptr.get());
    }
    std::sort(r.begin(), r.end(), [](auto a, auto b) { return a->GetId() < b->GetId(); });
    return r;
}

std::unordered_map<ComponentId, BaseComponentInfo *>
ECSRegister::GetRegisteredComponentsMap() const {
    std::unordered_map<ComponentId, BaseComponentInfo *> r;
    for (auto &[id, uptr] : known_components) {
        r.emplace(id, uptr.get());
    }
    return r;
}

void ECSRegister::ResetMemoryInfo() {
    component_memory_info.reset();
}

const ComponentMemoryInfo *ECSRegister::GetComponentMemoryInfo() const {
    if (!component_memory_info) {
        component_memory_info = std::make_unique<ComponentMemoryInfo>(
            ComponentMemoryInfo::CalculateOffsets((iComponentRegister *)this));
    }
    return component_memory_info.get();
}

//----------------------------------------------------------------------------------

void ECSRegister::Dump() {
    for (auto ptr : GetRegisteredSystemsInfo()) {
        AddLog(Resources, fmt::format("Known system: {:02x}:{:20} [{}] ", ptr->GetId(),
                                      ptr->GetName(), to_string(ptr->GetDetails())));
    }
    for (auto ptr : GetRegisteredComponentsInfo()) {
        AddLog(Resources, fmt::format("Known component: {:02x}:{:20} [{}] ", ptr->GetId(),
                                      ptr->GetName(), to_string(ptr->GetDetails())));
    }
}

} // namespace MoonGlare::ECS
