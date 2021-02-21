
#include "ecs/entity_manager.hpp"
#include "ecs/ecs_configuration.hpp"
#include <build_configuration.hpp>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::ECS {

//----------------------------------------------------------------------------------

EntityManager::EntityManager(EntityManagerId eid, gsl::not_null<iComponentArray *> _component_array)
    : manager_index(eid) {
    entity_tree_memory.reset(ArrayIndexTreeType::GetMemorySize(kEntityLimit));
    generation_buffer_memory.reset(kEntityLimit);

    entity_tree.component_array = _component_array;
    entity_tree.SetMemory(kEntityLimit, entity_tree_memory);
    entity_tree.generation_buffer = GenerationBufferType(generation_buffer_memory);

    entity_tree.Clear();

    root_entity = CreateEntity(entity_tree.Allocate());

    assert(entity_tree.component_array);
}

EntityManager::~EntityManager() {
}

//----------------------------------------------------------------------------------

iComponentArray *EntityManager::GetComponentArray() const {
    return entity_tree.component_array;
}

bool EntityManager::GetEntityIndex(Entity e, iComponentArray::IndexType &out) const {
    if (!EntityManager::IsValid(e)) {
        return false;
    }
    auto ef = SplitEntity(e);
    out = ef.index;
    return true;
}

Entity EntityManager::GetRootEntity() const {
    return root_entity;
}

Entity EntityManager::NewEntity() {
    return EntityManager::NewEntity(GetRootEntity());
}

Entity EntityManager::NewEntity(Entity parent) {
    if (!EntityManager::IsValid(parent)) {
        AddLog(Warning, "Attempt to allocate entity with invalid parent");
        return 0;
    }

    const auto ef_parent = SplitEntity(parent);
    const auto e = CreateEntity(entity_tree.Allocate(ef_parent.index));
    // dispatcher->Send(MoonGlare::Component::EntityDestructedEvent{ parentEntity[index], entity });
    return e;
}

bool EntityManager::NewEntities(size_t count, Entity *output_array) {
    return false;
}

bool EntityManager::IsValid(Entity entity) const {
    auto ef = SplitEntity(entity);
    return ef.manager_index == manager_index && entity_tree.generation_buffer.Test(ef.index, ef.generation);
}

void EntityManager::Release(Entity entity) {
    if (!EntityManager::IsValid(entity) || entity == root_entity) {
        return;
    }

    auto ef = SplitEntity(entity);
    entity_tree.generation_buffer.Next(ef.index);
    entity_tree.ReleaseBranch(ef.index);
}

bool EntityManager::GetEntityParent(Entity entity, Entity &parent) const {
    if (!EntityManager::IsValid(entity)) {
        return false;
    }

    // TODO
    // parent = //

    return false;
}

} // namespace MoonGlare::ECS
