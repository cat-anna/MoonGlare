#pragma once

#include <attribute_map.hpp>
#include <component/component_common.hpp>
#include <component_register.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <runtime_modules.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace MoonGlare::Tools {

class EditableComponent;

class EditableEntity : public std::enable_shared_from_this<EditableEntity> {
public:
    EditableEntity(SharedModuleManager manager, std::shared_ptr<EditableEntity> parent = {});
    virtual ~EditableEntity() = default;

    using ComponentId = MoonGlare::Component::ComponentId;

    void SaveToJson(nlohmann::json &j) const;
    void LoadFromJson(const nlohmann::json &j);

    std::string SerializeToJson();
    void DeserializeFromJson(std::string &out);
    void DeserializeToChild(std::string &out);

    void SetName(std::optional<std::string> name);
    std::optional<std::string> GetName() const;

    // const std::string& GetPatternURI() { return m_PatternURI; }
    // void SetPatternURI(std::string v) { m_PatternURI.swap(v); }

    template <typename F>
    void ForEachChildren(F &f) {
        for (auto &item : children) {
            f(item);
        }
    }

    template <typename F>
    void ForEachComponent(F &f) {
        for (auto &item : components) {
            f(item.first, item.second);
        }
    }

    std::shared_ptr<EditableEntity> GetParent() const { return parent.lock(); }

    void MoveUp(std::shared_ptr<EditableEntity> c);
    void MoveDown(std::shared_ptr<EditableEntity> c);

    int Find(std::shared_ptr<EditableEntity> c) {
        for (int i = 0; i < (int)children.size(); ++i)
            if (children[i] == c)
                return i;
        return -1;
    }

    std::shared_ptr<EditableEntity> AddChild();
    void DeleteChild(std::shared_ptr<EditableEntity>);
    std::shared_ptr<EditableComponent> AddComponent(ComponentId component_id);
    void DeleteComponent(std::shared_ptr<EditableComponent>);

    bool IsDeletable() const { return GetParent() != nullptr; }
    bool IsMovable() const { return GetParent() != nullptr; }

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool v) { enabled = v; }
    // void Clear();

protected:
    SharedModuleManager module_manager;
    std::weak_ptr<EditableEntity> parent;
    std::vector<std::shared_ptr<EditableEntity>> children;
    std::unordered_map<ComponentId, std::shared_ptr<EditableComponent>> components;

    bool enabled = true;
};

class EditableComponent : public std::enable_shared_from_this<EditableComponent> {
public:
    using ComponentId = MoonGlare::Component::ComponentId;

    EditableComponent(SharedModuleManager manager, std::shared_ptr<EditableEntity> parent, ComponentId component_id);
    virtual ~EditableComponent() = default;

    ComponentId GetComponentId() const { return component_info->id; }
    std::shared_ptr<iComponentRegister::ComponentInfo> GetComponentInfo() const { return component_info; }
    std::shared_ptr<iEditableType> GetComponentData() const { return component_data; };

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool v) { enabled = v; }

    void SaveToJson(nlohmann::json &j) const;
    void LoadFromJson(const nlohmann::json &j);

protected:
    SharedModuleManager module_manager;
    std::weak_ptr<EditableEntity> parent;
    std::shared_ptr<iComponentRegister::ComponentInfo> component_info;
    std::shared_ptr<iEditableType> component_data;
    bool enabled = true;
};

} // namespace MoonGlare::Tools
