#pragma once

#include "aligned_ptr.hpp"
#include "component/component_common.hpp"
#include "resource_id.hpp"
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
        bool json_serializable;
        bool has_resources;
    };

    using ComponentPtrFunc = void (*)(void *);
    struct ComponentOps {
        ComponentPtrFunc default_constructor = [](auto) {};
        ComponentPtrFunc destructor = [](auto) {};
    };

    using ComponentConstJsonFunc = void (*)(void *, const nlohmann::json &);
    using LoadResourcesFunc = void (*)(void *, iRuntimeResourceLoader &);

    struct ComponentIoOps {
        ComponentConstJsonFunc construct_from_json = nullptr;
        LoadResourcesFunc load_resources = nullptr;
    };

    virtual const std::type_info &GetTypeInfo() const = 0;

    const ComponentDetails &GetDetails() const { return details; }
    const ComponentOps &GetOps() const { return ops; }
    const ComponentIoOps &GetIoOps() const { return io_ops; }
    const std::string &GetName() const { return name; }
    ComponentId GetId() const { return id; }

protected:
    BaseComponentInfo(ComponentId _id, std::string _name, ComponentDetails _details,
                      ComponentOps _ops, ComponentIoOps _io_ops)
        : id(_id), name(std::move(_name)), details(_details), ops(_ops), io_ops(_io_ops) {}

private:
    const ComponentId id;
    const std::string name;
    const ComponentDetails details;
    const ComponentOps ops;
    const ComponentIoOps io_ops;
};

inline std::string to_string(const BaseComponentInfo::ComponentDetails &cd) {
    auto b = [](auto v) { return v ? 1 : 0; };
    return fmt::format(
        "entry_size={:3};real_size={:3};alignment={:2};trivial={};json={};resources={}",
        cd.entry_size, cd.real_size, cd.alignment, b(cd.trivial), b(cd.json_serializable),
        b(cd.has_resources));
}

namespace detail {

template <typename T>
struct IsJsonSerializable {
    static constexpr bool kValue = nlohmann::detail::has_from_json<nlohmann::json, T>::value;
};

} // namespace detail

template <typename T>
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
    static void CallConstructAndLoadFromJson(void *memory, const nlohmann::json &json) {
        if (memory) {
            auto *ptr = new (memory) T();
            json.get_to(*ptr);
        }
    }
    static void CallLoadResources(void *component, iRuntimeResourceLoader &loader) {
        assert(component);
        if constexpr (T::kHasResources) {
            LoadResources(*reinterpret_cast<T *>(component), loader);
        }
    }
    static ComponentDetails MakeDetails() {
        return ComponentDetails{
            .entry_size = alignof(T) == 16 ? Align16(sizeof(T)) : sizeof(T),
            .real_size = sizeof(T),
            .alignment = alignof(T),
            .trivial = std::is_trivially_constructible_v<T>,
            .json_serializable = detail::IsJsonSerializable<T>::kValue,
            .has_resources = T::kHasResources,
        };
    }
    static ComponentOps MakeOps() {
        return ComponentOps{
            .default_constructor = &CallPlacementNew,
            .destructor = &CallDestructor,
        };
    }

    static ComponentIoOps MakeIoOps() {
        ComponentIoOps ops{};
        if constexpr (detail::IsJsonSerializable<T>::kValue) {
            ops.construct_from_json = &CallConstructAndLoadFromJson;
        }
        ops.load_resources = &CallLoadResources;
        return ops;
    }

    ComponentInfo()
        : BaseComponentInfo(T::kComponentId, T::kComponentName, MakeDetails(), MakeOps(),
                            MakeIoOps()) {
        if (alignof(T) == 16 && sizeof(T) & 0xF) {
            AddLog(Warning, fmt::format("Size of component {:02x}:{} is not multiply of 16!",
                                        GetId(), GetName()));
        }
    }
};

} // namespace MoonGlare::ECS
