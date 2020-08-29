
#pragma once

#include "attribute_map.hpp"
#include "editable_type.hpp"
#include "variant_argument_map.hpp"
#include <any>
#include <memory>
#include <nlohmann/json.hpp>
#include <static_string.hpp>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace MoonGlare {

namespace detail {

using VariantType = VariantArgumentMap::VariantType;

template <typename T>
std::string TypeName() {
    return typeid(T).name();
}

template <>
std::string TypeName<std::string>() {
    return "std::string";
}

template <typename T>
constexpr bool BasicType = std::is_fundamental_v<T>;

template <>
constexpr bool BasicType<std::string> = true;

template <typename T>
auto GetVariantValue(const T &t) {
    return VariantType(t);
}
template <>
auto GetVariantValue<int>(const int &i) {
    return VariantType(static_cast<int64_t>(i));
}

template <>
auto GetVariantValue<float>(const float &f) {
    return VariantType(static_cast<double>(f));
}

template <typename T, size_t S, typename L>
auto GetVariantValue(const BasicStaticString<S, T, L> &t) {
    return VariantType(to_string(t));
}

template <typename OwnerObject, typename FieldType, typename AliasType = FieldType>
struct TypeAttribute : public iAttribute<OwnerObject> {
    TypeAttribute(std::string name, FieldType OwnerObject::*pointer) : name(std::move(name)), field_pointer(pointer) {}

    VariantType GetValue(const OwnerObject *object) const override {
        if constexpr (BasicType<AliasType>) {
            return GetVariantValue(object->*field_pointer);
        } else {
            return VariantType(std::any(object->*field_pointer));
        }
    }
    void SetValue(OwnerObject *object, const VariantType &value) const override {
        object->*field_pointer = VariantArgumentMap::CastVariant<AliasType>(value);
    }

    bool IsBasicType() const override { return BasicType<AliasType>; }
    std::string GetName() const override { return name; }
    std::string GetTypeName() const override { return TypeName<AliasType>(); }
    const std::type_info &GetTypeInfo() const override { return typeid(FieldType); }

    std::shared_ptr<iEditableType> CreateWrapper(std::shared_ptr<Tools::iEditableTypeProvider> provider,
                                                 std::any instance) const override;
    VariantType GetValue(std::any object) const {
        auto owner = std::any_cast<std::shared_ptr<OwnerObject>>(object);
        return GetValue(owner.get());
    }

    void SetValue(std::any object, const VariantType &value) const {
        auto owner = std::any_cast<std::shared_ptr<OwnerObject>>(object);
        SetValue(owner.get(), value);
    }

private:
    const std::string name;
    FieldType OwnerObject::*field_pointer;
};

template <typename T>
struct MemberPointerTrait {};

template <typename OwnerObject, typename FieldType>
struct MemberPointerTrait<FieldType OwnerObject::*> {
    using Owner = OwnerObject;
    using Field = FieldType;
};

} // namespace detail

template <typename OwnerObject>
struct AttributeMapBuilder : public iAttributeProvider<OwnerObject> {
    AttributeMapBuilder(std::string name) : name(std::move(name)) {}

    static std::shared_ptr<AttributeMapBuilder<OwnerObject>> Start(std::string name) {
        return std::make_shared<AttributeMapBuilder<OwnerObject>>(std::move(name));
    }

    std::vector<std::shared_ptr<iAttributeBase>> GetAttributeInfo() const override {
        std::vector<std::shared_ptr<iAttributeBase>> r;
        for (auto field : fields) {
            r.emplace_back(field);
        }
        return r;
    }
    std::vector<std::shared_ptr<iAttribute<OwnerObject>>> GetAttributes() const override { return fields; }
    std::shared_ptr<iAttributeBase> FindFieldByName(const std::string &field_name) const override {
        for (auto &item : fields) {
            if (item->GetName() == field_name) {
                return item;
            }
        }
        return nullptr;
    };

    std::string GetTypeName() const override { return detail::TypeName<OwnerObject>(); }
    std::string GetName() const override { return name; }
    std::string GetReadableInfo() const override {
        std::string members;
        members = GetName();
        members += "[";
        bool first = true;
        for (auto &attr : fields) {
            if (!first) {
                members += ",";
            }
            first = false;
            members += attr->GetTypeName();
            members += " ";
            members += attr->GetName();
        }
        members += "]";
        return members;
    }

    VariantArgumentMap GetAttributeValues(const OwnerObject *owner_object) const override {
        throw std::runtime_error(__FUNCTION__);
    }
    void SetAttributeValues(OwnerObject *owner_object, const VariantArgumentMap &argument_map) const override {
        throw std::runtime_error(__FUNCTION__);
    }

    virtual std::shared_ptr<iEditableType> CreateObject(std::shared_ptr<Tools::iEditableTypeProvider> provider) const {
        auto shared_me = const_cast<AttributeMapBuilder<OwnerObject> *>(this)->shared_from_this();
        return std::make_shared<Tools::EditableType<OwnerObject>>(
            std::move(provider), std::dynamic_pointer_cast<iAttributeProvider<OwnerObject>>(shared_me));
    }

    template <typename TypeAlias, typename FieldPointer>
    std::shared_ptr<AttributeMapBuilder<OwnerObject>> AddFieldWithAlias(std::string name, FieldPointer pointer) {
        using FieldType = detail::MemberPointerTrait<FieldPointer>::Field;
        using TypeAttribute = detail::TypeAttribute<OwnerObject, FieldType, TypeAlias>;
        fields.emplace_back(std::make_shared<TypeAttribute>(std::move(name), pointer));
        return std::dynamic_pointer_cast<AttributeMapBuilder<OwnerObject>>(shared_from_this());
    }

    template <typename FieldPointer>
    std::shared_ptr<AttributeMapBuilder<OwnerObject>> AddField(std::string name, FieldPointer pointer) {
        using FieldType = detail::MemberPointerTrait<FieldPointer>::Field;
        return AddFieldWithAlias<FieldType, FieldPointer>(std::move(name), pointer);
    }

private:
    std::string name;
    std::vector<std::shared_ptr<iAttribute<OwnerObject>>> fields;
};

namespace detail {

template <typename O, typename F, typename A>
std::shared_ptr<iEditableType>
TypeAttribute<O, F, A>::CreateWrapper(std::shared_ptr<Tools::iEditableTypeProvider> provider, std::any instance) const {

    auto filed_type = provider->GetTypeInfoByTypeName<F>(GetTypeName());
    if (filed_type == nullptr) {
        throw std::runtime_error("Not an editable field");
    }
    std::shared_ptr<O> owner = std::any_cast<std::shared_ptr<O>>(instance);
    F *member = &(owner.get()->*field_pointer);
    auto member_ptr = std::shared_ptr<F>(owner, member);
    return std::make_shared<Tools::EditableType<F>>(std::move(provider), std::move(filed_type), member_ptr);
};

} // namespace detail

} // namespace MoonGlare
