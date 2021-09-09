#pragma once

#include "array_index_tree.hpp"
#include "component/global_matrix.hpp"
#include "component/local_matrix.hpp"
#include "component/parent.hpp"
#include "entity_manager_interface.hpp"
#include "generation_buffer.hpp"
#include "pointer_memory_array.hpp"
#include <cstdint>
#include <gsl/gsl>

namespace MoonGlare::ECS {

class EntityManager final : public iEntityManager {
public:
    using EntityIndex = uint32_t;

    using GenerationBufferType = GenerationBuffer<EntityIndex>;

    struct ArrayIndexTreeType final : public SparseArrayIndexTree<EntityIndex, ArrayIndexTreeType> {
        GenerationBufferType generation_buffer;
        iComponentArray *component_array = nullptr;

        // void SwapValues(ElementIndex a, ElementIndex b) {}

        void ClearArrays() { generation_buffer.Randomize(); }
        void InitElement(ElementIndex e, ElementIndex parent) {
            component_array->MarkIndexAsValid(e);

            component_array->AssignComponent<Component::GlobalMatrix>(
                e, Component::GlobalMatrix::Identity());
            component_array->AssignComponent<Component::LocalMatrix>(
                e, Component::LocalMatrix::Identity());

            if (parent != kInvalidIndex) {
                component_array->AssignComponent<Component::Parent>(e, Component::Parent{parent});
            }
        }
        void ReleaseElement(ElementIndex e, ElementIndex parent) {
            generation_buffer.Next(e);
            component_array->ReleaseIndex(e, true);
        }
    };

    union EntityFields {
        struct {
            EntityIndex generation;
            EntityIndex manager_index : 8;
            EntityIndex index : 24;
        };
        Entity handle;
    };
    static_assert(sizeof(Entity) == sizeof(EntityFields));

    static EntityFields SplitEntity(Entity e) {
        EntityFields ef;
        ef.handle = e;
        return ef;
    }
    Entity CreateEntity(EntityIndex index) {
        EntityFields ef;
        ef.index = index;
        ef.manager_index = manager_index;
        ef.generation = entity_tree.generation_buffer.Get(index);
        return ef.handle;
    }

    EntityManager(EntityManagerId eid, gsl::not_null<iComponentArray *> _component_array);
    ~EntityManager();

    Entity GetRootEntity() const override;
    Entity NewEntity() override;
    Entity NewEntity(Entity parent) override;
    bool NewEntities(size_t count, Entity *output_array) override;
    void Release(Entity entity) override;
    bool IsValid(Entity entity) const override;
    bool GetEntityParent(Entity entity, Entity &parent) const override;
    bool GetEntityIndex(Entity e, iComponentArray::IndexType &out) const override;
    iComponentArray *GetComponentArray() const override;

private:
    Entity root_entity;

    ArrayIndexTreeType entity_tree;
    const EntityManagerId manager_index;

    PointerMemory<ArrayIndexTreeType::ElementIndex> entity_tree_memory;
    PointerMemory<GenerationBufferType::Generation> generation_buffer_memory;
};

} // namespace MoonGlare::ECS
