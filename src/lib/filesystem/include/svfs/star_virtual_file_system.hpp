#pragma once

#include "readonly_file_system.h"
#include "star_vfs.h"
#include "svfs/class_register.hpp"
#include "svfs/definitions.hpp"
#include "svfs/file_table_interface.hpp"
#include "variant_argument_map.hpp"
#include <any>
#include <string>
#include <string_view>
#include <template_class_list.h>

namespace MoonGlare::StarVfs {

class iVfsModule;

class StarVirtualFileSystem : public iStarVfs {
public:
    explicit StarVirtualFileSystem(iClassRegister *class_register);
    ~StarVirtualFileSystem() override;

    iVfsContainer *MountContainer(const std::string_view &container_class,
                                  const VariantArgumentMap &arguments) override;
    iVfsModule *LoadModule(const std::string_view &module_class, const VariantArgumentMap &arguments) override;
    std::unique_ptr<iVfsExporter> CreateExporter(const std::string_view &module_class,
                                                 const VariantArgumentMap &arguments) override;

    bool ReadFileByPath(const std::string &path, std::string &file_data) const override;

    bool EnumeratePath(const std::string_view &path, FileInfoTable &result_file_table) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace MoonGlare::StarVfs