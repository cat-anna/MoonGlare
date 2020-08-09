#pragma once

#include <array>

#include <boost/core/noncopyable.hpp>

#include <Memory/AlignedPtr.h>
#include <Memory/EnumArray.h>

#include <Foundation/FlagSet.h>
#include <Foundation/Tools/PerfViewClient.h>
#include <interface_map.h>

#include "ComponentInfo.h"
#include "Configuration.h"
#include "EntityArrayMapper.h"

namespace MoonGlare::Component {

class EntityManager;

class ComponentArray : private boost::noncopyable,
                       protected Tools::PerfView::PerfProducer {
  public:
    ComponentArray(InterfaceMap &ifaceMap);
    ~ComponentArray();

    using ComponentFlagSet = FlagSet<ComponentFlags>;
    static_assert(sizeof(ComponentFlagSet) == sizeof(uint8_t));

    template <typename T, typename... ARGS>
    T *Assign(Entity e, ARGS &&... args) {
        auto cci = ComponentInfo<T>::GetClassId();

        auto index = arrayMappers[cci]->GetIndex(e);
        if (index != ComponentIndex::Invalid) {
            auto memory = GetComponentMemory(index, cci);
            return reinterpret_cast<T *>(memory);
        }
        index = storageStatus[cci].Next();
        if (index == ComponentIndex::Invalid) {
            AddLogf(
                Warning,
                "Cannot assign component: Component buffer is full; ccid: %d ",
                (int)cci);
            return nullptr;
        }
        arrayMappers[cci]->SetIndex(e, index);

        auto &flags = GetComponentFlags(index, cci);
        flags = ComponentFlags::Active;

        auto memory = GetComponentMemory(index, cci);
        new (memory) T(std::forward<ARGS>(args)...);
        return reinterpret_cast<T *>(memory);
    }
    bool Create(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[cci]->GetIndex(e);
        if (index != ComponentIndex::Invalid) {
            AddLogf(Warning,
                    "Cannot create component: Component exists; ccid: %d ",
                    (int)cci);
            return true;
        }
        if (storageStatus[cci].Full()) {
            AddLogf(
                Warning,
                "Cannot create component: Component buffer is full; ccid: %d ",
                (int)cci);
            return false;
        }
        index = storageStatus[cci].Next();
        arrayMappers[cci]->SetIndex(e, index);
        componentOwner[cci][index] = e;

        auto &flags = GetComponentFlags(index, cci);
        flags = ComponentFlags::Active;

        auto memory = GetComponentMemory(index, cci);
        storageStatus[cci].info->constructor(memory);
        return true;
    }
    bool Load(Entity e, ComponentClassId cci, ComponentReader &reader) {
        auto index = arrayMappers[cci]->GetIndex(e);
        if (index != ComponentIndex::Invalid) {
            AddLogf(Warning,
                    "Cannot load component: Component exists; ccid: %d ",
                    (int)cci);
            return true;
        }
        if (storageStatus[cci].Full()) {
            AddLogf(
                Warning,
                "Cannot load component: Component buffer is full; ccid: %d ",
                (int)cci);
            return false;
        }
        index = storageStatus[cci].Next();
        arrayMappers[cci]->SetIndex(e, index);
        componentOwner[cci][index] = e;

        auto &flags = GetComponentFlags(index, cci);
        flags = ComponentFlags::Active;

        auto memory = GetComponentMemory(index, cci);
        storageStatus[cci].info->constructor(memory);
        storageStatus[cci].info->load(memory, reader, e);
        return memory;
    }

    template <typename T> T *Get(Entity e) {
        auto cci = ComponentInfo<T>::GetClassId();
        return reinterpret_cast<T *>(Get(e, cci));
    }
    void *Get(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid) {
            return nullptr;
        }

        auto memory = GetComponentMemory(index, cci);
        return memory;
    }

    template <typename T> bool HasComponent(Entity e) {
        auto cci = ComponentInfo<T>::GetClassId();
        return HasComponent(e, cci);
    }
    bool HasComponent(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[cci]->GetIndex(e);
        return index != ComponentIndex::Invalid;
    }

    template <typename T> void Remove(Entity e) {
        auto cci = ComponentInfo<T>::GetClassId();
        Remove(e, cci);
    }
    void Remove(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return;

        RemoveComponent(cci, e, index);
    }

    void RemoveAll(Entity e) {
        for (size_t i = 0; i < storageStatus.size(); ++i) {
            if (storageStatus[(ComponentClassId)i].info)
                Remove(e, static_cast<ComponentClassId>(i));
        }
    }

