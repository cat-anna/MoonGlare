
#include "ecs/entity_manager.hpp"
#include "ecs/ecs_configuration.hpp"
#include <build_configuration.hpp>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::ECS {

//----------------------------------------------------------------------------------

EntityManager::EntityManager(gsl::not_null<iComponentArray *> _component_array) {
    entity_tree_memory.reset(ArrayIndexTreeType::GetMemorySize(kEntityLimit));
    generation_buffer_memory.reset(kEntityLimit);

    entity_tree.component_array = _component_array;
    entity_tree.SetMemory(kEntityLimit, entity_tree_memory);
    entity_tree.generation_buffer = GenerationBufferType(generation_buffer_memory);

    entity_tree.Clear();

    EntityFields ef{};
    ef.index = entity_tree.Allocate();
    ef.generation = entity_tree.generation_buffer.Next(ef.index);
    AddLog(Warning, fmt::format("Root entity {}:{}", ef.index, ef.generation));
    root_entity = MakeEntity(ef);

    assert(entity_tree.component_array);
}

EntityManager::~EntityManager() {
}

//----------------------------------------------------------------------------------

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
    EntityFields ef{};
    ef.index = entity_tree.Allocate(ef_parent.index);
    ef.generation = entity_tree.generation_buffer.Get(ef.index);

    const auto e = MakeEntity(ef);
    // dispatcher->Send(MoonGlare::Component::EntityDestructedEvent{ parentEntity[index], entity });
    return e;
}

bool EntityManager::NewEntities(size_t count, Entity *output_array) {
    return false;
}

bool EntityManager::IsValid(Entity entity) const {
    auto ef = SplitEntity(entity);
    return entity_tree.generation_buffer.Test(ef.index, ef.generation);
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
