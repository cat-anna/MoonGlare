#pragma once

#include <array>
#include <bitset>

#include "Configuration.h"
#include "ComponentInfo.h"

namespace MoonGlare::Component {

class ComponentArray {
public:
    ComponentArray();
    ~ComponentArray();

    using Index = uint16_t;
    using ComponentTypeBitSet = Configuration::ComponentTypeBitSet;

    template<typename ... ARGS>
    static ComponentTypeBitSet MakeBitSet() { return (... | (1 << ComponentClassIdValue<ARGS>)); }

    template<typename T> T* QuerryComponent(Index index) {
        auto bitmask = MakeBitSet<T>();
        if ((componentValidArray[index] & bitmask) != bitmask) {
            __debugbreak();
            return nullptr;
        }
        T* ptr = GetComponentMemory<T>(index);
        return ptr;
    }
    template<typename T> T& GetComponent(Index index) {
        T* ptr = QuerryComponent<T>(index);
        if (!ptr) {
            __debugbreak();
            throw std::runtime_error("Component does not exists!");
        }
        return *reinterpret_cast<T*>(ptr);
    }
    //template<typename ... T> std::tuple<T&...> GetComponents(Index index) {
        //__debugbreak();
        //throw false;
    //}
    bool CreateComponent(Index index, ComponentClassId cci) {
        auto bitmask = 1 << cci;
        if ((componentValidArray[index] & bitmask) != 0)
            return true;

        auto &info = BaseComponentInfo::GetComponentTypeInfo(cci);
        void *ptr = GetComponentPointer(index, cci);
        if (!info.pod) {
            info.constructor(ptr);
        }
        componentValidArray[index] |= bitmask;
        return true;
    }
    template<typename T, typename ... ARGS> T& AssignComponent(Index index, ARGS&& ... args) {
        auto bitmask = MakeBitSet<T>();
        T* ptr = GetComponentMemory<T>(index);
        if ((componentValidArray[index] & bitmask) == 0) {
            new (ptr) T (std::forward<ARGS>(args)...);
            componentValidArray[index] |= bitmask;
        }
        return *reinterpret_cast<T*>(ptr);
    }
    template<typename T> void RemoveComponent(Index index) {
        auto bitmask = MakeBitSet<T>();
        if ((componentValidArray[index] & bitmask) != 0) {
            T* ptr = GetComponentMemory<T>(index);
            ptr->~T();
            componentValidArray[index] &= ~bitmask;
        }
    }
    template<typename ... T> bool HasComponent(Index index) {
        auto bitmask = MakeBitSet<T...>();
        return (componentValidArray[index] & bitmask) == bitmask;
    }
    bool HasComponent(Index index, ComponentClassId cci) {
        auto bitmask = 1 << cci;
        return (componentValidArray[index] & bitmask) == bitmask;
    }
    template<typename T, typename F> void ForEach(F func) {
        auto bitmask = MakeBitSet<T>();
        for (uint16_t pageIndex = 0; pageIndex < componentPageArray.size(); ++pageIndex) {
            if (!pageMemory[pageIndex])
                continue;
            for (uint16_t offset = 0; offset < Configuration::ComponentsPerPage; ++offset) {
                uint32_t index = pageIndex * Configuration::ComponentsPerPage + offset;
                if ((componentValidArray[index] & bitmask) != bitmask)
                    continue;

                T *t = GetComponentMemory<T>(pageIndex, offset);
                func(index, *t);
            }
        }
    }

    void* GetComponentPointer(Index index, ComponentClassId cci) {
        auto bitmask = 1 << cci;
        if ((componentValidArray[index] & bitmask) == 0)
            return nullptr;
        return GetComponentMemory(index, cci);
    }

    void ReleaseAllComponents();
    void ReleaseComponents(Index index);
private:
    template<typename T> using PerPageArray = std::array<T, Configuration::MaxComponentPages>;
    template<typename T> using PerComponentArray = std::array<T, Configuration::MaxComponentTypes>;
    template<typename T> using PerInstanceArray = std::array<T, Configuration::MaxComponent>;
    using PageMemoryPtr = std::unique_ptr<char[]>;
    using ComponentPage = PerComponentArray<char*>;

    PerPageArray<ComponentPage> componentPageArray;
    PerInstanceArray<ComponentTypeBitSet> componentValidArray;
    PerPageArray<PageMemoryPtr> pageMemory;

    bool AllocatePage(unsigned pageIndex);

    void* GetComponentMemory(Index index, ComponentClassId cci) {
        assert(cci < Configuration::MaxComponentTypes);
        auto[page, offset] = IndexToPage(index);
        char *componentPageMemory = componentPageArray[page][cci];
        assert(componentPageMemory);
        auto &info = BaseComponentInfo::GetComponentTypeInfo(cci);
        return componentPageMemory + offset * info.byteSize;
    }
    template<typename T>
    T* GetComponentMemory(Index index) {
        auto[page, offset] = IndexToPage(index);
        auto id = ComponentClassIdValue<T>;
        void *componentPageMemory = componentPageArray[page][id];
        assert(componentPageMemory);
        T *ptr = reinterpret_cast<T*>(componentPageMemory) + offset;
        return ptr;
    }
    template<typename T>
    T* GetComponentMemory(Index page, Index offset) {
        auto id = ComponentClassIdValue<T>;
        void *componentPageMemory = componentPageArray[page][id];
        assert(componentPageMemory);
        T *ptr = reinterpret_cast<T*>(componentPageMemory) + offset;
        return ptr;
    }
    std::pair<uint16_t, uint16_t> IndexToPage(uint32_t index) {
        if (index >= Configuration::MaxComponent) {
            __debugbreak();
            assert(false);
            throw std::runtime_error("Invalid index!");
        }
        uint16_t page = static_cast<uint16_t>(index / Configuration::ComponentsPerPage);
        uint16_t offset = index % Configuration::ComponentsPerPage;
        EnsurePageExists(page);
        return { page, offset, };
    }

    void EnsurePageExists(uint16_t pageIndex) {
        if (!pageMemory[pageIndex] && !AllocatePage(pageIndex))
            throw std::runtime_error("Cannot allocate page!");
    }
};

}
