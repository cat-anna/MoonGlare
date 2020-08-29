#pragma once

#include "editable_type_provider.hpp"
#include <runtime_modules.h>

namespace MoonGlare::Tools::RuntineModules {

class EditableTypeProviderModule : public iModule, public iEditableTypeProvider {
public:
    EditableTypeProviderModule(SharedModuleManager modmgr);

    void RegisterType(const std::string &group, std::shared_ptr<iAttributeProviderBase> pointer) override;
    std::shared_ptr<iEditableType> CreateObject(const std::string &full_name) const override;

    using TypesMapType = std::unordered_map<std::string, std::shared_ptr<iAttributeProviderBase>>;

    std::shared_ptr<iAttributeProviderBase> GetTypeInfoByName(const std::string &full_name) const override;
    std::shared_ptr<iAttributeProviderBase> GetTypeInfoByTypeName(const std::string &full_name) const override;

protected:
    TypesMapType by_name;
    TypesMapType by_type;
};

} // namespace MoonGlare::Tools::RuntineModules
