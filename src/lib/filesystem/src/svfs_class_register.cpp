#pragma once

#include <svfs/dynamic_file_container.hpp>
#include <svfs/host_folder_container.h>
#include <svfs/meta_module.h>
#include <svfs/svfs_class_register.hpp>


namespace MoonGlare::StarVfs {

void SvfsClassRegister::RegisterAll() {
    RegisterContainerClass<DynamicFileContainer>();
    RegisterContainerClass<HostFolderContainer>();
    RegisterModuleClass<MetaModule>();
}

std::unique_ptr<iVfsModule> SvfsClassRegister::CreateModuleObject(const std::string_view &class_name,
                                                                  iVfsModuleInterface *iface,
                                                                  const VariantArgumentMap &args) const {

    return module_class_register.CreateUnique(std::string(class_name), iface, args);
}

std::unique_ptr<iVfsContainer> SvfsClassRegister::CreateContainerObject(const std::string_view &class_name,
                                                                        iFileTableInterface *iface,
                                                                        const VariantArgumentMap &args) const {
    return container_class_register.CreateUnique(std::string(class_name), iface, args);
};

std::vector<std::string> SvfsClassRegister::GetRegisteredModuleClasses() const {
    std::vector<std::string> r;
    module_class_register.Enumerate([&r](auto item) { r.emplace_back(item.Alias); });
    return r;
}

std::vector<std::string> SvfsClassRegister::GetRegisteredContainerClasses() const {
    std::vector<std::string> r;
    container_class_register.Enumerate([&r](auto item) { r.emplace_back(item.Alias); });
    return r;
}

std::vector<std::string> SvfsClassRegister::GetRegisteredExporterClasses() const { return {}; }

} // namespace MoonGlare::StarVfs
