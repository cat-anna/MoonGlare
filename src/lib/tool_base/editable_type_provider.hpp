#pragma once

#include "editable_type.hpp"
#include <attribute_map.hpp>
#include <memory>
#include <string>
#include <vector>

namespace MoonGlare::Tools {

class iEditableTypeProvider {
public:
    virtual void RegisterType(const std::string &group, std::shared_ptr<iAttributeProviderBase> pointer) = 0;

    template <typename... TYPES>
    void RegisterTypes(const std::string &group) {
        std::vector<std::shared_ptr<iAttributeProviderBase>> types = {
            GetTypeInfo(static_cast<TYPES *>(0))...,
        };
        while (types.size() > 0) {
            RegisterType(group, std::move(types.back()));
            types.pop_back();
        }
    }

    virtual std::shared_ptr<iEditableType> CreateObject(const std::string &full_name) const = 0;

    virtual std::shared_ptr<iAttributeProviderBase> GetTypeInfoByName(const std::string &full_name) const = 0;
    virtual std::shared_ptr<iAttributeProviderBase> GetTypeInfoByTypeName(const std::string &full_name) const = 0;

    template <typename T>
    std::shared_ptr<iAttributeProvider<T>> GetTypeInfoByTypeName(const std::string &full_name) const {
        return std::dynamic_pointer_cast<iAttributeProvider<T>>(GetTypeInfoByTypeName(full_name));
    }

    template <typename T>
    std::shared_ptr<iAttributeProvider<T>> GetTypeInfoByName(const std::string &full_name) const {
        return std::dynamic_pointer_cast<iAttributeProvider<T>>(GetTypeInfoByName(full_name));
    }

protected:
    virtual ~iEditableTypeProvider() = default;
};

} // namespace MoonGlare::Tools
