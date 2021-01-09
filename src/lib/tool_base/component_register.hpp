#pragma once

#include <attribute_map.hpp>
#include <component/component_common.hpp>
#include <memory>
#include <string>
#include <vector>

namespace MoonGlare::Tools {

class iComponentRegister {
public:
    struct ComponentInfo {
        MoonGlare::Component::ComponentId id;
        std::string name;
        std::string editable_type_name;
        bool is_editable;
        bool is_serializable;
        size_t byte_size;
    };

    virtual void RegisterComponent(std::shared_ptr<ComponentInfo> info) = 0;
    virtual std::vector<std::shared_ptr<ComponentInfo>> GetComponents() const = 0;
    virtual std::shared_ptr<ComponentInfo> GetComponentInfo(MoonGlare::Component::ComponentId id) const = 0;
    virtual std::shared_ptr<ComponentInfo> GetComponentInfo(const std::string &component_name) const = 0;
    virtual std::shared_ptr<iEditableType> CreateComponentData(std::shared_ptr<ComponentInfo> info) const = 0;

    template <typename T>
    std::shared_ptr<ComponentInfo> CreateComponentInfo(const std::string &type_group) {
        auto info = std::make_shared<ComponentInfo>();
        info->id = T::kComponentId;
        info->name = T::kComponentName;
        info->byte_size = sizeof(T);
        info->is_editable = T::kEditable;
        info->is_serializable = T::kSerializable;
        info->editable_type_name = type_group + "." + info->name;
        return info;
    };

    template <typename... TYPES>
    void RegisterComponents(const std::string &type_group) {
        std::vector<std::shared_ptr<ComponentInfo>> infos = {
            CreateComponentInfo<TYPES>(type_group)...,
        };
        while (infos.size() > 0) {
            RegisterComponent(std::move(infos.back()));
            infos.pop_back();
        }
    }

protected:
    virtual ~iComponentRegister() = default;
};

} // namespace MoonGlare::Tools
