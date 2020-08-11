#pragma once

#include "file_table_interface.h"
#include "readonly_file_system.h"
#include "star_vfs.h"
#include "svfs/definitions.h"
#include "svfs/vfs_container.h"
#include "svfs/vfs_module.h"
#include "svfs/vfs_module_interface.hpp"
#include "variant_argument_map.hpp"
#include <any>
#include <string_view>
#include <svfs/class_register.hpp>
#include <template_class_list.h>

namespace MoonGlare::StarVfs {

class SvfsClassRegister : public iClassRegister {
public:
    explicit SvfsClassRegister() = default;
    ~SvfsClassRegister() override = default;

    void RegisterAll();

    template <typename T> void RegisterContainerClass() { container_class_register.Register<T>(T::kClassName); }
    template <typename T> void RegisterModuleClass() { module_class_register.Register<T>(T::kClassName); }
    template <typename T> void RegisterExporterClass() {
        // exporter_class_register.Register<T>(T::kClassName);
    }

    std::unique_ptr<iVfsModule> CreateModuleObject(const std::string_view &class_name, iVfsModuleInterface *,
                                                   const VariantArgumentMap &) const override;
    std::unique_ptr<iVfsContainer> CreateContainerObject(const std::string_view &class_name, iFileTableInterface *,
                                                         const VariantArgumentMap &) const override;

    std::vector<std::string> GetRegisteredModuleClasses() const override;
    std::vector<std::string> GetRegisteredContainerClasses() const override;
    std::vector<std::string> GetRegisteredExporterClasses() const override;

private:
    using ModuleClassRegister = Tools::TemplateClassList<iVfsModule, iVfsModuleInterface *, const VariantArgumentMap &>;
    using ContainerClassRegister =
        Tools::TemplateClassList<iVfsContainer, iFileTableInterface *, const VariantArgumentMap &>;
    using ModuleClassRegister = Tools::TemplateClassList<iVfsModule, iVfsModuleInterface *, const VariantArgumentMap &>;

    ContainerClassRegister container_class_register;
    ModuleClassRegister module_class_register;
};

} // namespace MoonGlare::StarVfs