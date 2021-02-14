#pragma once

#include "array_index_tree.hpp"
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
        void InitElement(ElementIndex e, ElementIndex parent) { component_array->MarkIndexAsValid(e); }
        void ReleaseElement(ElementIndex e, ElementIndex parent) {
            generation_buffer.Next(e);
            component_array->ReleaseIndex(e, true);
        }
    };

    struct EntityFields {
        EntityIndex generation;
        EntityIndex index;
    };
    static_assert(sizeof(Entity) == sizeof(EntityFields));

    static EntityFields SplitEntity(Entity e) { return {e >> 32, e & 0xFFFFFFFF}; }
    static Entity MakeEntity(EntityIndex index, EntityIndex generation) {
        return static_cast<Entity>(generation) << 32 | index;
    }
    static Entity MakeEntity(EntityFields ef) { return MakeEntity(ef.index, ef.generation); }

    EntityManager(gsl::not_null<iComponentArray *> _component_array);
    ~EntityManager();

    Entity GetRootEntity() const override;
    Entity NewEntity() override;
    Entity NewEntity(Entity parent) override;
    bool NewEntities(size_t count, Entity *output_array) override;
    void Release(Entity entity) override;
    bool IsValid(Entity entity) const override;
    bool GetEntityParent(Entity entity, Entity &parent) const override;

private:
    Entity root_entity;

    ArrayIndexTreeType entity_tree;

    PointerMemory<ArrayIndexTreeType::ElementIndex> entity_tree_memory;
    PointerMemory<GenerationBufferType::Generation> generation_buffer_memory;
};

} // namespace MoonGlare::ECS
