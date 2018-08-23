
#include "EntityManager.h"
#include <Foundation/Component/EntityEvents.h>

namespace MoonGlare::Component {

EntityManager::EntityManager() {
//#ifdef DEBUG
    generationbuffer.Fill(1);
//#else
    //generationbuffer.FillRandom(); //there is no mask!!
//#endif // DEBUG

    allocator.Clear();

    parentEntity.fill(Entity{});
    nameHash.fill(0);
    entityName.fill("");

    gcPosition = 0;

    uint32_t index;
    allocator.Allocate(index);
    rootEntity.SetIndex(index);
    rootEntity.SetGeneration(generationbuffer.Generation(index));
}

EntityManager::~EntityManager() {
    eventSinks.clear();
    rootEntity = Entity();
}

//------------------------------------------------------------------------------------------

void EntityManager::GCStep() {
    //TODO: GCStep
}

//------------------------------------------------------------------------------------------

bool EntityManager::Allocate(Entity parent, Entity &eout, std::string Name) {
    auto parentindex = parent.GetIndex();
    if (!IsAllocated(parent)) {
        AddLog(Error, "Parent entity is not valid!");
        return false;
    }

    uint32_t index;
    if (!allocator.Allocate(index)) {
        AddLog(Error, "Allocation failed!");
        __debugbreak();
        return false;
    }
    eout.SetIndex(index);
    eout.SetGeneration(generationbuffer.Generation(index));

    parentEntity[index] = parent;

    nameHash[index] = Space::Utils::MakeHash32(Name.c_str());
    entityName[index].swap(Name);

    for (auto *ev : eventSinks)
        ev->Queue(MoonGlare::Component::EntityCreatedEvent{parent, eout});

    return true;
}

bool EntityManager::Release(Entity entity) {
    if (!IsAllocated(entity)) {
        return false;
    }
    auto index = entity.GetIndex();

    generationbuffer.NewGeneration(index);
    allocator.Release(index);

    for (auto *ev : eventSinks)
        ev->Queue(MoonGlare::Component::EntityDestructedEvent{ parentEntity[index], entity });

    return true;
}

//---------------------------------------------------------------------------------------

bool EntityManager::GetParent(Entity e, Entity &outParent) {
    if (!IsAllocated(e))
        return false;

    auto index = e.GetIndex();
    outParent = parentEntity[index];

    return true;
}

bool EntityManager::SetEntityName(Entity e, std::string Name) {
    if (!IsAllocated(e))
        return false;

    auto index = e.GetIndex();
    nameHash[index] = Space::Utils::MakeHash32(Name.c_str());
    entityName[index].swap(Name);

    return true;
}

bool EntityManager::GetEntityName(Entity e, std::string &Name) {
    if (!IsAllocated(e))
        return false;

    auto index = e.GetIndex();
    Name = entityName[index];

    return true;
}

bool EntityManager::GetEntityName(Entity e, EntityNameHash & out) {
    if (!IsAllocated(e))
        return false;

    auto index = e.GetIndex();
    out = nameHash[index];

    return false;
}

bool EntityManager::GetEntityName(Entity e, const std::string *& Name) {
    if (!IsAllocated(e))
        return false;

    auto index = e.GetIndex();
    Name = &entityName[index];

    return true;
}

}
