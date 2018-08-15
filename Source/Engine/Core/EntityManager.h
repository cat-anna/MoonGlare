#pragma once

#include <Foundation/Component/EntityArrayMapper.h>
#include "../Configuration.h"
#include "nfCore.h"

namespace MoonGlare::Core {

class EntityManager final 
    : public Config::Current::DebugMemoryInterface {
public:
    template<class T> using Array = std::array<T, Configuration::Entity::IndexLimit>;
    template<class ... ARGS>
    using GenerationsAllocator_t = Space::Memory::StaticMultiAllocator<Configuration::Entity::IndexLimit, ARGS...>;
    using Generations_t = Space::Memory::GenerationRandomAllocator<GenerationsAllocator_t, Entity>;

    EntityManager();
    ~EntityManager();

    bool Initialize();
    bool Finalize();

    union EntityFlags {
        struct MapBits_t {
            bool m_Valid : 1;
        } m_Map;
        uint8_t m_UIntValue;
        void SetAll() { ClearAll(); m_UIntValue = ~m_UIntValue; }
        void ClearAll() { m_UIntValue = 0; }
        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UIntValue)), "Invalid Function map elements size!");
    };

    Entity GetRootEntity() { return m_Root; }

    bool Allocate(Entity &eout, std::string Name = std::string()) {
        return Allocate(GetRootEntity(), eout, std::move(Name));
    }
    bool Allocate(Entity parent, Entity &eout, std::string Name = std::string());
    bool Release(Entity entity);

    bool IsValid(Entity entity) const { return IsAllocated(entity); }

    bool SetEntityName(Entity e, std::string Name);
    bool GetEntityName(Entity e, std::string &Name);
    bool GetEntityName(Entity e, EntityNameHash &out);
    bool GetEntityName(Entity e, const std::string *&Name);
    bool GetRawFlags(Entity Owner, EntityFlags &flagsout);

    void RegisterEventSink(MoonGlare::Component::EventDispatcher* ed) { eventSinks.insert(ed); }
    void UnregisterEventSink(MoonGlare::Component::EventDispatcher* ed) { eventSinks.erase(ed); }
private: 
    Array<Entity> m_Parent;
    Array<EntityFlags> m_Flags;
    Array<EntityNameHash> m_NameHash;
    Array<std::string> m_Names;
    Generations_t m_Allocator;
    Entity m_Root;

    std::set<MoonGlare::Component::EventDispatcher*> eventSinks;

    bool IsAllocated(Entity entity) const {
        auto index = entity.GetIndex();
        return m_Allocator.IsHandleValid(entity) && m_Flags[index].m_Map.m_Valid;
    }
};

template<typename T = MoonGlare::Component::iSubsystem::ComponentIndex>
using EntityArrayMapper = MoonGlare::Component::EntityArrayMapper<T, Configuration::Entity::IndexLimit>;

} 
