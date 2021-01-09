
#pragma once

#include "variant_argument_map.hpp"
#include <any>
#include <nlohmann/json.hpp>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace MoonGlare {

class iEditableType;
namespace Tools {
class iEditableTypeProvider;
}
class iAttributeBase : public std::enable_shared_from_this<iAttributeBase> {
public:
    using VariantType = VariantArgumentMap::VariantType;

    virtual bool IsBasicType() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetTypeName() const = 0;
    virtual const std::type_info &GetTypeInfo() const = 0;

    virtual VariantType GetValue(std::any object) const = 0;
    virtual void SetValue(std::any object, const VariantType &value) const = 0;
    virtual std::shared_ptr<iEditableType> CreateWrapper(std::shared_ptr<Tools::iEditableTypeProvider> provider,
                                                         std::any instance) const = 0;

protected:
    virtual ~iAttributeBase() = default;
};

template <typename OwnerObject>
class iAttribute : public iAttributeBase {
public:
    virtual VariantType GetValue(const OwnerObject *object) const = 0;
    virtual void SetValue(OwnerObject *object, const VariantType &value) const = 0;

protected:
    ~iAttribute() override = default;
};

class iAttributeProviderBase : public std::enable_shared_from_this<iAttributeProviderBase> {
public:
    virtual std::vector<std::shared_ptr<iAttributeBase>> GetAttributeInfo() const = 0;
    virtual std::shared_ptr<iAttributeBase> FindFieldByName(const std::string &field_name) const = 0;
    virtual std::string GetName() const = 0;
    virtual std::string GetTypeName() const = 0;
    virtual std::string GetReadableInfo() const = 0;

    virtual std::shared_ptr<iEditableType>
    CreateObject(std::shared_ptr<Tools::iEditableTypeProvider> provider) const = 0;

protected:
    virtual ~iAttributeProviderBase() = default;
};

template <typename OwnerObject>
class iAttributeProvider : public iAttributeProviderBase {
public:
    ~iAttributeProvider() override = default;

    virtual std::vector<std::shared_ptr<iAttribute<OwnerObject>>> GetAttributes() const = 0;

    virtual VariantArgumentMap GetAttributeValues(const OwnerObject *owner_object) const = 0;
    virtual void SetAttributeValues(OwnerObject *owner_object, const VariantArgumentMap &argument_map) const = 0;
};

class iEditableType : public iAttributeProviderBase {
public:
    virtual ~iEditableType() = default;

    using VariantType = VariantArgumentMap::VariantType;

    // virtual void* GetVoidPointer() const = 0;

    virtual void Load(const nlohmann::json &json) = 0;
    virtual void Save(nlohmann::json &json) const = 0;

    // virtual VariantArgumentMap GetAttributeValues() const = 0;
    // virtual void SetAttributeValues(const VariantArgumentMap &argument_map) const = 0;

    virtual VariantType GetValue(std::shared_ptr<iAttributeBase> filed) const = 0;
    virtual void SetValue(std::shared_ptr<iAttributeBase>, const VariantType &value) const = 0;

    virtual void SetValue(const std::string &field_name, const VariantType &value) const {
        SetValue(FindFieldByName(field_name), value);
    }

    virtual std::shared_ptr<iEditableType> GetField(std::shared_ptr<iAttributeBase> filed) const = 0;
};

} // namespace MoonGlare
