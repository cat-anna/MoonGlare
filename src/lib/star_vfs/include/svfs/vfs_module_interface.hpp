
#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
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

    virtual const FileEntry *GetFileByPath(const std::string &file_path) const = 0;
    virtual bool ReadFile(const FileEntry *file_entry, std::string &file_data) const = 0;
    virtual bool WriteFile(const FileEntry *file_entry, const std::string &file_data) = 0;
    virtual bool IsDirectory(const FileEntry *file_entry) const = 0;
    virtual std::string GetFullPath(const FileEntry *file_entry) const = 0;
    virtual FileResourceId GetResourceId(const FileEntry *file_entry) const = 0;
    virtual bool IsHidden(const FileEntry *file_entry) const = 0;
    virtual void SetHidden(const FileEntry *file_entry, bool value) = 0;

    using EnumerateFileFunctor = std::function<bool(const FileEntry *file_entry, std::string_view file_name)>;
    virtual bool EnumeratePath(const std::string_view &start_path, EnumerateFileFunctor &functor,
                               bool recursive = false) const = 0;
    virtual bool EnumerateFile(const FileEntry *file_entry, EnumerateFileFunctor &functor,
                               bool recursive = false) const = 0;
};

} // namespace MoonGlare::StarVfs
