#pragma once

#include "writable_file_system.h"
#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant_argument_map.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

class iVfsModule;
class iVfsContainer;
class iVfsExporter;

class iStarVfs : public iWritableFileSystem {
public:
    virtual ~iStarVfs() = default;

    virtual iVfsContainer *MountContainer(const std::string_view &container_class,
                                          const VariantArgumentMap &arguments) = 0;
    virtual iVfsModule *LoadModule(const std::string_view &module_class,
                                   const VariantArgumentMap &arguments) = 0;
    virtual std::unique_ptr<iVfsExporter> CreateExporter(const std::string_view &module_class,
                                                         const VariantArgumentMap &arguments) = 0;
};

} // namespace MoonGlare::StarVfs
