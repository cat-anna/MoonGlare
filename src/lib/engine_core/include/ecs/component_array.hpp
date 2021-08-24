#pragma once

#include "component/component_common.hpp"
#include "component/parent.hpp"
#include "component_info.hpp"
#include "entity_manager_interface.hpp"
#include "pointer_memory_array.hpp"
#include <array>
#include <concepts>
#include <gsl/gsl>

namespace MoonGlare::ECS {

namespace detail {

constexpr Component::ValidComponentsMap MakeComponentMask(Component::ComponentId c_id) {
    return 1llu << c_id;
}
constexpr Component::ValidComponentsMap MakeComponentActiveMask(Component::ComponentId c_id) {
    return 1llu << (c_id + Component::kComponentActiveFlagsOffset);
}

constexpr Component::ValidComponentsMap
MakeComponentMaskWithActiveFlag(Component::ComponentId c_id) {
    return MakeComponentMask(c_id) | MakeComponentActiveMask(c_id);
}

template <typename... ARGS>
constexpr Component::ValidComponentsMap MakeComponentMask() {
    if constexpr (sizeof...(ARGS) > 0) {
        return (MakeComponentMask(ARGS::kComponentId) | ...);
    }
    return 0;
}

template <typename... ARGS>
constexpr Component::ValidComponentsMap MakeComponentActiveMask() {
    if constexpr (sizeof...(ARGS) > 0) {
        return (MakeComponentActiveMask(ARGS::kComponentId) | ...);
    }
    return 0;
}

template <typename... ARGS>
constexpr Component::ValidComponentsMap MakeComponentMaskWithActiveFlag() {
    if constexpr (sizeof...(ARGS) > 0) {
        return (MakeComponentMaskWithActiveFlag(ARGS::kComponentId) | ...);
    }
    return 0;
}

template <typename... ARGS>
constexpr Component::ValidComponentsMap MakeComponentMask(Component::ComponentFlags flag) {
    return MakeComponentMask<ARGS...>() | static_cast<Component::ValidComponentsMap>(flag);
}

template <typename... ARGS>
constexpr Component::ValidComponentsMap MakeComponentFlag(Component::ComponentFlags flag) {
    return static_cast<Component::ValidComponentsMap>(flag);
}

} // namespace detail

class iComponentRegister;

struct ComponentMemoryInfo {
    std::array<size_t, Component::kMaxComponents> memory_offsets{0};
    std::array<uintptr_t, Component::kMaxComponents> entry_size{0};
    std::array<BaseComponentInfo::ComponentOps, Component::kMaxComponents> ops{};
    size_t valid_components_and_flags_offest = 0;
    size_t total_memory_block_size = 0;

    void Dump();

    static ComponentMemoryInfo
    CalculateOffsets(gsl::not_null<iComponentRegister *> component_register);
};

struct ComponentArrayPage {
    struct Entry {
        uintptr_t element_size = 0;
        uint8_t *array_front = nullptr;

        uint8_t *GetElementMemory(size_t index) const { return array_front + index * element_size; }
    };

    Component::ValidComponentsMap *valid_components_and_flags = nullptr;
    std::array<Entry, Component::kMaxComponents> component_array;

    uint8_t *GetComponentMemory(size_t id, size_t index) const {
        return component_array[id].GetElementMemory(index);
    }

    size_t element_count = 0;

    ComponentArrayPage() { component_array.fill({}); }

    static void SetMemory(const ComponentMemoryInfo &offsets, ComponentArrayPage &page,
                          void *memory);
};

class ComponentArray final : public iComponentArray {
public:
    ComponentArray(gsl::not_null<iComponentRegister *> _component_register);
    ~ComponentArray() override;

    const ComponentArrayPage &GetComponentPage() const { return component_page; }

    //iComponentArray
    void *CreateComponent(IndexType index, ComponentId c_id,
                          bool call_default_constructor = true) override;
    void *GetComponent(IndexType index, ComponentId c_id) const override;
    void RemoveComponent(IndexType index, ComponentId c_id) override;
    void RemoveAllComponents(IndexType index) override;
    void SetComponentActive(IndexType index, ComponentId c_id, bool active) override;
    void MarkIndexAsValid(IndexType index) override;
    void ReleaseIndex(IndexType index, bool destruct_components = true) override;
    bool HasComponent(IndexType index, ComponentId c_id) const override;
    bool IsComponentActive(IndexType index, ComponentId c_id) override;
    bool IsIndexValid(IndexType index) const override;

    //template wrappers

    template <typename... Components, typename F>
    void Visit(F visit_functor) {
        constexpr auto mask = detail::MakeComponentMaskWithActiveFlag<Components...>() |
                              detail::MakeComponentFlag(ComponentFlags::kValid);

        for (IndexType index = 0; index <= component_page.element_count; ++index) {
            if ((component_page.valid_components_and_flags[index] & mask) == mask) {
                // entry is valid and has all requested components are active
                visit_functor(*iComponentArray::GetComponent<Components>(index)...);
            }
        }
    }

    template <typename ComponentClass, typename F>
    void VisitWithParent(F visit_functor) {
        constexpr auto parent_mask =
            detail::MakeComponentMaskWithActiveFlag<ComponentClass, Component::Parent>() |
            detail::MakeComponentFlag(ComponentFlags::kValid);

        constexpr auto mask =
            detail::MakeComponentMaskWithActiveFlag<ComponentClass, Component::Parent>() |
            detail::MakeComponentFlag(ComponentFlags::kValid);

        for (IndexType index = 0; index <= component_page.element_count; ++index) {
            if ((component_page.valid_components_and_flags[index] & parent_mask) == parent_mask) {
                // entry is valid and has all requested components are active
                const auto *parent = iComponentArray::GetComponent<Component::Parent>(index);
                if ((component_page.valid_components_and_flags[parent->index] & mask) == mask) {
                    visit_functor(*iComponentArray::GetComponent<ComponentClass>(parent->index),
                                  *iComponentArray::GetComponent<ComponentClass>(index));
                }
            }
        }
    }

    template <typename F>
    void ForEachComponent(IndexType index, F functor) {
        if (!IsIndexValid(index)) {
            return;
        }
        for (ComponentId c_id = 0; c_id < Component::kMaxComponents; ++c_id) {
            if (HasComponent(index, c_id)) {
                functor(c_id);
            }
        }
    }

private:
    ComponentArrayPage component_page;
    const ComponentMemoryInfo component_offsets;

    RawMemory component_page_memory;

    void *GetComponentMemory(IndexType index, ComponentId c_id) const {
        return component_page.GetComponentMemory(c_id, index);
    }

    void SetFlags(IndexType index, bool v, ValidComponentsMap flags) {
        if (v) {
            component_page.valid_components_and_flags[index] |= flags;
        } else {
            component_page.valid_components_and_flags[index] &= ~flags;
        }
    }

#if 0
    using ComponentFlagSet = FlagSet<ComponentFlags>;
    static_assert(sizeof(ComponentFlagSet) == sizeof(uint8_t));

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
#endif
};

} // namespace MoonGlare::ECS
