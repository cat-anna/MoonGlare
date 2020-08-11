
#pragma once

#include <cstdint>
#include <svfs/vfs_container.h>

namespace MoonGlare::StarVfs {

class FileTable;

class iVfsModuleInterface {
public:
    virtual ~iVfsModuleInterface() = default;
    virtual const FileTable *GetFileTable() const = 0;
    virtual iVfsContainer *GetContainer(uint32_t container) const = 0;

    virtual iVfsContainer *CreateContainer(const std::string_view &container_class,
                                           const VariantArgumentMap &arguments) = 0;
};

} // namespace MoonGlare::StarVfs
