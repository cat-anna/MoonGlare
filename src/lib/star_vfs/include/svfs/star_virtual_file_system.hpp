#pragma once

#include "readonly_file_system.h"
#include "star_vfs.h"
#include "svfs/class_register.hpp"
#include "svfs/file_table_interface.hpp"
#include "svfs/svfs_hooks.hpp"
#include "variant_argument_map.hpp"
#include <any>
#include <string>
#include <string_view>
#include <template_class_list.h>

namespace MoonGlare::StarVfs {

class iVfsModule;

class StarVirtualFileSystem : public iStarVfs {
public:
    explicit StarVirtualFileSystem(iClassRegister *class_register, iStarVfsHooks *hooks = nullptr);
    ~StarVirtualFileSystem() override;

    std::string DumpStructure() const override;

    iVfsContainer *MountContainer(std::string_view container_class,
                                  const VariantArgumentMap &arguments) override;
    iVfsModule *LoadModule(std::string_view module_class,
                           const VariantArgumentMap &arguments) override;
    std::unique_ptr<iVfsExporter> CreateExporter(std::string_view module_class,
                                                 const VariantArgumentMap &arguments) override;

    bool WriteFileByPath(std::string_view path, const std::string &file_data) override;

    // iReadOnlyFileSystem
    bool ReadFileByPath(std::string_view path, std::string &file_data) const override;
    bool ReadFileByResourceId(FileResourceId resource, std::string &file_data) const override;
    bool EnumeratePath(std::string_view path, FileInfoTable &result_file_table) const override;
    FileResourceId GetResourceByPath(std::string_view path) const override;
    std::string GetNameOfResource(FileResourceId resource,
                                  bool wants_full_path = true) const override;
    bool FindFilesByExt(std::string_view ext, FileInfoTable &result_file_table) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace MoonGlare::StarVfs