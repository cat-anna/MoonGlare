#pragma once

#include <attribute_map.hpp>
#include <memory>
#include <string>
#include <vector>

namespace MoonGlare::Tools {

template <typename Type>
class EditableType : public iEditableType {
public:
    template <typename... ARGS>
    EditableType(std::shared_ptr<iEditableTypeProvider> type_provider,
                 std::shared_ptr<iAttributeProvider<Type>> provider, ARGS &&... args)
        : attribute_provider(std::move(provider)), type_provider(std::move(type_provider)),
          type_instance(std::make_shared<Type>(std::forward<ARGS>(args)...)) {}

    EditableType(std::shared_ptr<iEditableTypeProvider> type_provider,
                 std::shared_ptr<iAttributeProvider<Type>> provider, std::shared_ptr<Type> instance)
        : attribute_provider(std::move(provider)), type_provider(std::move(type_provider)),
          type_instance(std::move(instance)) {}

    virtual ~EditableType() = default;

    std::vector<std::shared_ptr<iAttributeBase>> GetAttributeInfo() const override {
        return attribute_provider->GetAttributeInfo();
    }
    std::string GetTypeName() const override { return attribute_provider->GetTypeName(); }
    std::string GetName() const override { return attribute_provider->GetName(); }
    std::string GetReadableInfo() const override { return attribute_provider->GetReadableInfo(); };
    std::shared_ptr<iAttributeBase> FindFieldByName(const std::string &filed_name) const override {
        return attribute_provider->FindFieldByName(filed_name);
    }

    std::shared_ptr<iEditableType> CreateObject(std::shared_ptr<iEditableTypeProvider> provider) const override {
        throw std::runtime_error(__FUNCTION__); //
    }

    void Load(const nlohmann::json &json) override { *type_instance = json.get<Type>(); }
    void Save(nlohmann::json &json) const override { json = *type_instance; }

    std::shared_ptr<iEditableType> GetField(std::shared_ptr<iAttributeBase> field) const override {
        return field->CreateWrapper(type_provider, std::any(type_instance));
    }

    VariantType GetValue(std::shared_ptr<iAttributeBase> field) const override {
        return field->GetValue(std::any(type_instance));
    }
    void SetValue(std::shared_ptr<iAttributeBase> field, const VariantType &value) const override {
        return field->SetValue(std::any(type_instance), value);
    }

private:
    std::shared_ptr<iAttributeProvider<Type>> const attribute_provider;
    std::shared_ptr<iEditableTypeProvider> const type_provider;
    std::shared_ptr<Type> type_instance;
};

} // namespace MoonGlare::Tools
