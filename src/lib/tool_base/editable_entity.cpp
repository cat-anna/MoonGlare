#include "editable_entity.hpp"

namespace MoonGlare::Tools {

EditableEntity::EditableEntity(SharedModuleManager manager, std::shared_ptr<EditableEntity> parent)
    : module_manager(std::move(manager)), parent(parent) {
}

//----------------------------------------------------------------------------------

void EditableEntity::SetName(std::optional<std::string> name) {
    auto name_component = AddComponent("Name");
    if (!name.has_value() || name.value().empty()) {
        DeleteComponent(name_component);
        return;
    }

    auto value_field = name_component->GetComponentData()->FindFieldByName("value");
    if (!value_field) {
        return; //TODO
    }
    name_component->GetComponentData()->SetValue(value_field, VariantArgumentMap::VariantType(name.value_or("")));
}

std::optional<std::string> EditableEntity::GetName() const {
    auto cr = module_manager->QueryModule<iComponentRegister>();
    auto ci = cr->GetComponentInfo("Name");
    if (!ci) {
        return std::nullopt;
    }
    auto exists = components.find(ci->id);
    if (exists == components.end()) {
        return std::nullopt;
    }
    auto &name_component = exists->second;
    if (!name_component->IsEnabled()) {
        return std::nullopt;
    }
    auto value_field = name_component->GetComponentData()->FindFieldByName("value");
    if (!value_field) {
        return std::nullopt; //TODO
    }
    return VariantArgumentMap::CastVariant<std::string>(name_component->GetComponentData()->GetValue(value_field));
}

//----------------------------------------------------------------------------------

void EditableEntity::SaveToJson(nlohmann::json &j) const {
    auto children_json = nlohmann::json::array();
    for (auto &item : children) {
        nlohmann::json child;
        item->SaveToJson(child);
        children_json.push_back(child);
    }
    j["children"] = children_json;

    auto components_json = nlohmann::json::array();
    for (auto &item : components) {
        nlohmann::json child;
        item.second->SaveToJson(child);
        components_json.push_back(child);
    }
    j["components"] = components_json;
    j["enabled"] = enabled;
}

void EditableEntity::LoadFromJson(const nlohmann::json &j) {
    try {
        j.at("enabled").get_to(enabled);
        for (auto &element : j["children"]) {
            AddChild()->LoadFromJson(element);
        }
        for (auto &element : j["components"]) {
            auto component_id = element.at("component_id").get<ComponentId>();
            AddComponent(component_id)->LoadFromJson(element);
        }
    } catch (const std::exception &e) {
        AddLogf(Error, "Failed to load child: %s", e.what());
    }
}

std::string EditableEntity::SerializeToJson() {
    nlohmann::json j;
    SaveToJson(j);
    return j.dump(4);
}

void EditableEntity::DeserializeFromJson(std::string &out) {
    if (out.empty()) {
        return;
    }
    nlohmann::json j = nlohmann::json::parse(out);
    LoadFromJson(j);
}

void EditableEntity::DeserializeToChild(std::string &out) {
    nlohmann::json j = nlohmann::json::parse(out);
    AddChild()->LoadFromJson(j);
}

//----------------------------------------------------------------------------------

std::shared_ptr<EditableEntity> EditableEntity::AddChild() {
    auto child = std::make_shared<EditableEntity>(module_manager, shared_from_this());
    child->SetName("child");
    children.emplace_back(child);
    return child;
}

void EditableEntity::DeleteChild(std::shared_ptr<EditableEntity> c) {
    children.erase(std::remove(children.begin(), children.end(), c), children.end());
}

std::shared_ptr<EditableComponent> EditableEntity::AddComponent(const std::string &component_name) {
    auto cr = module_manager->QueryModule<iComponentRegister>();
    return AddComponent(cr->GetComponentInfo(component_name)->id);
}

std::shared_ptr<EditableComponent> EditableEntity::AddComponent(ComponentId component_id) {
    auto existing = components.find(component_id);
    if (existing != components.end()) {
        return existing->second;
    }
    auto component = std::make_shared<EditableComponent>(module_manager, shared_from_this(), component_id);
    components[component_id] = component;
    return component;
}

void EditableEntity::DeleteComponent(std::shared_ptr<EditableComponent> c) {
    auto component_id = c->GetComponentId();
    auto existing = components.find(component_id);
    if (existing == components.end()) {
        return;
    }
    components.erase(existing);
}

void EditableEntity::MoveDown(std::shared_ptr<EditableEntity> c) {
    auto idx = Find(c);
    if (idx < 0)
        return;
    if (idx + 1 >= (int)children.size())
        return;
    children[idx].swap(children[idx + 1]);
}

void EditableEntity::MoveUp(std::shared_ptr<EditableEntity> c) {
    auto idx = Find(c);
    if (idx <= 0)
        return;
    children[idx].swap(children[idx - 1]);
}

//----------------------------------------------------------------------------------

EditableComponent::EditableComponent(SharedModuleManager manager, std::shared_ptr<EditableEntity> parent,
                                     ComponentId component_id)
    : module_manager(std::move(manager)), parent(parent) {
    auto cr = module_manager->QueryModule<iComponentRegister>();
    component_info = cr->GetComponentInfo(component_id);
    component_data = cr->CreateComponentData(component_info);
}

void EditableComponent::SaveToJson(nlohmann::json &j) const {
    component_data->Save(j["data"]);
    j["enabled"] = enabled;
    j["class"] = component_info->name;
    j["component_id"] = component_info->id;
}

void EditableComponent::LoadFromJson(const nlohmann::json &j) {
    try {
        component_data->Load(j.at("data"));
        j.at("enabled").get_to(enabled);
    } catch (...) {
    }
}

} // namespace MoonGlare::Tools
