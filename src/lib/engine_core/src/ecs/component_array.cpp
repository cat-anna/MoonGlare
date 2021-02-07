#include "ecs/component_array.hpp"
#include "ecs/component_interface.hpp"
#include "ecs/ecs_configuration.hpp"
#include <optional>

namespace MoonGlare::ECS {

void ComponentMemoryInfo::Dump() {
    AddLog(Debug, "Dumping component array page offsts:");
    AddLog(Debug, fmt::format("    mem_block_size={:08x}({:.1f}KiB) validity_and_flags={:08x}", total_memory_block_size,
                              total_memory_block_size / 1024.0f, valid_components_and_flags_offest));
    for (size_t i = 0; i < entry_size.size(); ++i) {
        if (entry_size[i] != 0) {
            AddLog(Debug,
                   fmt::format("    id={:2} offest={:08x} entry_size={:02x}", i, memory_offsets[i], entry_size[i]));
        }
    }
}

ComponentMemoryInfo ComponentMemoryInfo::CalculateOffsets(gsl::not_null<iComponentRegister *> component_register) {
    ComponentMemoryInfo r;
    r.memory_offsets.fill(0);
    r.entry_size.fill(0);

    r.valid_components_and_flags_offest = 0;
    r.total_memory_block_size += kComponentPageSize * sizeof(Component::ValidComponentsMap);

    auto c_map = component_register->GetRegisteredComponentsMap();
    for (size_t i = 0; i < r.memory_offsets.size(); ++i) {
        r.total_memory_block_size = Align16(r.total_memory_block_size);
        r.total_memory_block_size += 16; //leave some space

        r.memory_offsets[i] = r.total_memory_block_size;
        if (auto it = c_map.find(i); it != c_map.end()) {
            auto *c_info = it->second;
            const auto &details = c_info->GetDetails();
            r.entry_size[i] = static_cast<uintptr_t>(details.entry_size);
            r.ops[i] = c_info->GetOps();
            r.total_memory_block_size += details.entry_size * kComponentPageSize;
        }
    }

    r.Dump();
    return r;
}

void ComponentArrayPage::Allocate(const ComponentMemoryInfo &offsets, ComponentArrayPage &page) {
    auto memory = malloc(offsets.total_memory_block_size);
    if (memory == nullptr) {
        // TODO
    }

    memset(memory, 0, offsets.total_memory_block_size);

    page = ComponentArrayPage{};
    page.memory_block_start = memory;
    page.element_count = kComponentPageSize;
    page.memory_block_size = offsets.total_memory_block_size;
    page.valid_components_and_flags =
        reinterpret_cast<Component::ValidComponentsMap *>(memory) + offsets.valid_components_and_flags_offest;

    for (size_t i = 0; i < page.component_array.size(); ++i) {
        page.component_array[i] = ComponentArrayPage::Entry{
            .element_size = offsets.entry_size[i],
            .array_front = reinterpret_cast<uint8_t *>(memory) + offsets.memory_offsets[i],
        };
    }
}

//----------------------------------------------------------------------------------

ComponentArray::ComponentArray(gsl::not_null<iComponentRegister *> _component_register)
    : component_offsets(*_component_register->GetComponentMemoryInfo()) {
    ComponentArrayPage::Allocate(component_offsets, component_page);
}

ComponentArray::~ComponentArray() {
    //TODO: clean up
}

//----------------------------------------------------------------------------------

void *ComponentArray::CreateComponent(IndexType index, ComponentId c_id, bool call_default_constructor) {
    if (!IsIndexValid(index)) {
        return nullptr;
    }

    void *memory = GetComponentMemory(index, c_id);
    if (call_default_constructor) {
        component_offsets.ops[c_id].default_constructor(memory);
    }
    if (HasComponent(index, c_id)) {
        return memory;
    }

    auto mask = detail::MakeComponentMaskWithActiveFlag(c_id);
    SetFlags(index, true, mask);

    return memory;
}

void *ComponentArray::GetComponent(IndexType index, ComponentId c_id) const {
    if (!IsIndexValid(index)) {
        return nullptr;
    }

    if (HasComponent(index, c_id)) {
        return GetComponentMemory(index, c_id);
    }

    return nullptr;
}

void ComponentArray::RemoveComponent(IndexType index, ComponentId c_id) {
    if (!IsIndexValid(index)) {
        return;
    }
    if (!HasComponent(index, c_id)) {
        return;
    }
    void *memory = GetComponentMemory(index, c_id);
    component_offsets.ops[c_id].destructor(memory);
    auto mask = detail::MakeComponentMaskWithActiveFlag(c_id);
    SetFlags(index, false, mask);
}

void ComponentArray::RemoveAllComponents(IndexType index) {
    ForEachComponent(index, [index, this](auto c_id) { RemoveComponent(index, c_id); });
}

void ComponentArray::SetComponentActive(IndexType index, ComponentId c_id, bool active) {
    if (!IsIndexValid(index)) {
        return;
    }
    auto mask = detail::MakeComponentActiveMask(c_id);
    SetFlags(index, active, mask);
}

void ComponentArray::MarkIndexAsValid(IndexType index) {
    const auto mask = detail::MakeComponentMask<>(ComponentFlags::kValid);
    component_page.valid_components_and_flags[index] |= mask;
    max_seen_index = std::max(max_seen_index, index);
}

void ComponentArray::ReleaseIndex(IndexType index, bool destruct_components) {
    if (!ComponentArray::IsIndexValid(index)) {
        return;
    }
    if (destruct_components) {
        ComponentArray::RemoveAllComponents(index);
    }
    const auto mask = detail::MakeComponentMask<>(ComponentFlags::kValid);
    component_page.valid_components_and_flags[index] &= ~mask;

    if (index == max_seen_index) {
        //TODO:
        --max_seen_index;
    }
}

//----------------------------------------------------------------------------------

} // namespace MoonGlare::ECS
