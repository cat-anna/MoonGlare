#pragma once

#include "svfs/hashes.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace MoonGlare {

struct FileInfo {
    std::string_view file_name;
    bool is_directory;
    bool is_hidden;
    StarVfs::FilePathHash file_path_hash;
    StarVfs::FilePathHash parent_path_hash;
};
using FileInfoTable = std::vector<FileInfo>;

class iReadOnlyFileSystem {
public:
    virtual bool ReadFileByPath(std::string_view path, std::string &file_data) const = 0;
    virtual bool ReadFileByResourceId(FileResourceId resource, std::string &file_data) const = 0;

    virtual bool EnumeratePath(std::string_view path, FileInfoTable &result_file_table) const = 0;

    // virtual void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out) = 0;
    // virtual std::string GetFileName(StarVFS::FileID fid) const = 0;
    // virtual std::string GetFullFileName(StarVFS::FileID fid) const = 0;
    // virtual bool FileExists(const std::string &full_path) const = 0;

    virtual FileResourceId GetResourceByPath(std::string_view path) const = 0;
    virtual std::string GetNameOfResource(FileResourceId resource, bool wants_full_path = true) const = 0;

protected:
    virtual ~iReadOnlyFileSystem() = default;
};

} // namespace MoonGlare
