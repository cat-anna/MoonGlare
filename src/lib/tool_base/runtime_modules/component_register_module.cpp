#include "component_register_module.hpp"
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Tools::RuntineModules {

using ComponentInfo = ComponentRegisterModule::ComponentInfo;

ComponentRegisterModule::ComponentRegisterModule(SharedModuleManager modmgr) : iModule(modmgr) {
}

bool ComponentRegisterModule::PostInit() {
    type_provider = GetModuleManager()->QueryModule<iEditableTypeProvider>();
    return true;
}

void ComponentRegisterModule::RegisterComponent(std::shared_ptr<ComponentInfo> info) {
    AddLogf(Info, "Registered component %02x:%s:%s (%d bytes)", info->id, info->name.c_str(),
            info->editable_type_name.c_str(), info->byte_size);
    info_vector.emplace_back(std::move(info));
}

std::vector<std::shared_ptr<ComponentInfo>> ComponentRegisterModule::GetComponents() const {
    return info_vector;
}

std::shared_ptr<ComponentInfo> ComponentRegisterModule::GetComponentInfo(MoonGlare::Component::ComponentId id) const {
    for (auto &item : info_vector) {
        if (item->id == id) {
            return item;
        }
    }
    throw std::runtime_error(fmt::format("Component with id {:0x} is not registered", id));
}

std::shared_ptr<ComponentInfo> ComponentRegisterModule::GetComponentInfo(const std::string &component_name) const {
    for (auto &item : info_vector) {
        if ( //item->name == component_name ||
            item->editable_type_name == component_name) {
            return item;
        }
    }
    throw std::runtime_error(fmt::format("Component with name {} is not registered", component_name));
}

std::shared_ptr<iEditableType> ComponentRegisterModule::CreateComponentData(std::shared_ptr<ComponentInfo> info) const {
    auto provider = type_provider.lock();
    if (!provider) {
        throw std::runtime_error("Provider pointer expired");
    }

    return provider->CreateObject(info->editable_type_name);
}

} // namespace MoonGlare::Tools::RuntineModules
