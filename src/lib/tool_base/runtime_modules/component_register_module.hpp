#pragma once

#include "component_register.hpp"
#include <editable_type_provider.hpp>
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class ComponentRegisterModule : public iModule, public iComponentRegister {
public:
    ComponentRegisterModule(SharedModuleManager modmgr);

    void RegisterComponent(std::shared_ptr<ComponentInfo> info) override;
    std::vector<std::shared_ptr<ComponentInfo>> GetComponents() const override;
    std::shared_ptr<ComponentInfo> GetComponentInfo(MoonGlare::Component::ComponentId id) const override;
    std::shared_ptr<ComponentInfo> GetComponentInfo(const std::string &component_name) const override;
    std::shared_ptr<iEditableType> CreateComponentData(std::shared_ptr<ComponentInfo> info) const override;

    bool PostInit() override;

protected:
    std::vector<std::shared_ptr<ComponentInfo>> info_vector;
    std::weak_ptr<iEditableTypeProvider> type_provider;
};

} // namespace MoonGlare::Tools::RuntineModules
