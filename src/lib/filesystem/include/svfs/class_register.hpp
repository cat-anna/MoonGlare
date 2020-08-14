#pragma once

#include "svfs/file_table_interface.hpp"
#include "svfs/variant_argument_map.hpp"
#include "svfs/vfs_container.hpp"
#include "svfs/vfs_exporter.hpp"
#include "svfs/vfs_module.hpp"
#include "svfs/vfs_module_interface.hpp"
#include <memory>
#include <string_view>
#include <vector>


namespace MoonGlare::StarVfs {

class iClassRegister {
public:
    explicit iClassRegister() = default;
    virtual ~iClassRegister() = default;

    virtual std::unique_ptr<iVfsContainer> CreateContainerObject(const std::string_view &class_name,
                                                                 iFileTableInterface *,
                                                                 const VariantArgumentMap &) const = 0;
    virtual std::unique_ptr<iVfsModule> CreateModuleObject(const std::string_view &class_name, iVfsModuleInterface *,
                                                           const VariantArgumentMap &) const = 0;
    virtual std::unique_ptr<iVfsExporter> CreateExporterObject(const std::string_view &class_name,
                                                               iVfsModuleInterface *,
                                                               const VariantArgumentMap &) const = 0;

    virtual std::vector<std::string> GetRegisteredModuleClasses() const = 0;
    virtual std::vector<std::string> GetRegisteredContainerClasses() const = 0;
    virtual std::vector<std::string> GetRegisteredExporterClasses() const = 0;

private:
};

} // namespace MoonGlare::StarVfs