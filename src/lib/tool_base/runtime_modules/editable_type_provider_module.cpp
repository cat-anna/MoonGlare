#include "editable_type_provider_module.hpp"
#include <orbit_logger.h>

namespace MoonGlare::Tools::RuntineModules {

EditableTypeProviderModule::EditableTypeProviderModule(SharedModuleManager modmgr) : iModule(modmgr) {
}

void EditableTypeProviderModule::RegisterType(const std::string &group,
                                              std::shared_ptr<iAttributeProviderBase> pointer) {
    auto full_name = group + "." + pointer->GetName();
    AddLogf(Info, "Registered type %s.%s", group.c_str(), pointer->GetReadableInfo().c_str());
    by_type[pointer->GetTypeName()] = pointer;
    by_name[full_name] = std::move(pointer);
}

std::shared_ptr<iEditableType> EditableTypeProviderModule::CreateObject(const std::string &full_name) const {
    auto it = by_name.find(full_name);
    if (it == by_name.end()) {
        AddLogf(Error, "Failed to find name %s - not registered", full_name.c_str());
        return nullptr;
    }

    auto shared_me = std::dynamic_pointer_cast<iEditableTypeProvider>(
        const_cast<EditableTypeProviderModule *>(this)->shared_from_this());
    return it->second->CreateObject(shared_me);
}

std::shared_ptr<iAttributeProviderBase>
EditableTypeProviderModule::GetTypeInfoByName(const std::string &full_name) const {
    auto it = by_name.find(full_name);
    if (it == by_name.end()) {
        AddLogf(Error, "Failed to find name %s - not registered", full_name.c_str());
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<iAttributeProviderBase>
EditableTypeProviderModule::GetTypeInfoByTypeName(const std::string &full_name) const {
    auto it = by_type.find(full_name);
    if (it == by_type.end()) {
        AddLogf(Error, "Failed to find type %s - not registered", full_name.c_str());
        return nullptr;
    }
    return it->second;
}

} // namespace MoonGlare::Tools::RuntineModules
