#pragma once

#include "component/component_common.hpp"
#include <aligned_ptr.hpp>
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <orbit_logger.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

namespace MoonGlare::ECS {

using ComponentId = Component::ComponentId;

class BaseComponentInfo {
public:
    struct ComponentDetails {
        size_t entry_size;
        size_t real_size;
        size_t alignment;
        bool trivial;
    };

    using ComponentPtrFunc = void (*)(void *);
    struct ComponentOps {
        ComponentPtrFunc default_constructor = [](auto) {};
        ComponentPtrFunc destructor = [](auto) {};
    };

    virtual const std::type_info &GetTypeInfo() const = 0;

    const ComponentDetails &GetDetails() const { return details; }
    const ComponentOps &GetOps() const { return ops; }
    const std::string &GetName() const { return name; }
    ComponentId GetId() const { return id; }

protected:
    BaseComponentInfo(ComponentId _id, std::string _name, ComponentDetails _details, ComponentOps _ops)
        : id(_id), name(std::move(_name)), details(_details), ops(_ops) {}

private:
    const ComponentId id;
    const std::string name;
    const ComponentDetails details;
    const ComponentOps ops;
};

inline std::string to_string(const BaseComponentInfo::ComponentDetails &cd) {
    return fmt::format("entry_size={:3};real_size={:3};alignment={:2};trivial={};", cd.entry_size, cd.real_size,
                       cd.alignment, cd.trivial ? 1 : 0);
}

template <class T>
struct ComponentInfo : public BaseComponentInfo {
    const std::type_info &GetTypeInfo() const override { return typeid(T); }

    static void CallPlacementNew(void *memory) {
        if (memory) {
            new (memory) T();
        }
    }
    static void CallDestructor(void *memory) {
        if (memory) {
            reinterpret_cast<T *>(memory)->~T();
        }
    }

    ComponentDetails MakeDetails() {
        return ComponentDetails{
            .entry_size = alignof(T) == 16 ? Align16(sizeof(T)) : sizeof(T),
            .real_size = sizeof(T),
            .alignment = alignof(T),
            .trivial = std::is_trivially_constructible_v<T>,
        };
    }
    ComponentOps MakeOps() {
        return ComponentOps{
            .default_constructor = &CallPlacementNew,
            .destructor = &CallDestructor,
        };
    }

    ComponentInfo() : BaseComponentInfo(T::kComponentId, T::kComponentName, MakeDetails(), MakeOps()) {
        if (alignof(T) == 16 && sizeof(T) & 0xF) {
            AddLog(Warning, fmt::format("Size of component {:02x}:{} is not multiply of 16!", GetId(), GetName()));
        }
    }
};

} // namespace MoonGlare::ECS
