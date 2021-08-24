
#pragma once

#include "component/component_common.hpp"
#include <cstdint>
#include <tuple>
#include <vector>

namespace MoonGlare::ECS {

using Entity = uint64_t;         //TODO: change to enum
using EntityManagerId = uint8_t; //TODO: change to enum

class iComponentArray {
public:
    virtual ~iComponentArray() = default;

    using IndexType = uint64_t; //TODO: change to enum
    using ComponentFlags = Component::ComponentFlags;
    using ValidComponentsMap = Component::ValidComponentsMap;
    using ComponentId = Component::ComponentId;

    virtual void *CreateComponent(IndexType index, ComponentId c_id,
                                  bool call_default_constructor = true) = 0;
    virtual void *GetComponent(IndexType index, ComponentId c_id) const = 0;
    virtual void RemoveComponent(IndexType index, ComponentId c_id) = 0;
    virtual void RemoveAllComponents(IndexType index) = 0;
    virtual bool HasComponent(IndexType index, ComponentId c_id) const = 0;

    virtual void SetComponentActive(IndexType index, ComponentId c_id, bool active) = 0;
    virtual bool IsComponentActive(IndexType index, ComponentId c_id) = 0;

    virtual void MarkIndexAsValid(IndexType index) = 0;
    virtual void ReleaseIndex(IndexType index, bool destruct_components = true) = 0;
    virtual bool IsIndexValid(IndexType index) const = 0;

    //template wrappers

    template <typename T>
    bool HasComponent(IndexType index) {
        return HasComponent(index, T::kComponentId);
    }

    template <typename T>
    bool IsComponentActive(IndexType index) {
        return IsComponentActive(index, T::kComponentId);
    }

    template <typename T, typename... ARGS>
    T *AssignComponent(IndexType index, ARGS &&...args) {
        auto memory = CreateComponent(index, T::kComponentId, false);
        if (memory != nullptr) {
            return new (memory) T(std::forward<ARGS>(args)...);
        }
        return nullptr;
    }

    template <typename T>
    T *GetComponent(IndexType index) {
        return reinterpret_cast<T *>(GetComponent(index, T::kComponentId));
    }

    template <typename T>
    void RemoveComponent(IndexType index) {
        RemoveComponent(index, T::kComponentId);
    }

    template <typename T>
    void SetComponentActive(IndexType index, bool active) {
        SetComponentActive(index, T::kComponentId, active);
    }
};

#ifdef WANTS_GTEST_MOCKS
struct ComponentArrayMock : public iComponentArray {
    MOCK_METHOD3(CreateComponent,
                 void *(IndexType index, ComponentId c_id, bool call_default_constructor));
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
    virtual bool GetEntityIndex(Entity e, iComponentArray::IndexType &out) const = 0;

    virtual iComponentArray *GetComponentArray() const = 0;
    // virtual EntityManagerId GetManagerId() const = 0;
};

#ifdef WANTS_GTEST_MOCKS
struct EntityManagerMock : public iEntityManager {
    MOCK_METHOD0(NewEntity, Entity());
    MOCK_METHOD1(NewEntity, Entity(Entity parent));
    MOCK_METHOD2(NewEntities, bool(size_t count, Entity *output_array));
    MOCK_METHOD1(Release, void(Entity entity));
    MOCK_CONST_METHOD0(GetRootEntity, Entity());
    MOCK_CONST_METHOD1(IsValid, bool(Entity entity));
    MOCK_CONST_METHOD2(GetEntityParent, bool(Entity entity, Entity &parent));
    MOCK_CONST_METHOD2(GetEntityIndex, bool(Entity, iComponentArray::IndexType &));
    MOCK_CONST_METHOD0(GetComponentArray, iComponentArray *());

    const uint64_t kMask = 0xFADE0000'00000000;
    Entity root_entity = kMask;

    std::vector<std::tuple<Entity, Entity>> allocated_children;

    iComponentArray *component_array = nullptr;

    EntityManagerMock() {
        using namespace ::testing;
        EXPECT_CALL(*this, GetComponentArray()).WillRepeatedly(Invoke([this]() {
            return this->component_array;
        }));
        EXPECT_CALL(*this, GetRootEntity()).WillRepeatedly(Invoke([this]() {
            return this->root_entity;
        }));
        EXPECT_CALL(*this, NewEntity(_)).WillRepeatedly(Invoke([this](auto parent) {
            auto child = allocated_children.size() + root_entity + 1;
            allocated_children.emplace_back(child, parent);
            return child;
        }));
        EXPECT_CALL(*this, GetEntityIndex(_, _)).WillRepeatedly(Invoke([this](auto e, auto &data) {
            if ((e & kMask) != kMask) {
                return false;
            }
            data = e & ~kMask;
            return true;
        }));
    }
};
#endif

} // namespace MoonGlare::ECS
