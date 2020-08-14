
#pragma once

#include <cstdint>
#include <svfs/vfs_container.hpp>

namespace MoonGlare::StarVfs {

class FileEntry;
class FileTable;
class iClassRegister;

class iVfsModuleInterface {
public:
    virtual ~iVfsModuleInterface() = default;
    virtual const FileTable *GetFileTable() const = 0;
    virtual iVfsContainer *GetContainer(uint32_t container) const = 0;
    virtual iClassRegister *GetClassRegister() const = 0;

    virtual iVfsContainer *CreateContainer(const std::string_view &container_class,
                                           const VariantArgumentMap &arguments) = 0;

    virtual bool ReadFile(const FileEntry *file_entry, std::string &file_data) const = 0;
};

} // namespace MoonGlare::StarVfs
