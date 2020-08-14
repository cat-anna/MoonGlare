#pragma once

#include "readonly_file_system.h"
#include "svfs/definitions.hpp"
#include "svfs/variant_argument_map.hpp"
#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace MoonGlare::StarVfs {

class iVfsModule;
class iVfsContainer;
class iVfsExporter;

class iStarVfs : public iReadOnlyFileSystem {
public:
    virtual ~iStarVfs() = default;

    virtual iVfsContainer *MountContainer(const std::string_view &container_class,
                                          const VariantArgumentMap &arguments) = 0;
    virtual iVfsModule *LoadModule(const std::string_view &module_class, const VariantArgumentMap &arguments) = 0;
    virtual std::unique_ptr<iVfsExporter> CreateExporter(const std::string_view &module_class,
                                                         const VariantArgumentMap &arguments) = 0;
};

} // namespace MoonGlare::StarVfs
