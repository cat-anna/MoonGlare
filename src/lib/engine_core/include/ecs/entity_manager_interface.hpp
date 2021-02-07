
#pragma once

#include "component/component_common.hpp"
#include <cstdint>

namespace MoonGlare::ECS {

class iEntityManager {
public:
    virtual ~iEntityManager() = default;

    using IndexType = uint64_t;
    using ComponentFlags = Component::ComponentFlags;
    using ValidComponentsMap = Component::ValidComponentsMap;

    virtual void *CreateComponent(IndexType index, ComponentId c_id, bool call_default_constructor = true) = 0;
    virtual void *GetComponent(IndexType index, ComponentId c_id) const = 0;
    virtual void RemoveComponent(IndexType index, ComponentId c_id) = 0;
    virtual void RemoveAllComponents(IndexType index) = 0;
    virtual bool HasComponent(IndexType index, ComponentId c_id) const = 0;

    virtual void SetComponentActive(IndexType index, ComponentId c_id, bool active) = 0;
    virtual bool IsComponentActive(IndexType index, ComponentId c_id) = 0;

    virtual void MarkIndexAsValid(IndexType index) = 0;
    virtual void ReleaseIndex(IndexType index, bool destruct_components = true) = 0;
    virtual bool IsIndexValid(IndexType index) const = 0;
};

} // namespace MoonGlare::ECS
