
#pragma once

#include "component/component_common.hpp"
#include <unordered_map>
#include <vector>

namespace MoonGlare::ECS {

class BaseComponentInfo;
struct ComponentMemoryInfo;

class iComponentRegister {
public:
    virtual ~iComponentRegister() = default;

    using ComponentId = Component::ComponentId;

    virtual const BaseComponentInfo *GetComponentsInfo(ComponentId c_id) const = 0;
    virtual std::vector<BaseComponentInfo *> GetRegisteredComponentsInfo() const = 0;
    virtual std::unordered_map<ComponentId, BaseComponentInfo *> GetRegisteredComponentsMap() const = 0;
    virtual const ComponentMemoryInfo *GetComponentMemoryInfo() const = 0;
};

} // namespace MoonGlare::ECS