#pragma once

#include "assimp_container.hpp"
#include <map>
#include <runtime_modules.h>
#include <set>
#include <string>
#include <svfs/vfs_module.hpp>
#include <unordered_map>
#include <variant_argument_map.hpp>

namespace MoonGlare::Tools::SvfsModules {

class AssimpImporterModule : public StarVfs::iVfsModule {
public:
    static constexpr const char *kClassName = "assimp_import";

    AssimpImporterModule(StarVfs::iVfsModuleInterface *module_interface, const VariantArgumentMap &arguments);
    virtual ~AssimpImporterModule();

    void Execute() override;

private:
    std::string root_point;
    std::unordered_map<std::string, AssimpContainer *> known_files;
    SharedModuleManager module_manager;
};

} // namespace MoonGlare::Tools::SvfsModules