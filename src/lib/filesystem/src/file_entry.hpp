#pragma once

#include <string>
#include <string_view>
#include <svfs/hashes.hpp>
#include <vector>

namespace MoonGlare::StarVfs {

class FileEntry {
public:
    std::string file_name; // TODO: switch to memory pool

    ContainerFileId container_file_id{0};
    uint32_t container_id{0};

    FilePathHash parent_path_hash{0};
    FilePathHash file_path_hash{0};
    FileResourceId resource_id{0};

    FileEntry *parent{nullptr};
    std::vector<std::unique_ptr<FileEntry>> children; // TODO:

    std::string GetFullPath() const;
    FileEntry *AddChild(std::string child_file_name, FilePathHash child_path_hash);

    bool IsDirectory() const { return !children.empty(); }
    bool IsHidden() const { return file_name.size() > 0 && file_name[0] == '.'; }
};

} // namespace MoonGlare::StarVfs