    template <typename COMPONENT, typename CALLABLE> void Visit(CALLABLE func) {
        static_assert(std::is_invocable_v<CALLABLE, Entity, COMPONENT &>);
        auto cci = ComponentInfo<COMPONENT>::GetClassId();
        const auto &ss = storageStatus[cci];
        for (size_t index = 0; index < ss.allocated; ++index) {
            auto &flags = GetComponentFlags((ComponentIndex)index, cci);
            if (flags & ComponentFlags::Active) {
                void *mem = GetComponentMemory((ComponentIndex)index, cci);
                Entity owner = componentOwner[cci][index];
                func(owner, *reinterpret_cast<COMPONENT *>(mem));
            }
        }
    }

    template <typename T> void SetActive(Entity e, bool v) {
        return SetActive(e, v, ComponentInfo<T>::GetClassId());
    }
    void SetActive(Entity e, bool v, ComponentClassId cci) {
        return SetFlags(e, v, ComponentFlags::Active, cci);
    }
    template <typename T>
    void SetFlags(Entity e, bool v, ComponentFlagSet flags) {
        return SetFlags(e, v, flags, ComponentInfo<T>::GetClassId());
    }
    void SetFlags(Entity e, bool v, ComponentFlagSet flags,
                  ComponentClassId cci) {
        auto index = arrayMappers[cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid) {
            return; // TODO: sth else?
        }
        GetComponentFlags(index, cci).Set(v, flags);
    }
    template <typename T> bool IsActive(Entity e) const {
        return IsActive(e, ComponentInfo<T>::GetClassId());
    }
    bool IsActive(Entity e, ComponentClassId cci) const {
        return TestFlags(e, ComponentFlags::Active, cci);
    }
    template <typename T>
    bool TestFlags(Entity e, ComponentFlagSet flags) const {
        return TestFlags(e, flags, ComponentInfo<T>::GetClassId());
    }
    bool TestFlags(Entity e, ComponentFlagSet flags,
                   ComponentClassId cci) const {
        auto index = arrayMappers[cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid) {
            return false; // TODO: sth else?
        }
        return GetComponentFlags(index, cci) & flags;
    }

    int PushToScript(iSubsystemManager *manager, Entity e, ComponentClassId cci,
                     lua_State *lua);

    void DumpStatus(const char *id = nullptr) const;

    void ReleaseComponents(ComponentClassId cci);
    void ReleaseAllComponents();

    void Step();
    void GCStep();
    void GCStep(ComponentClassId ccid);

  private:
    template <typename T>
    using PerComponentType =
        EnumArray<ComponentClassId, T, Configuration::MaxComponentTypes>;
    using ComponentMemory = Memory::aligned_array<uint8_t>;

    using ComponentFlagArray = std::unique_ptr<ComponentFlagSet[]>;
    using ComponentOwnerArray = std::unique_ptr<Entity[]>;

    EntityManager *entityManager = nullptr;
    void *__padding = nullptr;

    struct StorageStatus {
        size_t allocated;
        size_t capacity;
        size_t elementByteSize;
        const BaseComponentInfo::ComponentClassInfo *info;

        bool Full() const { return allocated >= capacity; }

        void Deallocate() { --allocated; }
        ComponentIndex Last() const {
            return static_cast<ComponentIndex>(allocated - 1);
        }
        ComponentIndex Next() {
            auto ci = static_cast<ComponentIndex>(allocated);
            if (ci == capacity)
                return ComponentIndex::Invalid;
            ++allocated;
            return ci;
        }
    };

    PerComponentType<std::unique_ptr<EntityArrayMapper<>>> arrayMappers;
    PerComponentType<StorageStatus> storageStatus;
    PerComponentType<ComponentMemory> componentMemory;
    PerComponentType<ComponentOwnerArray> componentOwner;
    PerComponentType<ComponentFlagArray> componentFlag;

    void *GetComponentMemory(ComponentIndex index, ComponentClassId cci) {
        return componentMemory[cci].get() +
               index * storageStatus[cci].elementByteSize;
    }
    const void *GetComponentMemory(ComponentIndex index,
                                   ComponentClassId cci) const {
        return componentMemory[cci].get() +
               index * storageStatus[cci].elementByteSize;
    }
    ComponentFlagSet &GetComponentFlags(ComponentIndex index,
                                        ComponentClassId cci) {
        return componentFlag[cci][index];
    }
    const ComponentFlagSet &GetComponentFlags(ComponentIndex index,
                                              ComponentClassId cci) const {
        return componentFlag[cci][index];
    }

    void SwapComponents(ComponentClassId cci, ComponentIndex a,
                        ComponentIndex b);
    void RemoveComponent(ComponentClassId cci, Entity e, ComponentIndex index);
};

} // namespace MoonGlare::Component
