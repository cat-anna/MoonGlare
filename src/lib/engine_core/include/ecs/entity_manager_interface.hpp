
#pragma once

#include "component/component_common.hpp"
#include <cstdint>

namespace MoonGlare::ECS {

class iComponentArray {
public:
    virtual ~iComponentArray() = default;

    using IndexType = uint64_t;
    using ComponentFlags = Component::ComponentFlags;
    using ValidComponentsMap = Component::ValidComponentsMap;
    using ComponentId = Component::ComponentId;

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

#ifdef WANTS_GTEST_MOCKS
struct ComponentArrayMock : public iComponentArray {
    MOCK_METHOD3(CreateComponent, void *(IndexType index, ComponentId c_id, bool call_default_constructor));
    MOCK_CONST_METHOD2(GetComponent, void *(IndexType index, ComponentId c_id));
    MOCK_METHOD2(RemoveComponent, void(IndexType index, ComponentId c_id));
    MOCK_METHOD1(RemoveAllComponents, void(IndexType index));
    MOCK_CONST_METHOD2(HasComponent, bool(IndexType index, ComponentId c_id));
    MOCK_METHOD3(SetComponentActive, void(IndexType index, ComponentId c_id, bool active));
    MOCK_METHOD2(IsComponentActive, bool(IndexType index, ComponentId c_id));
    MOCK_METHOD1(MarkIndexAsValid, void(IndexType index));
    MOCK_METHOD2(ReleaseIndex, void(IndexType index, bool destruct_components));
    MOCK_CONST_METHOD1(IsIndexValid, bool(IndexType index));
};
#endif

using Entity = uint64_t;

class iEntityManager {
public:
    virtual ~iEntityManager() = default;

    virtual Entity GetRootEntity() const = 0;
    virtual Entity NewEntity() = 0;
    virtual Entity NewEntity(Entity parent) = 0;
    virtual bool NewEntities(size_t count, Entity *output_array) = 0;
    virtual void Release(Entity entity) = 0;
    virtual bool IsValid(Entity entity) const = 0;
    virtual bool GetEntityParent(Entity entity, Entity &parent) const = 0;
};

} // namespace MoonGlare::ECS
