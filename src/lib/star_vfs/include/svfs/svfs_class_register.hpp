#pragma once

#include "readonly_file_system.h"
#include "star_vfs.h"
#include "svfs/file_table_interface.hpp"
#include "svfs/vfs_container.hpp"
#include "svfs/vfs_exporter.hpp"
#include "svfs/vfs_module.hpp"
#include "svfs/vfs_module_interface.hpp"
#include <any>
#include <string_view>
#include <svfs/class_register.hpp>
#include <template_class_list.h>
#include <variant_argument_map.hpp>

namespace MoonGlare::StarVfs {

class SvfsClassRegister : public iClassRegister {
public:
    explicit SvfsClassRegister() = default;
    ~SvfsClassRegister() override = default;

    void RegisterAll();
    void RegisterBase();
    void RegisterTools();

    template <typename T>
    void RegisterContainerClass() {
        container_class_register.Register<T>(T::kClassName);
    }
    template <typename T>
    void RegisterModuleClass() {
        module_class_register.Register<T>(T::kClassName);
    }
    template <typename T>
    void RegisterExporterClass() {
        exporter_class_register.Register<T>(T::kClassName);
    }

    std::unique_ptr<iVfsContainer> CreateContainerObject(const std::string_view &class_name, iFileTableInterface *,
                                                         const VariantArgumentMap &) const override;
    std::unique_ptr<iVfsModule> CreateModuleObject(const std::string_view &class_name, iVfsModuleInterface *,
                                                   const VariantArgumentMap &) const override;
    std::unique_ptr<iVfsExporter> CreateExporterObject(const std::string_view &class_name, iVfsModuleInterface *,
                                                       const VariantArgumentMap &) const override;

    std::vector<std::string> GetRegisteredModuleClasses() const override;
    std::vector<std::string> GetRegisteredContainerClasses() const override;
    std::vector<std::string> GetRegisteredExporterClasses() const override;

private:
    using ContainerClassRegister =
        Tools::TemplateClassList<iVfsContainer, iFileTableInterface *, const VariantArgumentMap &>;
    using ModuleClassRegister = Tools::TemplateClassList<iVfsModule, iVfsModuleInterface *, const VariantArgumentMap &>;
    using ExporterClassRegister =
        Tools::TemplateClassList<iVfsExporter, iVfsModuleInterface *, const VariantArgumentMap &>;

    ContainerClassRegister container_class_register;
    ModuleClassRegister module_class_register;
    ExporterClassRegister exporter_class_register;
};

} // namespace MoonGlare::StarVfs