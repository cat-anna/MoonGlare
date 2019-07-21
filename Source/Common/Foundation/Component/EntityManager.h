#pragma once

#include <boost/core/noncopyable.hpp>

#include <Memory/StaticGenerationBuffer.h>
#include <Memory/StaticIndexQueue.h>
#include <Foundation/InterfaceMap.h>

#include "Configuration.h"
#include "Entity.h"

#include "EventDispatcher.h"

namespace MoonGlare::Component {

class EntityManager final : private boost::noncopyable {
public:
    template<class T> using Array = std::array<T, Component::Configuration::EntityLimit>;

    EntityManager(InterfaceMap &ifaceMap);
    ~EntityManager();

    void GCStep();

    Entity GetRootEntity() { return rootEntity; }

    bool Allocate(Entity &eout, std::string Name = std::string()) { return Allocate(GetRootEntity(), eout, std::move(Name)); }
    bool Allocate(Entity parent, Entity &eout, std::string Name = std::string());
    bool Release(Entity entity);

    bool IsValid(Entity entity) const { return IsAllocated(entity); }

    bool SetEntityName(Entity e, std::string Name);
    bool GetEntityName(Entity e, std::string &Name);
    bool GetEntityName(Entity e, EntityNameHash &out);
    bool GetEntityName(Entity e, const std::string *&Name);
    bool GetParent(Entity e, Entity &outParent);

private: 
    Entity rootEntity;
    uint32_t gcPosition;
    EventDispatcher *dispatcher = nullptr;

    Memory::GenerationBuffer<Entity::Generation_t, Configuration::EntityLimit> generationbuffer;
    Memory::StaticIndexQueue<Entity::Index_t, Configuration::EntityLimit> allocator;

    Array<Entity> parentEntity;
    Array<EntityNameHash> nameHash;
    Array<std::string> entityName;

    bool IsAllocated(Entity entity) const {
        auto index = entity.GetIndex();
        auto generation = entity.GetGeneration();
        return generation == generationbuffer.Generation(index);
    }
};

} 
