#pragma once

#include <boost/core/noncopyable.hpp>
#include <array>

#include "Configuration.h"
#include "ComponentInfo.h"

#include "EntityArrayMapper.h"

namespace MoonGlare::Component {

class ComponentArray : private boost::noncopyable {
public:
    ComponentArray();
    ~ComponentArray();

    template<typename T, typename ... ARGS> 
    T* Assign(Entity e, ARGS&& ... args) {
        auto cci = ComponentInfo<T>::GetClassId();

        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        if (index != ComponentIndex::Invalid) {
            auto memory = GetComponentMemory(index, cci);
            return reinterpret_cast<T*>(memory);
        }
        if (storageStatus[(size_t)cci].Full()) {
            //TODO: ?
            return nullptr;
        }
        index = storageStatus[(size_t)cci].Next();
        arrayMappers[(size_t)cci]->SetIndex(e, index);

        auto memory = GetComponentMemory(index, cci);
        new(memory)T(std::forward<ARGS>(args)...);
        return reinterpret_cast<T*>(memory);
    }
    bool Create(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        if (index != ComponentIndex::Invalid) {
            return true;
        }
        if (storageStatus[(size_t)cci].Full()) {
            //TODO: ?
            return false;
        }
        index = storageStatus[(size_t)cci].Next();
        arrayMappers[(size_t)cci]->SetIndex(e, index);

        auto memory = GetComponentMemory(index, cci);
        storageStatus[(size_t)cci].info->constructor(memory);
        return true;
    }
    bool Load(Entity e, ComponentClassId cci, ComponentReader &reader) {
        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        if (index != ComponentIndex::Invalid) {
            return true;
        }
        if (storageStatus[(size_t)cci].Full()) {
            //TODO: ?
            return false;
        }
        index = storageStatus[(size_t)cci].Next();
        arrayMappers[(size_t)cci]->SetIndex(e, index);

        auto memory = GetComponentMemory(index, cci);
        storageStatus[(size_t)cci].info->constructor(memory);
        storageStatus[(size_t)cci].info->load(memory, reader, e);
        return memory;
    }

    template<typename T>
    T* Get(Entity e) {
        auto cci = ComponentInfo<T>::GetClassId(); 
        return reinterpret_cast<T*>(Get(e, cci));
    }
    void* Get(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid) {
            return nullptr;
        }

        auto memory = GetComponentMemory(index, cci);
        return memory;
    }

    template<typename T>
    bool HasComponent(Entity e) {
        auto cci = ComponentInfo<T>::GetClassId();
        return HasComponent(e, cci);
    }
    bool HasComponent(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        return index != ComponentIndex::Invalid;
    }

    template<typename T>
    void Remove(Entity e) {
        auto cci = ComponentInfo<T>::GetClassId();
        Remove(e, cci);
    }
    void Remove(Entity e, ComponentClassId cci) {
        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return;

        auto lastIndex = storageStatus[(size_t)cci].Last();
        SwapComponents(cci, index, lastIndex);
        
        auto mem = GetComponentMemory(lastIndex, cci);
        storageStatus[(size_t)cci].info->destructor(mem);
        arrayMappers[(size_t)cci]->ClearIndex(e);

        storageStatus[(size_t)cci].Deallocate();
    }

    void RemoveAll(Entity e) {
        for (size_t i = 0; i < storageStatus.size(); ++i) {
            if (!storageStatus[i].info)
                break;
            Remove(e, static_cast<ComponentClassId>(i));
        }
    }

    int PushToScript(Entity e, ComponentClassId cci, lua_State *lua) {
        auto index = arrayMappers[(size_t)cci]->GetIndex(e);
        if (index == ComponentIndex::Invalid) {
            return 0;
        }
        auto sp = storageStatus[(size_t)cci].info->scriptPush;
        if (!sp)
            return 0;
        sp(this, e, lua);
    }

    void DumpStatus(const char *id = nullptr) const;

    void ReleaseComponents(ComponentClassId cci);
    void ReleaseAllComponents();
private:
    template<typename T> using PerComponentType = std::array<T, Configuration::MaxComponentTypes>;
    using ComponentMemory = std::unique_ptr<char[]>;
    using ComponentOwner = std::unique_ptr<Entity[]>;

    struct StorageStatus {
        size_t allocated;
        size_t capacity;
        size_t elementByteSize;
        const BaseComponentInfo::ComponentClassInfo* info;

        bool Full() const { return allocated >= capacity; }

        void Deallocate() { --allocated; }
        ComponentIndex Last() const { return  static_cast<ComponentIndex>(allocated); } 
        ComponentIndex Next() {
            auto ci = static_cast<ComponentIndex>(allocated);
            ++allocated;
            return ci;
        }
    };

    PerComponentType<std::unique_ptr<EntityArrayMapper<>>> arrayMappers;
    PerComponentType<StorageStatus> storageStatus;
    PerComponentType<ComponentMemory> componentMemory;
    PerComponentType<ComponentOwner> componentOwner;

    void* GetComponentMemory(ComponentIndex index, ComponentClassId cci) {
        return componentMemory[(size_t)cci].get() + index * storageStatus[(size_t)cci].elementByteSize;        
    }

    void SwapComponents(ComponentClassId cci, ComponentIndex a, ComponentIndex b) {
        auto aE = componentOwner[(size_t)cci][a];
        auto aMem = GetComponentMemory(a, cci);

        auto bE = componentOwner[(size_t)cci][b];
        auto bMem = GetComponentMemory(b, cci);

        arrayMappers[(size_t)cci]->SetIndex(aE, b);
        arrayMappers[(size_t)cci]->SetIndex(bE, a);
        componentOwner[(size_t)cci][a] = bE;
        componentOwner[(size_t)cci][b] = aE;

        storageStatus[(size_t)cci].info->swap(bMem, aMem);
    }
};

}
